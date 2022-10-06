# project name
project("Mini-Video-Processor")

# requirement of cmake version
cmake_minimum_required (VERSION 3.8)

# set the directory of executable files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/dist/bin/debug)    
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/dist/bin/release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/dist/lib/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/dist/lib/release)
set(jsoncpp_DIR D:/vcpkg/packages/jsoncpp_x64-windows/share/jsoncpp)

# find required packages
find_package( OpenCV REQUIRED )
find_package( jsoncpp CONFIG REQUIRED )
	
# directory of headers
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

# find all sources in the directory
aux_source_directory(./src SOURCE_LIST)
message(STATUS "Sources: ${SOURCE_LIST}")


# name of executable file and path of source file
add_executable("mvp" ${SOURCE_LIST})

# link libraries
target_link_libraries("mvp" ${OpenCV_LIBS})
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
target_link_libraries("mvp" jsoncpp_lib jsoncpp_object JsonCpp::JsonCpp)