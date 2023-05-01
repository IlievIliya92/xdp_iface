cmake_minimum_required(VERSION 3.4)
project(xdpifacepy VERSION 1.0.1 DESCRIPTION "XDP iface python bindings")

add_custom_command(OUTPUT lib/_xdpiface_ctypes.py
                   COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py build
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(xdpifacepy_target ALL DEPENDS lib/_xdpiface_ctypes.py)

install(CODE "execute_process(COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py install)")
