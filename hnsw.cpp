#include "hnsw.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>
using namespace std;

void HNSWGraph::buildPQ(int _subspaceNum, int _centroidNum, int dim, int max_iter){
	subspaceNum = _subspaceNum;
	centroidNum = _centroidNum;
	usePQ = true;
	int subspaceDim = dim / subspaceNum;
	for(int i = 0; i < subspaceNum; ++i){
		vector<vector<double>> points(itemNum, vector<double>(subspaceDim));
		for(int j = 0; j < itemNum; ++j){
			for(int k = 0; k < subspaceDim; ++k)
				points[j][k] = items[j].values[i * subspaceDim + k];
		}
		kmeans.push_back(KMeans(centroidNum, max_iter));
		kmeans[i].initialize(points);
	}
	for(int i = 0; i < items.size(); ++i)
		items[i].get_centroids(kmeans);
}

vector<int> HNSWGraph::searchLayer(Item& q, int ep, int ef, int lc) {
	set<pair<double, int>> candidates;
	set<pair<double, int>> nearestNeighbors;
	unordered_set<int> isVisited;

	// if(usePQ) cout << "debug 1" << endl;
	double td = usePQ ? q.pqdist(items[ep], kmeans) : q.dist(items[ep]);
	// if(usePQ) cout << "debug 2" << endl;
	candidates.insert(make_pair(td, ep));
	nearestNeighbors.insert(make_pair(td, ep));
	isVisited.insert(ep);
	while (!candidates.empty()) {
		auto ci = candidates.begin(); candidates.erase(candidates.begin());
		int nid = ci->second;
		auto fi = nearestNeighbors.end(); fi--;
		if (ci->first > fi->first) break;
		for (int ed: layerEdgeLists[lc][nid]) {
			if (isVisited.find(ed) != isVisited.end()) continue;
			fi = nearestNeighbors.end(); fi--;
			isVisited.insert(ed);
			if(usePQ == false)
				td = q.dist(items[ed]);
			else
				td = q.pqdist(items[ed], kmeans);
			if ((td < fi->first) || nearestNeighbors.size() < ef) {
				candidates.insert(make_pair(td, ed));
				nearestNeighbors.insert(make_pair(td, ed));
				if (nearestNeighbors.size() > ef) nearestNeighbors.erase(fi);
			}
		}
	}
	vector<int> results;
	for(auto &p: nearestNeighbors) results.push_back(p.second);
	return results;
}

vector<int> HNSWGraph::KNNSearch(Item& q, int K) {
	int maxLyer = layerEdgeLists.size() - 1;
	int ep = enterNode;
	for (int l = maxLyer; l >= 1; l--) ep = searchLayer(q, ep, 1, l)[0];
	return searchLayer(q, ep, K, 0);
}

void HNSWGraph::addEdge(int st, int ed, int lc) {
	if (st == ed) return;
	layerEdgeLists[lc][st].push_back(ed);
	layerEdgeLists[lc][ed].push_back(st);
}

void HNSWGraph::Insert(Item& q) {
	int nid = items.size();
	itemNum++; items.push_back(q);
	// sample layer
	int maxLyer = layerEdgeLists.size() - 1;
	int l = 0;
	uniform_real_distribution<double> distribution(0.0,1.0);
	while(l < ml && (1.0 / ml <= distribution(generator))) {
		l++;
		if (layerEdgeLists.size() <= l) layerEdgeLists.push_back(unordered_map<int, vector<int>>());
	}
	if (nid == 0) {
		enterNode = nid;
		return;
	}
	// search up layer entrance
	int ep = enterNode;
	for (int i = maxLyer; i > l; i--) ep = searchLayer(q, ep, 1, i)[0];
	for (int i = min(l, maxLyer); i >= 0; i--) {
		int MM = l == 0 ? MMax0 : MMax;
		vector<int> neighbors = searchLayer(q, ep, efConstruction, i);
		vector<int> selectedNeighbors = vector<int>(neighbors.begin(), neighbors.begin()+min(int(neighbors.size()), M));
		for (int n: selectedNeighbors) addEdge(n, nid, i);
		for (int n: selectedNeighbors) {
			if (layerEdgeLists[i][n].size() > MM) {
				vector<pair<double, int>> distPairs;
				for (int nn: layerEdgeLists[i][n]) distPairs.emplace_back(items[n].dist(items[nn]), nn);
				sort(distPairs.begin(), distPairs.end());
				layerEdgeLists[i][n].clear();
				for (int d = 0; d < min(int(distPairs.size()), MM); d++) layerEdgeLists[i][n].push_back(distPairs[d].second);
			}
		}
		ep = selectedNeighbors[0];
	}
	if (l == layerEdgeLists.size() - 1) enterNode = nid;
}
