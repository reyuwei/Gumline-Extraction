//Wu K, Chen L, Li J, et al. Tooth segmentation on dental meshes using morphologic skeleton[J]. 
//Computers & Graphics, 2014, 38: 199-211.
//Rossl C, Kobbelt L, Seidel H P. Extraction of feature lines on triangulated surfaces using morphological operators[C]
//Proceedings of the AAAI Symposium on Smart Graphics. 2000: 71-75.

#include <stdio.h>
#include <stdlib.h>
#include "TriMesh.h"
#include "TriMesh_algo.h"
#include "XForm.h"
#include "apparentridge.h"
#include "GLCamera.h"
#include "timestamp.h"
#include "GL/glui.h"
#include <set>
#include <vector>
#include <queue>
#ifndef DARWIN
//#include <GL/glext.h>
#endif
#include <algorithm>
using namespace std;

using namespace trimesh;

#define MINCONNECTEDSIZE 20
#define CURVTHRESA -5
#define CURVTHRESB -0.5


class toothseg
{
public:
	TriMesh *obj;
	vector<int> featurepoints;
	vector<int> featurepoints_e;
	vector<int> skeleton;

	vector<int> complex;
	vector<int> center;
	vector<int> gumdisk;
	vector<int> toothdisk;
	vector<int> special;

	vector<int> border;

	vector<int> drawskeleton;

	vector<int> regiontagforvertices;




	vector<int> disk_;
	vector<int> center_;
	vector<int> complex_;

	vector<bool> guminfeature;
	vector<int> gumarea;
	vector<bool> toothinfeature;
	vector<int> tootharea;

	vector<vector<int>> connectedparts;

	int assumgum = -1;

	int orifpnumber = 0;  //original feature point number

	toothseg();
	toothseg(TriMesh *obj_);
	~toothseg();

	int showfeaturepoints = 0;
	int showcomplexpoints = 0;
	int showskeletonpoints = 1;
	int showconnectedparts = 0;
	int showdiskpoints = 0;
	int showcenterpoints = 0;
	int showfloodfill = 0;
	int drawskeletonedge = 1;



	vector<float> color;

	void BindOBJ(TriMesh *obj_);
	void DrawFeaturePoints();

	void AssumGumPoint();

	void doseg();
	///////////////////////////////////
	void ExtractFeaturePoints();

	void ConnectivityFiltering(string condition);
	void ConnectivityTest();
	int ConnectedParts(vector<int> points);
	void Dilation();
	void Erosion();
	void Opening(int n);
	void Closing(int n);

	void Sizing(int k, int n);

	void initColor();

	void ClassificatePoints();
	void Skeletonize();
	void Pruning();

	void FindRegions();

	void SingleWidth();

	void FloodFillGum();

	///////////////////////////////////
	void GetOrderedNeighbors(vector<int> &neighbors, int center);
	void OrderSkeletonToDraw();


	//////////////////////
	//void ClassificatePoints_();
	//void Skeletonize_();



};

toothseg::toothseg()
{
	initColor();
}
void toothseg::initColor()
{
	for (int i = 0; i < 5000; i++)
	{
		color.push_back(rand()*1.0 / RAND_MAX);
	}
}


toothseg::toothseg(TriMesh *obj_)
{
	this->obj = obj_;
}

void toothseg::BindOBJ(TriMesh *obj_)
{
	this->obj = obj_;
	this->obj->need_neighbors();
	this->obj->need_normals();
	this->AssumGumPoint();
}

toothseg::~toothseg()
{
	delete this->obj;
}

void toothseg::doseg()
{
	ExtractFeaturePoints();
	ConnectivityFiltering("");
	Closing(1);
	ConnectivityFiltering("max");

	//FloodFillGum();
	//SingleWidth();
	//FindRegions();
	//OrderSkeleton();
}

void toothseg::DrawFeaturePoints()
{
	glPointSize(4.0f);
	glBegin(GL_POINTS);

	if (showfeaturepoints)
	{
		for (int i = 0; i < featurepoints.size(); i++)
		{
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3fv(this->obj->vertices[featurepoints[i]]);
		}
	}

	if (showcomplexpoints)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		for (int i = 0; i < complex.size(); i++)
		{
			glVertex3fv(this->obj->vertices[complex[i]]);
		}
		for (int i = 0; i < complex_.size(); i++)
		{
			glVertex3fv(this->obj->vertices[complex_[i]]);
		}
	}


	if (showdiskpoints)
	{
		glColor3f(1.0f, 0.8f, 0.9f);
		for (int i = 0; i < gumdisk.size(); i++)
		{
			glVertex3fv(this->obj->vertices[gumdisk[i]]);
		}

		glColor3f(0.0f, 1.0f, 1.0f);
		for (int i = 0; i < toothdisk.size(); i++)
		{
			glVertex3fv(this->obj->vertices[toothdisk[i]]);
		}


		for (int i = 0; i < disk_.size(); i++)
		{
			glVertex3fv(this->obj->vertices[disk_[i]]);
		}
	}

	if (showcenterpoints)
	{
		glColor3f(1.0f, 1.0f, 0.0f);
		for (int i = 0; i < center.size(); i++)
		{
			glVertex3fv(this->obj->vertices[center[i]]);
		}
		for (int i = 0; i < center_.size(); i++)
		{
			glVertex3fv(this->obj->vertices[center_[i]]);
		}
	}

	if (showskeletonpoints)
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		for (int i = 0; i < skeleton.size(); i++)
		{
			glVertex3fv(this->obj->vertices[skeleton[i]]);
		}
		/*	glColor3f(1.0f, 0.0f, 0.0f);
			for (int i = 0; i < special.size(); i++)
			{
			glVertex3fv(this->obj->vertices[special[i]]);
			}*/

	}





	if (showfloodfill)
	{
		//glColor3f(0.8f, 0.8f, 0.9f);
		//for (int i = 0; i < gumarea.size(); i++)
		//{
		//	glVertex3fv(this->obj->vertices[gumarea[i]]);
		//}
		//glColor3f(0.8f, 0.8f, 0.f);
		//for (int i = 0; i < tootharea.size(); i++)
		//{
		//	glVertex3fv(this->obj->vertices[tootharea[i]]);
		//}
		if (regiontagforvertices.size() > 0)
		{
			for (int i = 0; i < this->obj->vertices.size(); i++)
			{
				float regionid = this->regiontagforvertices[i];
				if (regionid != 0)
				{
					glColor3f(color[regionid], color[regionid + 10], color[regionid + 20]);
					glVertex3fv(this->obj->vertices[i]);
				}
			}
		}


	}

	//glColor3f(1.0,137.0/255,80.0/255);
	//for (int i = 0; i < tootharea.size(); i++)
	//{
	//	glVertex3fv(this->obj->vertices[tootharea[i]]);
	//}


	//if (showconnectedparts)
	//{
	//	for (int i = 0; i < this->connectedparts.size(); i++)
	//	{
	//		//glColor3f(i*1.0 / connectedparts.size(), i*1.0 / connectedparts.size(), i*1.0 / connectedparts.size());
	//		glColor3f(color[i], color[i + 1], color[i + 2]);

	//		for (int j = 0; j < this->connectedparts[i].size(); j++)
	//		{
	//			glVertex3fv(this->obj->vertices[this->featurepoints[this->connectedparts[i][j]]]);
	//		}

	//	}
	//}



	glPointSize(1.0f);
	glEnd();

	if (drawskeletonedge)
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i = 0; i < this->drawskeleton.size(); i++)
		{
			glVertex3fv(this->obj->vertices[drawskeleton[i]]);
		}
		glEnd();
	}


}

void toothseg::ExtractFeaturePoints()
{
	cout << "Extract Feature Points with mean curvature: [" << CURVTHRESA << "," << CURVTHRESB << "]" << endl;
	vector<float> k1 = this->obj->curv1;
	vector<float> k2 = this->obj->curv2;

	for (int i = 0; i < k1.size(); i++)
	{
		//mean curvature thresholding
		//float meancurv = k1[i] + k2[i];

		float meancurv = k1[i];

		if (meancurv <= CURVTHRESB && meancurv >= CURVTHRESA)
			this->featurepoints.push_back(i);
	}
	orifpnumber = this->featurepoints.size();

}

void toothseg::ConnectivityTest()
{
	connectedparts.clear();
	vector<bool> packed;
	vector<int> cpid;
	vector<int> partsize;
	int avgpartsize = 0;
	int index = 0;

	for (int i = 0; i < this->featurepoints.size(); i++)
	{
		packed.push_back(false);
		cpid.push_back(-1);
	}

	for (int i = 0; i < this->featurepoints.size(); i++)
	{
		//cout << "connected parts:" << connectedparts.size() << "     iterator: " << i << endl;
		if (!packed[i])
		{
			queue<int> Q;
			vector<int> L;
			Q.push(i);
			L.push_back(i);
			packed[i] = true;
			while (Q.size()>0)
			{
				int p = Q.front();
				Q.pop();
				vector<int> neighbors = this->obj->neighbors[featurepoints[p]];
				for (int j = 0; j < neighbors.size(); j++)
				{
					vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
					if (it != this->featurepoints.end())
					{
						int idxinfeaturepoints = it - featurepoints.begin();
						if (!packed[idxinfeaturepoints])
						{
							//in feature points - connected
							Q.push(idxinfeaturepoints);
							L.push_back(idxinfeaturepoints);
							packed[idxinfeaturepoints] = true;
						}
					}
				}
			}
			for (int k = 0; k < L.size(); k++)
			{
				cpid[L[k]] = index;
			}
			index++;
			connectedparts.push_back(L);
			partsize.push_back(L.size());
			avgpartsize += L.size();
		}
	}
	avgpartsize /= connectedparts.size();
	//cout << connectedparts.size() << endl;
	sort(partsize.begin(), partsize.end());
	int midpartsize = partsize[partsize.size() / 2];



}

int toothseg::ConnectedParts(vector<int> points)
{
	vector<bool> packed;
	vector<int> cpid;
	vector<int> partsize;
	int avgpartsize = 0;
	int index = 0;

	for (int i = 0; i < points.size(); i++)
	{
		packed.push_back(false);
		cpid.push_back(-1);
	}

	for (int i = 0; i < points.size(); i++)
	{
		if (!packed[i])
		{
			queue<int> Q;
			vector<int> L;
			Q.push(i);
			L.push_back(i);
			packed[i] = true;
			while (Q.size()>0)
			{
				int p = Q.front();
				Q.pop();
				vector<int> neighbors = this->obj->neighbors[points[p]];
				for (int j = 0; j < neighbors.size(); j++)
				{
					vector<int>::iterator it = find(points.begin(), points.end(), neighbors[j]);
					if (it != points.end())
					{
						int idxinfeaturepoints = it - points.begin();
						if (!packed[idxinfeaturepoints])
						{
							//in feature points - connected
							Q.push(idxinfeaturepoints);
							L.push_back(idxinfeaturepoints);
							packed[idxinfeaturepoints] = true;
						}
					}
				}
			}
			for (int k = 0; k < L.size(); k++)
			{
				cpid[L[k]] = index;
			}
			index++;
		}
	}
	return index + 1;
}


void toothseg::ConnectivityFiltering(string condition)
{

	cout << "Connectivity Filtering" << endl;
	this->ConnectivityTest();

	vector<int> afterfilter;
	int maxpart = 0;
	int maxidx = -1;

	for (int i = 0; i < connectedparts.size(); i++)
	{
		if (connectedparts[i].size() > maxpart)
		{
			maxpart = connectedparts[i].size();
			maxidx = i;
		}

		if (condition != "max")
		{
			if (connectedparts[i].size() > MINCONNECTEDSIZE)
			{
				vector<int> onepart = connectedparts[i];
				for (int j = 0; j < onepart.size(); j++)
				{
					afterfilter.push_back(this->featurepoints[onepart[j]]);
				}
			}
		}
	}

	if (condition == "max")
	{
		this->showconnectedparts = false;
		afterfilter.clear();
		//for (int i = 0; i < connectedparts.size(); i++)
		//{
		//	if (connectedparts[i].size() == maxpart)
		//	{
		vector<int> onepart = connectedparts[maxidx];
		for (int j = 0; j < onepart.size(); j++)
		{
			afterfilter.push_back(this->featurepoints[onepart[j]]);
		}
		//}
		//}
	}

	this->featurepoints.clear();
	this->featurepoints = afterfilter;
}


void toothseg::Dilation()
{
	cout << "Dilation" << endl;

	int orin = this->featurepoints.size();
	for (int i = 0; i < orin; i++)
	{
		int curr = this->featurepoints[i];
		vector<int> neighbors = this->obj->neighbors[curr];

		for (int j = 0; j < neighbors.size(); j++)
		{
			vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
			if (it == this->featurepoints.end())
			{
				//not existed
				this->featurepoints.push_back(neighbors[j]);
			}
		}
	}
}

void toothseg::Erosion()
{
	cout << "Erosion" << endl;

	featurepoints_e.clear();
	//vector<int> featurepoints_erosion;
	for (int i = 0; i < this->featurepoints.size(); i++)
	{
		vector<int> neighbors = this->obj->neighbors[this->featurepoints[i]];
		bool allin = true;
		for (int j = 0; j < neighbors.size(); j++)
		{
			vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
			if (it == this->featurepoints.end())
			{
				//not existed
				allin = false;
				break;
			}
		}
		if (allin == true)
			featurepoints_e.push_back(this->featurepoints[i]);
	}
	featurepoints.clear();
	featurepoints = featurepoints_e;
}

void toothseg::Opening(int n)
{

	for (int i = 0; i < n; i++)		Erosion();
	for (int i = 0; i < n; i++)     Dilation();
}

void toothseg::Closing(int n)
{
	for (int i = 0; i < n; i++)     Dilation();
	for (int i = 0; i < n; i++)		Erosion();
}

void toothseg::Sizing(int k, int n)
{
	for (int i = 0; i < k; i++) { this->Opening(n); this->Closing(n); }
	cout << this->featurepoints.size() << endl;
}

void toothseg::GetOrderedNeighbors(vector<int> &neighbors, int center)
{
	vector<int> orderedneighbors;
	vector<int> neighborfaces = this->obj->adjacentfaces[center];

	TriMesh::Face f = this->obj->faces[neighborfaces[0]];
	int faceidx = neighborfaces[0];
	//counter clockwise
	int cinf = f.indexof(center);
	orderedneighbors.push_back(f.v[(cinf + 1) % 3]);
	orderedneighbors.push_back(f.v[(cinf + 2) % 3]);


	//while (orderedneighbors.size() != neighborfaces.size())
	while (orderedneighbors.size() != neighbors.size())
	{
		int nextf = this->obj->across_edge[faceidx][(cinf + 1) % 3];
		TriMesh::Face nextface = this->obj->faces[nextf];
		cinf = nextface.indexof(center);
		orderedneighbors.push_back(nextface.v[(cinf + 2) % 3]);
		faceidx = nextf;
	}

	neighbors.clear();
	neighbors = orderedneighbors;
}

void toothseg::ClassificatePoints()
{
	//bool redofloodfill;
	//for (int i = 0; i < this->featurepoints.size(); i++)
	//{
	//	if (this->guminfeature[this->featurepoints[i]])
	//	{
	//		redofloodfill = false;
	//		break;
	//	}
	//}
	//if (redofloodfill) this->FloodFillGum();

	cout << "\nPoints Classification" << endl;


	this->complex.clear();
	this->center.clear();
	this->toothdisk.clear();
	this->gumdisk.clear();
	this->special.clear();

	for (int i = 0; i < this->featurepoints.size(); i++)
	{
		vector<int> neighbors = this->obj->neighbors[this->featurepoints[i]];
		GetOrderedNeighbors(neighbors, this->featurepoints[i]);

		vector<int> isfeature;
		int complexity = 0;

		bool allin = true;
		int inn = 0;
		for (int j = 0; j < neighbors.size(); j++)
		{
			isfeature.push_back(1);
			vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
			if (it == this->featurepoints.end())
			{
				//not existed
				isfeature[isfeature.size() - 1] = 0;
				allin = false;
				//break;
			}
			else
			{
				inn++;
			}
			if (j >= 1)
				complexity += abs(isfeature[j] - isfeature[j - 1]);
		}
		if (isfeature.size() >= 2)
			complexity += abs(isfeature[0] - isfeature[isfeature.size() - 1]);

		if (complexity >= 2 && complexity < 4)
			this->border.push_back(this->featurepoints[i]);



		if (complexity >= 4)
		{
			//if (inn==2)
			this->complex.push_back(this->featurepoints[i]);
		}
		if (allin == true)
		{
			this->center.push_back(this->featurepoints[i]);
			/*for (int j = 0; j < neighbors.size(); j++)
			{
			if (this->guminfeature[neighbors[j]])
			this->gumdisk.push_back(neighbors[j]);
			else
			this->toothdisk.push_back(neighbors[j]);
			}*/
			//this->disk.insert(this->disk.end(), neighbors.begin(), neighbors.end());
		}
		if (allin == false)
		{
			if (this->guminfeature[this->featurepoints[i]])
			{
				if (this->toothinfeature[this->featurepoints[i]])
					this->special.push_back(this->featurepoints[i]);
				else
					this->gumdisk.push_back(this->featurepoints[i]);
			}
			else if (this->toothinfeature[this->featurepoints[i]])
			{
				this->toothdisk.push_back(this->featurepoints[i]);
			}
		}


	}
}

void toothseg::Skeletonize()
{
	cout << "Skeleton" << endl;

	vector<int> temp1;
	vector<int> temp2;
	vector<int> temp3;
	vector<int> temp4;

	//sort(disk.begin(), disk.end());
	sort(gumdisk.begin(), gumdisk.end());
	sort(toothdisk.begin(), toothdisk.end());
	sort(border.begin(), border.end());
	sort(center.begin(), center.end());
	sort(complex.begin(), complex.end());
	sort(featurepoints.begin(), featurepoints.end());

	vector<int>::iterator vector_iterator = unique(gumdisk.begin(), gumdisk.end());
	if (vector_iterator != gumdisk.end()){
		gumdisk.erase(vector_iterator, gumdisk.end());
	}

	vector_iterator = unique(toothdisk.begin(), toothdisk.end());
	if (vector_iterator != toothdisk.end()){
		toothdisk.erase(vector_iterator, toothdisk.end());
	}

	set_union(complex.begin(), complex.end(), center.begin(), center.end(), back_inserter(temp1));
	set_difference(featurepoints.begin(), featurepoints.end(), temp1.begin(), temp1.end(), back_inserter(temp2));

	double oneside = rand()*1.0 / RAND_MAX;
	if (oneside > 0.5)
	{
		cout << "eliminate gum disk point" << endl;
		set_intersection(gumdisk.begin(), gumdisk.end(), temp2.begin(), temp2.end(), back_inserter(temp3));
	}
	else
	{
		cout << "eliminate tooth disk point" << endl;
		set_intersection(toothdisk.begin(), toothdisk.end(), temp2.begin(), temp2.end(), back_inserter(temp3));
	}
	set_difference(featurepoints.begin(), featurepoints.end(), temp3.begin(), temp3.end(), back_inserter(temp4));

	this->skeleton.clear();
	this->skeleton = temp4;
}

void toothseg::Pruning()
{

	ClassificatePoints();

	cout << "Pruning" << endl;
	//sort(disk.begin(), disk.end());
	//sort(center.begin(), center.end());
	//sort(complex.begin(), complex.end());
	//sort(skeleton.begin(), skeleton.end());
	////sort(featurepoints.begin(), featurepoints.end());
	//vector<int> temp1;
	//vector<int> temp2;

	//set_difference(featurepoints.begin(), featurepoints.end(), complex.begin(), complex.end(), back_inserter(temp1));
	//set_difference(skeleton.begin(), skeleton.end(), temp1.begin(), temp1.end(), back_inserter(temp2));

	//this->skeleton.clear();
	//this->skeleton = temp2;

	this->skeleton = this->complex;
}

void toothseg::SingleWidth()
{
	int featurepnumber = this->featurepoints.size();
	if (this->skeleton.size() == 0)
		this->skeleton = this->featurepoints;
	int diff = 100;
	do
	{
		this->FloodFillGum();
		do
		{
			ClassificatePoints();
			Skeletonize();
			/*ClassificatePoints_();
			Skeletonize_();*/
			diff = featurepnumber - this->skeleton.size();
			featurepnumber = this->skeleton.size();
			this->featurepoints = this->skeleton;
		} while (abs(diff) > 10);
	} while (abs(diff) > 0);



	////save all flood fill area touched area as gumline
	//this->FloodFillGum();
	//this->skeleton.clear();
	//for (int i = 0; i < this->toothinfeature.size(); i++)
	//{
	//	if (toothinfeature[i])
	//	{
	//		this->skeleton.push_back(i);
	//	}
	//}
	// filter out not connected area, save only the max connected area
	this->featurepoints = this->skeleton;
	//this->ConnectivityFiltering("max");
	this->skeleton = this->featurepoints;

	// save only complex points as skeleton
	//this->FloodFillGum();
	//this->ClassificatePoints();
	//this->skeleton = this->complex;
	//OrderSkeletonToDraw();

}

void toothseg::OrderSkeletonToDraw() //BFS
{
	cout << "find lines" << endl;
	vector<bool> packed;
	for (int i = 0; i < this->obj->vertices.size(); i++)
	{
		packed.push_back(false);
	}
	queue<int> Q;
	vector<int> L;
	Q.push(skeleton[0]);
	//L.push_back(skeleton[0]);
	while (Q.size() > 0)
	{
		int p = Q.front();
		Q.pop();
		vector<int> neighbors = this->obj->neighbors[p];
		for (int j = 0; j < neighbors.size(); j++)
		{
			vector<int>::iterator it = find(this->skeleton.begin(), this->skeleton.end(), neighbors[j]);
			if (it != this->skeleton.end())
			{
				// in
				if (!packed[neighbors[j]])
				{
					//in feature points - connected
					Q.push(neighbors[j]);
					L.push_back(neighbors[j]);
					L.push_back(p);
					packed[neighbors[j]] = true;
				}
			}
		}
	}
	this->drawskeleton = L;
}


void toothseg::AssumGumPoint()
{
	float minz = 1e10;
	for (int i = 0; i < this->obj->vertices.size(); i++)
	{
		if (this->obj->vertices[i][2] < minz)
		{
			assumgum = i;
			minz = this->obj->vertices[i][2];
		}
	}
}

void toothseg::FloodFillGum() //BFS
{
	cout << "Flood Fill Gum" << endl;
	//find the vertex with smallest z
	vector<bool> packed, packed2;
	guminfeature.clear();
	for (int i = 0; i < this->obj->vertices.size(); i++)
	{
		packed.push_back(false);
		packed2.push_back(false);
		gumarea.push_back(false);
		guminfeature.push_back(false);
		toothinfeature.push_back(false);
	}

	//start region grow
	queue<int> Q;
	vector<int> L;
	Q.push(assumgum);
	L.push_back(assumgum);
	while (Q.size() > 0)
	{
		int p = Q.front();
		Q.pop();
		vector<int> neighbors = this->obj->neighbors[p];
		for (int j = 0; j < neighbors.size(); j++)
		{
			vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
			if (it == this->featurepoints.end())
			{
				//not in
				if (!packed[neighbors[j]])
				{
					//in feature points - connected
					Q.push(neighbors[j]);
					L.push_back(neighbors[j]);
					packed[neighbors[j]] = true;
				}
			}
			else
			{
				guminfeature[neighbors[j]] = true;
			}
		}
	}
	this->gumarea = L;


	// find tooth area
	tootharea.clear();
	sort(gumarea.begin(), gumarea.end());
	sort(featurepoints.begin(), featurepoints.end());

	vector<int> allv, temp;
	for (int i = 0; i < this->obj->vertices.size(); i++) allv.push_back(i);
	set_difference(allv.begin(), allv.end(), gumarea.begin(), gumarea.end(), back_inserter(temp));
	set_difference(temp.begin(), temp.end(), featurepoints.begin(), featurepoints.end(), back_inserter(tootharea));
	vector<bool> marked;
	for (int i = 0; i < this->obj->vertices.size(); i++)
	{
		marked.push_back(false);
	}
	for (int i = 0; i < tootharea.size(); i++)
	{
		marked[tootharea[i]] = true;
		vector<int> neighbors = this->obj->neighbors[tootharea[i]];
		for (int j = 0; j < neighbors.size(); j++)
		{
			if (!marked[neighbors[j]])
			{
				vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
				if (it != featurepoints.end())
				{
					toothinfeature[neighbors[j]] = true;
				}
				marked[neighbors[j]] = true;
			}
		}
	}


}

void toothseg::FindRegions()
{
	for (int i = 0; i < this->obj->vertices.size(); i++)
	{
		this->regiontagforvertices.push_back(-1);
	}
	int idx = 1; //0 for feature points
	for (int i = 0; i < this->obj->vertices.size(); i++)
	{
		//start region grow
		if (regiontagforvertices[i] == -1)
		{
			regiontagforvertices[i] = idx;
			queue<int> Q;
			vector<int> L;
			Q.push(i);
			L.push_back(i);
			while (Q.size() > 0)
			{
				int p = Q.front();
				Q.pop();
				vector<int> neighbors = this->obj->neighbors[p];
				for (int j = 0; j < neighbors.size(); j++)
				{
					vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
					if (it == this->featurepoints.end())
					{
						//not in
						if (regiontagforvertices[neighbors[j]] == -1)
						{
							//in feature points - connected
							Q.push(neighbors[j]);
							L.push_back(neighbors[j]);
							regiontagforvertices[neighbors[j]] = idx;
						}
					}
				}
			}
			idx++;
		}
	}

	for (int i = 0; i < this->featurepoints.size(); i++)
	{
		this->regiontagforvertices[this->featurepoints[i]] = 0;
	}


}





//
////////////////according to Tooth paper, not stable
//void toothseg::ClassificatePoints_()
//{
//	cout << "Points Classification_" << endl;
//
//	this->complex_.clear();
//	this->center_.clear();
//	this->disk_.clear();
//
//	vector<bool> disklabel;
//	vector<bool> centerlabel;
//
//	for (int i = 0; i < this->featurepoints.size(); i++)
//	{
//		disklabel.push_back(false);
//		centerlabel.push_back(false);
//		vector<int> neighbors = this->obj->neighbors[this->featurepoints[i]];
//		GetOrderedNeighbors(neighbors, this->featurepoints[i]);
//
//		vector<int> isfeature;
//		int complexity = 0;
//		int counter = 0;
//		bool allin = true;
//		for (int j = 0; j < neighbors.size(); j++)
//		{
//			isfeature.push_back(1);
//			vector<int>::iterator it = find(this->featurepoints.begin(), this->featurepoints.end(), neighbors[j]);
//			if (it == this->featurepoints.end())
//			{
//				//not existed
//				isfeature[isfeature.size() - 1] = 0;
//				allin = false;
//				//break;
//			}
//
//			if (j >= 1)
//				complexity += abs(isfeature[j] - isfeature[j - 1]);
//		}
//		if (isfeature.size() >= 2)
//			complexity += abs(isfeature[0] - isfeature[isfeature.size() - 1]);
//
//		//if (complexity >= 2 && complexity < 4)
//		//{
//		//	this->disk_.push_back(this->featurepoints[i]);
//		//	disklabel[i] = true;
//		//}
//		if (allin == false)
//		{
//			this->disk_.push_back(this->featurepoints[i]);
//		}
//
//
//		if (complexity >= 4)
//			this->complex_.push_back(this->featurepoints[i]);
//
//
//
//		if (allin == true)
//		{
//			this->center_.push_back(this->featurepoints[i]);
//			centerlabel[i] = true;
//		}
//
//
//
//
//
//
//		/////////////////////////////////////////////////
//		////eliminate boundary disk point
//		//vector<int> temp;
//		//for (int i = 0; i < this->disk_.size(); i++)
//		//{
//		//	int isbd = true;
//		//	int centercounter = 0;
//		//	vector<int> neighbors = this->obj->neighbors[this->disk_[i]];
//		//	for (int j = 0; j < neighbors.size(); j++)
//		//	{
//		//		vector<int>::iterator it = find(this->center_.begin(), this->center_.end(), neighbors[j]);
//		//		if (it != this->center_.end())
//		//		{
//		//			centercounter++;
//		//		}
//		//		else
//		//		{
//		//			it = find(this->complex_.begin(), this->complex_.end(), neighbors[j]);
//		//			if (it != this->complex_.end())
//		//			{
//		//				centercounter++;
//		//			}
//		//		}
//		//	}
//		//	/*if (isbd == false)
//		//	{
//		//	temp.push_back(this->disk_[i]);
//		//	}*/
//		//	if (centercounter >= 2)
//		//		temp.push_back(this->disk_[i]);
//		//}
//
//		//this->disk_ = temp;
//
//
//
//
//	}
//}
//void toothseg::Skeletonize_()
//{
//	cout << "Skeleton" << endl;
//
//	vector<int> temp1;
//	vector<int> temp2;
//	vector<int> temp3;
//	vector<int> temp4;
//
//	sort(disk_.begin(), disk_.end());
//	sort(center_.begin(), center_.end());
//	sort(complex_.begin(), complex_.end());
//	sort(featurepoints.begin(), featurepoints.end());
//
//	vector<int>::iterator vector_iterator = unique(disk_.begin(), disk_.end());
//	if (vector_iterator != disk_.end()){
//		disk_.erase(vector_iterator, disk_.end());
//	}
//
//	set_union(complex_.begin(), complex_.end(), center_.begin(), center_.end(), back_inserter(temp1));
//	set_difference(featurepoints.begin(), featurepoints.end(), temp1.begin(), temp1.end(), back_inserter(temp2));
//	set_intersection(disk_.begin(), disk_.end(), temp2.begin(), temp2.end(), back_inserter(temp3));
//	set_difference(featurepoints.begin(), featurepoints.end(), temp3.begin(), temp3.end(), back_inserter(temp4));
//
//	this->skeleton.clear();
//	this->skeleton = temp4;
//}
