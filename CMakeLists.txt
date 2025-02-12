cmake_minimum_required(VERSION 3.10)
project(SocketApp)

set(CMAKE_CXX_STANDARD 17)


find_package(Qt5 REQUIRED COMPONENTS Widgets)


add_executable(server server.cpp)
target_link_libraries(server Qt5::Widgets)  


add_executable(client client.cpp)
target_link_libraries(client Qt5::Widgets)  