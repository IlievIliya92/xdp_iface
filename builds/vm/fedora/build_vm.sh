#!/bin/bash -e

# Build Fedora 37 VM & install deps
virt-builder fedora-37 -o f37vm.qcow2 --format qcow2 \
    --root-password password:fedora \
    --update --selinux-relabel --size 40G \
    --install "git,clang,llvm,m4,kernel-headers,cmake,bpftool,elfutils-libelf-devel,libpcap-devel,perf,pkg-config,python3,python3-pip" \
    --upload xdp_iface_setup.sh:/home/xdp_iface_setup.sh \
    --run-command "chmod +x /home/xdp_iface_setup.sh" \
    --run-command './home/xdp_iface_setup.sh' \
    --run-command "sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/g' /etc/ssh/sshd_config"