cmake_minimum_required(VERSION 2.8)
project(pcdtotxt)

# 设置编译模式
set(CMAKE_BUILD_TYPE "Debug")

# 添加 PCL 和 OpenCV 库的依赖
find_package(PCL REQUIRED)
find_package(OpenCV REQUIRED)

# 添加头文件路径
include_directories(${PCL_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

# 添加源文件
add_executable(pcdtotxt pcdtotxt.cpp)

# 链接 PCL 和 OpenCV 库
target_link_libraries(pcdtotxt ${PCL_LIBRARIES} ${OpenCV_LIBS})
