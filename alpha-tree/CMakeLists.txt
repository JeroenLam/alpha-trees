cmake_minimum_required(VERSION 3.23.3)
project( AlphaTree )
find_package( OpenCV 4 REQUIRED )
add_executable( ${PROJECT_NAME} main.cpp )
include_directories(${OpenCV_INCLUDE_DIRS} source)
link_directories( source )
add_subdirectory( source )
target_include_directories(${PROJECT_NAME} PUBLIC source)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} SalienceTree )
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
