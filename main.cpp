#include "hnsw.h"
#include "utils.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

void randomTest(int numItems, int dim, int numQueries, int K) {
	default_random_engine generator;
	uniform_real_distribution<double> distribution(0.0,1.0);
	vector<Item> randomItems;
	for (int i = 0; i < numItems; i++) {
		vector<double> temp(dim);
		for (int d = 0; d < dim; d++) {
			temp[d] = distribution(generator);
		}
		randomItems.emplace_back(temp);
	}
	random_shuffle(randomItems.begin(), randomItems.end());

	// construct graph
	HNSWGraph myHNSWGraph(10, 30, 30, 10, 2);
	for (int i = 0; i < numItems; i++) {
		// if (i % 10000 == 0) cout << i << endl;
		myHNSWGraph.Insert(randomItems[i]);
	}
	
	double total_brute_force_time = 0.0;
	double total_hnsw_time = 0.0;

	cout << "START QUERY" << endl;
	int numHits = 0;
	for (int i = 0; i < numQueries; i++) {
		// Generate random query
		vector<double> temp(dim);
		for (int d = 0; d < dim; d++) {
			temp[d] = distribution(generator);
		}
		Item query(temp);

		// Brute force
		clock_t begin_time = clock();
		vector<pair<double, int>> distPairs;
		for (int j = 0; j < numItems; j++) {
			if (j == i) continue;
			distPairs.emplace_back(query.dist(randomItems[j]), j);
		}
		sort(distPairs.begin(), distPairs.end());
		total_brute_force_time += double( clock () - begin_time ) /  CLOCKS_PER_SEC;

		begin_time = clock();
		vector<int> knns = myHNSWGraph.KNNSearch(query, K);
		total_hnsw_time += double( clock () - begin_time ) /  CLOCKS_PER_SEC;

		if (knns[0] == distPairs[0].second) numHits++;
	}
	cout << numHits << " " <<  numQueries / total_brute_force_time  << " " <<  numQueries / total_hnsw_time << endl;
	// myHNSWGraph.printGraph();
}


void gistTest(int numItems, int dim, int numQueries, int K, bool usePQ = false){
	const string buildpath = "/share/ann_benchmarks/gist/train.tsv";
	vector<Item> buildItems = readVectorsFromTSV(buildpath, dim, numItems);

	// construct graph
	HNSWGraph myHNSWGraph(10, 30, 30, 10, 2);
	for (int i = 0; i < numItems; i++) {
		// if (i % 1000 == 0) cout << "build progress: " << (double)i / numItems * 100 << "%" << endl;
		myHNSWGraph.Insert(buildItems[i]);
	}
	cout << "build hnsw success" << endl;
	if(usePQ == true){
		myHNSWGraph.buildPQ(dim / 2, 2048, dim, 200);
		cout << "build pq success" << endl;
	}


	const string testpath = "/share/ann_benchmarks/gist/test.tsv";
	vector<Item> testItems = readVectorsFromTSV(testpath, dim, numQueries);
	double total_brute_force_time = 0.0;
	double total_hnsw_time = 0.0;

	int numHits = 0;
	// double interSize = 0;
	for (int i = 0; i < numQueries; i++) {
		Item &query = testItems[i];
		// cout << "query #" << i << endl;
		if(usePQ) query.get_centroids(myHNSWGraph.kmeans);
		
		// for(auto t: query.centroids) cout << t << " ";
		// cout << endl;
		// Brute force
		clock_t begin_time = clock();
		vector<pair<double, int>> distPairs;
		for (int j = 0; j < numItems; j++) {
			if (j == i) continue;
			distPairs.emplace_back(query.dist(buildItems[j]), j);
		}
		sort(distPairs.begin(), distPairs.end());
		total_brute_force_time += double( clock () - begin_time ) /  CLOCKS_PER_SEC;

		vector<int> groundtruth;
		for(int i = 0; i < K; ++i)
			groundtruth.push_back(distPairs[i].second);

		// cout << "start KNN" << endl;
		begin_time = clock();
		vector<int> knns = myHNSWGraph.KNNSearch(query, K);
		total_hnsw_time += double( clock () - begin_time ) /  CLOCKS_PER_SEC;

		// cout << "end KNN" << endl;
		if (knns[0] == groundtruth[0]) numHits++;
		// interSize = interSize / (i + 1) * i + (double)intersectionSize(groundtruth, knns) / (i + 1);

	}
	if(usePQ)
		cout << "HNSW+PQ:" << endl;
	else cout << "HNSW:" << endl; 
	cout << "Top-1 Recall:" << numHits << " " << numQueries / total_brute_force_time  << " " << numQueries / total_hnsw_time << endl;
}


int main() {
	gistTest(10000, 960, 100, 10, false);
	gistTest(10000, 960, 100, 10, true);
	// randomTest(10000, 4, 100, 5);
	return 0;
}
