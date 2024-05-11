#include <iostream>
#include <fstream>
#include <string>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <boost/filesystem.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;
using namespace pcl;

// 加载点云文件
bool loadPointCloud(const string& pcdPath, PointCloud<PointXYZI>::Ptr cloud) {
    if (io::loadPCDFile<PointXYZI>(pcdPath, *cloud) == -1) {
        cerr << "Error: Unable to load PCD file: " << pcdPath << endl;
        return false;
    }
    return true;
}

// 计算2D平面尺寸
void calculate2DPlaneSize(const PointCloud<PointXYZI>::Ptr cloud, float& minX, float& maxX, float& minY, float& maxY, float resolution) {
    for (const auto &point : cloud->points) {
        if (point.x < minX) {
            minX = point.x;
        }
        if (point.x > maxX) {
            maxX = point.x;
        }
        if (point.y < minY) {
            minY = point.y;
        }
        if (point.y > maxY) {
            maxY = point.y;
        }
    }
}

// 将点云数据映射到2D平面
void mapPointCloudTo2DPlane(const PointCloud<PointXYZI>::Ptr cloud, vector<vector<int>>& promData,
                             float minX, float minY, float minZ, float maxZ, float resolution) {
    int width = promData[0].size();
    int height = promData.size();

    for (const auto &point : cloud->points) {
        // Check if the point's Z coordinate is within the specified range
        if (point.z >= minZ && point.z <= maxZ) {
            int xIndex = static_cast<int>((point.x - minX) / resolution);
            int yIndex = static_cast<int>((point.y - minY) / resolution);
            if (xIndex >= 0 && xIndex < width && yIndex >= 0 && yIndex < height) {
                promData[yIndex][xIndex] = 0; // Mark the pixel as occupied
            }
        }
    }
}

// 将映射后的数据写入.prom.txt文件
bool writePromFileTxt(const string& ProbMapPath, const vector<vector<int>>& promData, int width, int height, float minX, float minY, float resolution) 
 {
    ofstream outputFile(ProbMapPath);
    if (!outputFile.is_open()) {
        cerr << "Error: Unable to open prom.txt for writing." << endl;
        return false;
    }
    outputFile << 15 << ' ' << resolution << ' ' << height << ' ' << width << ' ' << resolution << ' ' << minX << ' ' << minY << endl;
    for (const auto& row : promData) {
        for (int value : row) {
            outputFile << value << ' ';
        }
        outputFile << endl;
    }
    outputFile.close();
    
    cout << "prom.txt file has been successfully created." << endl;
    return true;
}
bool writeYamlFile(const string& path,float minX, float minY)
 {
    ofstream offile_y;
    offile_y.open(path,ios::out);

    offile_y<<"image: PM.pgm\n";
    offile_y<<"resolution: "<<"0.05\n";
    offile_y<<"origin: ["<<minX <<", "<< minY<<", 0.0]\n";
    offile_y<<"occupied_thresh: 0.5\n";
    offile_y<<"free_thresh: 0.5\n";
    offile_y<<"negate: 0\n";
    std::cout<<"YAML FILE SAVED, PLEASE EDIT PATH TO MAP!"<<std::endl;
    return true;
}

// 将点云映射到图像并生成pgm图像
void generatePGMImage(const PointCloud<PointXYZI>::Ptr cloud, const string& pgmPath,
                      float minX, float maxX, float minY, float maxY, float minZ,float maxZ, float resolution)
                      {
    // 计算图像宽度和高度
    int width = static_cast<int>(ceil((maxX - minX) / resolution));
    int height = static_cast<int>(ceil((maxY - minY) / resolution));

    // 创建灰度图像
    cv::Mat pgmImage(height, width, CV_8U, cv::Scalar(255));

    // 将点云映射到图像中
    for (const auto &point : cloud->points) {
        if (point.z >= minZ && point.z <= maxZ) {
            int x = static_cast<int>((point.x - minX) / resolution);
            int y = static_cast<int>((maxY - point.y) / resolution); // 使用 maxY 减去当前Y值，因为原点在左上角
            if (x >= 0 && x < width && y >= 0 && y < height) {
                // 计算灰度值（这里简单地用Z值作为灰度）
                uchar grayValue = static_cast<uchar>((point.z / maxZ) * 255); // 使用maxZ来归一化Z值
                pgmImage.at<uchar>(y, x) = grayValue;
            }
        }
    }

    // 保存pgm图像
    cv::imwrite(pgmPath, pgmImage);
    cout << "PGM image has been successfully created." << endl;
}

int main(){
    string baseDir = "/home/sft/PCDto2D/pcd/";
    string ProbMapPath = "/home/sft/PCDto2D/ProbMap.txt";
    string pgmPath = "/home/sft/PCDto2D/PM.pgm";
    string yamlPath = "/home/sft/PCDto2D/PM.yaml";
    
    float resolution = 0.05;
    float minX = numeric_limits<float>::max();
    float maxX = numeric_limits<float>::min();
    float minY = numeric_limits<float>::max();
    float maxY = numeric_limits<float>::min();
    float minZ = -0.5;
    float maxZ = 0.5;

    // 构建 2D 平面的尺寸
    for (long long num = 1; num <= 1; ++num) { 
        string pcdPath = baseDir + to_string(num) + ".pcd";
        PointCloud<PointXYZI>::Ptr cloud(new PointCloud<PointXYZI>);
        if (!loadPointCloud(pcdPath, cloud)) {
            continue;
        }
        calculate2DPlaneSize(cloud, minX, maxX, minY, maxY, resolution);
    }

    int width = static_cast<int>(ceil((maxX - minX) / resolution));
    int height = static_cast<int>(ceil((maxY - minY) / resolution));

    // 创建并初始化.prom.txt文件数据
    vector<vector<int>> promData(height, vector<int>(width, 254));

    // 将点云数据映射到2D平面
    for (long long num = 1; num <= 1; ++num) { 
        string pcdPath = baseDir + to_string(num) + ".pcd";
        PointCloud<PointXYZI>::Ptr cloud(new PointCloud<PointXYZI>);
        if (!loadPointCloud(pcdPath, cloud)) {
            continue;
        }
        mapPointCloudTo2DPlane(cloud, promData, minX, minY, minZ, maxZ, resolution);
    }

    // 将映射后的数据写入.prom.txt文件
    cout << "now start to txt" << endl;
    if (!writePromFileTxt(ProbMapPath, promData, width, height, minX, minY, resolution)) 
    {
        cout << " to txt faile" << endl;
        return -1;
    }

    //生成yaml文件
    if(!writeYamlFile(yamlPath, minX, minY))
    {
        cout << " to yaml faile" << endl;
        return -1;
    }
    // 生成PGM图像
    for (long long num = 1; num <= 1; ++num) 
    {
        string pcdPath = baseDir + to_string(num) + ".pcd";
        PointCloud<PointXYZI>::Ptr cloud(new PointCloud<PointXYZI>);
        if (!loadPointCloud(pcdPath, cloud)) {
            continue;
        }
         generatePGMImage(cloud, pgmPath, minX, maxX, minY, maxY, minZ,maxZ,resolution);
    return 0;
    }
}
