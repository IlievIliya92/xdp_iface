cmake_minimum_required(VERSION 3.9)
project(xdpifacecpp VERSION 1.1.0 DESCRIPTION "XDP Iface CPP bindings")

include(GNUInstallDirs)
set(SRC_FILES src/xdp_iface.cpp src/xdp_sock.cpp)
add_library(xdpifacecpp SHARED ${SRC_FILES})
target_link_libraries(xdpifacecpp PUBLIC xdpiface)

target_include_directories(xdpifacecpp PUBLIC include)
target_include_directories(xdpifacecpp PRIVATE src)

#### Test App
add_executable(xdpiface_app_cpp src/xdp_iface_app.cpp)

target_include_directories(xdpiface_app_cpp PRIVATE src)
target_link_libraries(xdpiface_app_cpp PUBLIC xdpiface)
target_link_libraries(xdpiface_app_cpp PUBLIC xdpifacecpp)


set(HEADER_FILES include/xdpiface.hpp)
install(FILES ${HEADER_FILES} DESTINATION includedir)
set_target_properties(xdpifacecpp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/xdpiface.hpp)
configure_file(xdpifacecpp.pc.in xdpifacecpp.pc @ONLY)

install(TARGETS xdpifacecpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xdpifacecpp.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


