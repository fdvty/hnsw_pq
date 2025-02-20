#include<vector>
#include<iostream>
#include<fstream>
#include<sstream>
#include<algorithm>
#include"hnsw.h"


std::vector<Item> readVectorsFromTSV(const std::string& filepath, int dim, int numItems) {
    std::vector<Item> items;  // 存储所有向量
    std::ifstream file(filepath);  // 打开文件

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return items;
    }

    std::string line;
    while (std::getline(file, line)) {  // 逐行读取文件
        std::istringstream iss(line);
        std::vector<double> values(dim);  // 存储当前行的向量
        double value;
        int count = 0;

        // 读取每个值
        while (iss >> value) {
            if (count >= dim) {
                std::cerr << "Warning: Line contains more values than expected. Ignoring extra values." << std::endl;
                break;
            }
            values[count++] = value;
        }

        // 检查是否读取了足够的数值
        if (count < dim) {
            std::cerr << "Error: Line contains fewer values than expected. Skipping this line." << std::endl;
            continue;
        }

        // 将向量添加到 items 中
        items.push_back({values});
		if(items.size() >= numItems)
			break;
    }

    file.close();
    return items;
}


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