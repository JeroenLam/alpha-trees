set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
find_package( OpenCV 4 REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(SalienceTree SalienceTree.cpp SalienceTree.h)

add_library(DistanceFunction INTERFACE)
set_target_properties(DistanceFunction PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(DistanceFunction INTERFACE .)

add_library(TreeFilter INTERFACE)
set_target_properties(TreeFilter PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(TreeFilter INTERFACE .)

target_link_libraries(SalienceTree ${OpenCV_LIBS} DistanceFunction)
