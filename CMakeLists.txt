cmake_minimum_required(VERSION 3.20)
project(llvm-ct)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

add_library(CTPass MODULE
  src/ArgDepAnalysis.cpp
  src/CTBranchPass.cpp
  src/Plugin.cpp
)

target_include_directories(CTPass PRIVATE ${LLVM_INCLUDE_DIRS} include)
target_compile_definitions(CTPass PRIVATE ${LLVM_DEFINITIONS_LIST})

target_compile_features(CTPass PRIVATE cxx_std_17)
target_compile_options(CTPass PRIVATE -Wall -Wextra -Wconversion)

set_target_properties(CTPass PROPERTIES PREFIX "")
