cmake_minimum_required(VERSION 2.6)

project(reflect)

# // g++ -std=c++11 -o alloc alloc.cc -lgtest -lpthread && ./alloc

add_definitions(-fpermissive)
add_definitions(-ggdb3)
add_definitions(-fdebug-types-section)
add_definitions(-fno-eliminate-unused-debug-types)
add_definitions(-fdebug-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=)
add_definitions(-ftls-model=local-dynamic)

include_directories(.)
link_directories(${CMAKE_SOURCE_DIR}/build)

#add_definitions(-O3)
#add_definitions(-gdwarf-4)

add_library(${PROJECT_NAME} SHARED alloc.cc)

target_compile_options(${PROJECT_NAME} PUBLIC "-fPIC")
#target_link_libraries(${PROJECT_NAME} "-pie -Wl,-E") # requires main

add_executable(${PROJECT_NAME}_tests tst/alloc_tests.cc tst/test_main.cc)
target_link_libraries(${PROJECT_NAME}_tests gtest pthread dwarf elf dw dl ffi)
