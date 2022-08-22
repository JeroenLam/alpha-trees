set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
find_package( OpenCV 4 REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(gabor gabor.cpp gabor.h)
