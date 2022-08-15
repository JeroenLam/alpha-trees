add_library(EdgeQueue EdgeQueue.cpp EdgeQueue.h)
add_library(SalienceTree SalienceTree.cpp SalienceTree.h)
target_link_libraries(SalienceTree EdgeQueue)
