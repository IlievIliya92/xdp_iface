cmake_minimum_required(VERSION 3.4)
project(xdp_sock_bpf VERSION 1.0.1 DESCRIPTION "XDP BPF socket")

set(CLANG_FLAGS -g -O2 -target bpf)
set(CLANG_COMPILER_WARNING -Wall -Wno-unused-value -Wno-pointer-sign -Wno-compare-distinct-pointer-types -Werror)

set(EBPF_IFACE xdp_sock_bpf)
add_custom_command(OUTPUT  ${EBPF_IFACE}.o
                   COMMAND clang ${CLANG_FLAGS} ${COMPILER_WARNING} -c ${CMAKE_CURRENT_SOURCE_DIR}/${EBPF_IFACE}.c -o ${EBPF_IFACE}.o
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
add_custom_target(${EBPF_IFACE}_target ALL DEPENDS ${EBPF_IFACE}.o)

set (prefix "${CMAKE_INSTALL_PREFIX}")
set (exec_prefix "\${prefix}")
set (libdir "\${prefix}/lib${LIB_SUFFIX}")

install(
    FILES "${CMAKE_SOURCE_DIR}/build/${EBPF_IFACE}.o"
    DESTINATION "${libdir}/bpf/"
)
