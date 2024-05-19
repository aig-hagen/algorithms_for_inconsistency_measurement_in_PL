cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS OFF)

project(InconsistencyMeasureAlgorithms
  VERSION
    1.0
  DESCRIPTION
    "Implementations of algorithms for inconsistency measurement"
)

add_executable(im_app main.cpp)
add_subdirectory(src/)
# Add SAT solver library
add_subdirectory(lib/cadical)
# Add ASP solver library
add_subdirectory(lib/clingo)

# Add MaxSAT solver library
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/EvalMaxSAT/lib/EvalMaxSAT)

# Add include path of EvalMaxSAT
list(APPEND im_app_includes lib/EvalMaxSAT/lib/EvalMaxSAT/src)

# Add include path of EvalMaxSAT dependencies
list(APPEND im_app_includes lib/EvalMaxSAT/lib/MaLib/src)
list(APPEND im_app_includes lib/EvalMaxSAT/lib/glucose/src)

target_include_directories(im_app SYSTEM BEFORE PRIVATE ${im_app_includes})
message(STATUS ${im_app_includes})
message("----- INCLUDE PATHS: ${im_app_includes}")

target_link_libraries(im_app cadical libclingo EvalMaxSAT)
