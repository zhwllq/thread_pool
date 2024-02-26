project(threadpool)                                                                                                                                          
cmake_minimum_required(VERSION 3.22)                                                                                                                         
                                                                                                                                                             
## Generate an threadpool test executable file                                                                                                               
set(OBJNAME thread_pool_test)                                                                                                                                
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}")                                                                                                          
add_executable(${OBJNAME} thread_pool_test.cpp thread_pool.cpp)                                                                                              
target_link_libraries(${OBJNAME} pthread)
