cmake_minimum_required(VERSION 2.5)

project(mymuduo)

#mymuduo 最终编译好的so文件放在根目录的lib里 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

#定义 参与编译的源代码文件
aux_source_directory(. SRC_LIST )

#编译生成动态库
add_library(mymuduo SHARED ${SRC_LIST})