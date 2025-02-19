#include<vector>
#include<iostream>
#include<fstream>
#include<sstream>
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

int intersectionSize(const std::vector<int>& vec1, const std::vector<int>& vec2) {
    int count = 0;
    for(auto v1: vec1)
        for(auto v2: vec2)
            if(v1 == v2)
                count++;
    return count;
}