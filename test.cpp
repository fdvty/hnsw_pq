#include <iostream>
#include <vector>
#include <cstdlib>
#include "kmeans.h"
 
using namespace std;
 

 
int main() {
    int K, max_iter;
    cout << "Enter number of clusters (K): ";
    cin >> K;
    cout << "Enter maximum number of iterations: ";
    cin >> max_iter;
 
    // 输入数据点
    int n, m;
    cout << "Enter number of data points (n): ";
    cin >> n;
    cout << "Enter number of features per data point (m): ";
    cin >> m;
 
    vector<vector<double>> data(n, vector<double>(m));
    cout << "Enter data points (each point has " << m << " features):" << endl;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> data[i][j];
        }
    }
 
    // 创建 KMeans 对象并运行聚类
    KMeans kmeans(K, max_iter);
    kmeans.initialize(data);
    kmeans.fit();
    kmeans.print_result();
 
    return 0;
}