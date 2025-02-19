#ifndef KMEANS_H
#define KMEANS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
 
using namespace std;
 
// 计算两个点之间的欧几里得距离
inline double euclidean_distance(const vector<double>& point1, const vector<double>& point2) {
    double sum = 0;
    for (size_t i = 0; i < point1.size(); ++i) {
        sum += pow(point1[i] - point2[i], 2);
    }
    // return sqrt(sum);
    return sum;
}
 
// K-means 聚类算法
class KMeans {
private:
    int K;  // 簇的个数
    int max_iter;  // 最大迭代次数
    vector<vector<double>> points;  // 数据点
    vector<vector<double>> centroids;  // 簇的中心
    vector<int> labels;  // 每个数据点的簇标签
 
public:
    KMeans(int K, int max_iter) : K(K), max_iter(max_iter) {}
 
    // 初始化数据点和簇的中心
    void initialize(const vector<vector<double>>& data) {
        points = data;
        labels.resize(points.size(), -1);
        centroids.resize(K, vector<double>(points[0].size(), 0));
 
        // 随机选择 K 个数据点作为初始簇中心
        srand(time(0));
        for (int i = 0; i < K; ++i) {
            int random_index = rand() % points.size();
            centroids[i] = points[random_index];
        }
    }
 
    // 计算簇的中心
    void update_centroids() {
        vector<int> counts(K, 0);
        for (int i = 0; i < K; ++i) {
            fill(centroids[i].begin(), centroids[i].end(), 0);
        }
 
        // 累加每个簇的数据点，计算新的簇中心
        for (size_t i = 0; i < points.size(); ++i) {
            int cluster_id = labels[i];
            counts[cluster_id]++;
            for (size_t j = 0; j < points[i].size(); ++j) {
                centroids[cluster_id][j] += points[i][j];
            }
        }
 
        // 计算新的簇中心
        for (int i = 0; i < K; ++i) {
            if (counts[i] > 0) {
                for (size_t j = 0; j < centroids[i].size(); ++j) {
                    centroids[i][j] /= counts[i];
                }
            }
        }
    }
 
    // 分配每个点到最近的簇中心
    void assign_clusters() {
        for (size_t i = 0; i < points.size(); ++i) {
            double min_dist = euclidean_distance(points[i], centroids[0]);
            int min_index = 0;
            for (int j = 1; j < K; ++j) {
                double dist = euclidean_distance(points[i], centroids[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_index = j;
                }
            }
            labels[i] = min_index;
        }
    }
 
    // 运行 K-means 聚类算法
    void fit() {
        for (int iter = 0; iter < max_iter; ++iter) {
            assign_clusters();
            vector<vector<double>> prev_centroids = centroids;
            update_centroids();
 
            // 检查簇中心是否收敛
            bool converged = true;
            for (int i = 0; i < K; ++i) {
                if (euclidean_distance(prev_centroids[i], centroids[i]) > 1e-6) {
                    converged = false;
                    break;
                }
            }
 
            if (converged) {
                cout << "Converged after " << iter + 1 << " iterations." << endl;
                break;
            }
        }
    }

    int calculate_centroid_id(const vector<double>& point){
        // cout << "K=" << K << endl;
        double min_dist = euclidean_distance(point, centroids[0]);
        int min_index = 0;
        for (int j = 1; j < K; ++j) {
            double dist = euclidean_distance(point, centroids[j]);
            if (dist < min_dist) {
                min_dist = dist;
                min_index = j;
            }
        }
        return min_index;
    }

    double centroid_distance(int c1, int c2){
        return euclidean_distance(centroids[c1], centroids[c2]);
    }
 
    // 打印结果
    void print_result() const {
        cout << "Cluster centers:" << endl;
        for (int i = 0; i < K; ++i) {
            for (double val : centroids[i]) {
                cout << val << " ";
            }
            cout << endl;
        }
 
        cout << "Labels for each point:" << endl;
        for (size_t i = 0; i < points.size(); ++i) {
            cout << "Point " << i + 1 << " is in cluster " << labels[i] << endl;
        }
    }
};

#endif