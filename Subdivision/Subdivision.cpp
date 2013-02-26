#include <fstream>
#include <vector>
#include <list>
#include <queue>

#include "OBJFileReader.h"
#include "Solid.h"
#include "iterators.h"

using namespace MeshLib;
using namespace std;

#define Num 20000
#define TRUE  1


int ifsharpequalone(map<int,int> &a)
{
	map<int,int>::iterator iter;
	for (iter=a.begin();iter!=a.end();iter++)
	{
		if (iter->second == 1)
		{
			return 1;
		}
	}
	return 0;
}

void Cut_redundantnode(Solid  &mesh)
{
	SolidVertexIterator piter(&mesh);
	std::map<int, int> vidToObjID;

	do 
	{
		for(; !piter.end(); ++piter)
		{
			Vertex *v = *piter;
			MeshLib::VertexEdgeIterator edgeiter(v);
			MeshLib::Edge * inter_edge;
			int nsharp = 0;
			for (;!edgeiter.end();++edgeiter)
			{			
				inter_edge = *edgeiter;
				if (inter_edge->string() == "sharp")
				{
					nsharp++;
				}
			}


			//delete nsharp == 1
			if (1 == nsharp)
			{
				edgeiter.reset();
				for (;!edgeiter.end();++edgeiter)
				{			
					inter_edge = *edgeiter;
					if (inter_edge->string() == "sharp")
					{
						inter_edge->string () = std::string("");
						//nsharp = 0;
					}
				}

			}
			vidToObjID[v->id()] = nsharp;
		}
		piter.reset();

	} while (ifsharpequalone(vidToObjID));
}


//void Visit_DFS()
//{
//		
//}

//void()
//{
//
//}
void savegraph(Solid &mesh, map<Edge*,int> &Edgemap, int n)
{
	map<Edge*, int>::iterator itmap = Edgemap.begin();
	while(itmap != Edgemap.end())
	{
		if (itmap->second != -1)
		{
			itmap->first->string() = "";
		}
		itmap++;
	}
	Cut_redundantnode(mesh);
	char a[10];
	_itoa(n,a,10);
	string str = a;
	str = str + ".m";
	const char *p = str.c_str();
	mesh.write(p);
}

void Recovery(Solid& mesh, list<Edge*> &Edgegraph)
{
	list<Edge*>::iterator iter =Edgegraph.begin();
	for (;iter !=Edgegraph.end();iter++)
	{
		Edge* pEdge = *iter;
		pEdge->string() = "sharp";
	}
}
void main(int argc, char *argv[])
{
	// Read in the obj file
	Solid mesh;
	//OBJFileReader of;
	std::ifstream in(argv[1]);
	//of.readToSolid(&mesh, in);
	mesh.read(in);

	list<Edge*> Edgegraph;
	map<Edge*,int> Edgemap;
	int Edgeid = 0;
	SolidEdgeIterator iter(&mesh);
	for(;!iter.end();++iter)
	{
		Edge *e = *iter;
		std::string stra = "sharp";
		if (e->string() == stra)
		{
			Edgegraph.push_back(e);
			Edgemap[e] = Edgeid;
			Edgeid++;
		}
	}

	// using adjacency lists to represent graph [16:20/2/24/2013 Zhe]
	list<Edge*> Graphlist[Num];
	list<Vertex*> Graphvertexlist[Num];
	list<Vertex*> Verlist;
	map<Edge*, int>::iterator itmap = Edgemap.begin();
	while (itmap != Edgemap.end())
	{
		Edge* inedge = itmap->first;/*
		Vertex *v1, *v2;
		inedge->get_vertices(v1,v2);*/
		list<Edge*>::iterator iteratorgraph= Edgegraph.begin();
		for(;iteratorgraph != Edgegraph.end();iteratorgraph++)
		{
			Edge* jnedge = *iteratorgraph;
			Vertex* pvertex = inedge->conjunction(jnedge);
			Vertex* otherpoint = jnedge->other_vertex(pvertex);
			if (inedge != jnedge && (pvertex != NULL))
			{
				Graphlist[itmap->second].push_back(jnedge);
				Graphvertexlist[pvertex->id()].push_back(otherpoint);
				Verlist.push_back(pvertex);
			}
		}
		itmap++;		
	}
	// end [16:41/2/24/2013 Zhe]

	//using BFS establish MST
	int Indexgraph[Num] = {0};
	//itmap = Edgemap.begin();
	//int nIndex = itmap->second;
	//Indexgraph[nIndex] = 1;
	/*Vertex* first = Verlist.front();*/
	queue<Vertex*> sharpIndex;
	Vertex* pnVertex = Verlist.front();
	sharpIndex.push(pnVertex);
	Indexgraph[pnVertex->id()] = 1;
	list<Vertex*>::iterator mstiterator;
	list<Edge*> finalV;
	while (!sharpIndex.empty())
	{
		Vertex* e = sharpIndex.front();
		int nIndex = e->id();
		sharpIndex.pop();
		for(mstiterator = Graphvertexlist[nIndex].begin();mstiterator != Graphvertexlist[nIndex].end();++mstiterator)
		{
			if(0 == Indexgraph[(*mstiterator)->id()])
			{
				Indexgraph[(*mstiterator)->id()] = 1;
				sharpIndex.push(*mstiterator);
				int s = e->id();
				int t = (*mstiterator)->id();
				Edge *ed = mesh.idEdge(s,t);
				finalV.push_back(ed);
				Edgemap[ed] = -1;			
			}
		}

	}

	itmap = Edgemap.begin();
	list<Edge*> individual;
	while (itmap != Edgemap.end())
	{
		if (-1 != itmap->second)
		{
			individual.push_back(itmap->first);
		}
		itmap++;
	}
	if (individual.empty())
	{
		return;
	}

	list<Edge*>::iterator edgeiter = individual.begin();
	int n = 10;
	for(;edgeiter != individual.end();edgeiter++)
	{
		Edge* ped = *edgeiter;
		int original = Edgemap[ped];
		Edgemap[ped] = -1;
		savegraph(mesh, Edgemap, n);
		Edgemap[ped] = original;
		n++;
		Recovery(mesh,Edgegraph);
	}


}