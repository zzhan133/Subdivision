#include <fstream>
#include <vector>
#include <list>
#include <queue>

#include "OBJFileReader.h"
#include "Solid.h"
#include "iterators.h"

using namespace MeshLib;

using namespace std;
#define Num 40000
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

void main(int argc, char *argv[])
{
	// Read in the obj file
	Solid mesh;
	OBJFileReader of;
	std::ifstream in(argv[1]);
	of.readToSolid(&mesh, in);

	/******************* Put you subdivision processing here *********************/


	//1. highlights all edges  zz begin 2013-02-21 16:11:20

	SolidEdgeIterator eiter(&mesh);
	for(; !eiter.end(); ++eiter)
	{
		Edge *e = *eiter;
		e->string ()=std::string("sharp");
	}
	//end zz 2013Äê2ÔÂ21ÈÕ16:18:10

	//2. establish the adjacency list [2/21/2013 Zhe]
	list<int> Graphface[Num];
	SolidFaceIterator fiter(&mesh);
	for(; !fiter.end(); ++fiter)
	{
		Face *f = *fiter;
		FaceHalfedgeIterator hfiter(f);
		HalfEdge *hf = NULL;
		for (;!hfiter.end();++hfiter)
		{		
			hf = *hfiter;
			hf = hf->he_sym();
			Graphface[f->id()].push_back(hf->face()->id());
		}
	}
	//  [2/21/2013 Zhe]

	/************************************************************************/
	/* // 3.MST Algorithm [19:00/2/21/2013 Zhe]                                                                     */
	/************************************************************************/
	list<int> Mstqueue;
	queue<int> nLayer;
	list<int>::iterator mstiterator,Mstqueueiterator,Layeriterator;
	fiter.reset();
	int a[Num] = {0};

	Face *MSTface = *fiter;
	Mstqueue.push_back(MSTface->id());
	nLayer.push(MSTface->id()); //initialization
	a[MSTface->id()] = TRUE;
	while (!nLayer.empty())
	{
		int Faceid = nLayer.front();	
		nLayer.pop();
		//MstGraphface[Faceid].push_back(Faceid);
		for (mstiterator = Graphface[Faceid].begin();mstiterator != Graphface[Faceid].end();++mstiterator)
		{
			//Layeriterator = find (Mstqueue.begin(), Mstqueue.end(), *mstiterator);
			if (a[*mstiterator] == 0) 
			{
				Mstqueue.push_back(*mstiterator);
				nLayer.push(*mstiterator);
				a[*mstiterator] = TRUE;
				int nFaceid_father = Faceid;
				int nFaceId_son = *mstiterator;

				MeshLib::Face *f = mesh.idFace (nFaceid_father);
				MeshLib::FaceHalfedgeIterator fheiter(f);

				MeshLib::Edge * inter_edge;
				for (; !fheiter.end (); ++fheiter)
				{
					MeshLib::HalfEdge *he = *fheiter;
					he = he->he_sym ();
					if (he->face ()->id ()== nFaceId_son)
					{
						inter_edge=he->edge ();
						inter_edge->string () = std::string("");
						break;
					}
				}
			}
		}
	}



	// 3. cut the graph [23:19/2/21/2013 Zhe]
	SolidVertexIterator iter(&mesh);
	std::map<int, int> vidToObjID;

	do 
	{
		for(; !iter.end(); ++iter)
		{
			Vertex *v = *iter;
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
		iter.reset();

	} while (ifsharpequalone(vidToObjID));

	mesh.write ("kitten_1.m"); 

	//	os.close();
}