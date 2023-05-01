cmake_minimum_required(VERSION 3.4)
project(xdpifacerust VERSION 1.0.1 DESCRIPTION "XDP iface rust bindings")

add_custom_target(xdpifacerust_target ALL
    COMMAND sudo cargo build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libxdpiface-sys)
