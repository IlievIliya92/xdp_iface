#!/bin/bash

VM_DISK_IMAGE="./f37vm.qcow2"

# Build Fedora 37 VM & install deps
virt-builder fedora-37 -o $VM_DISK_IMAGE --format qcow2 \
    --root-password password:toor \
    --timezone "`cat /etc/timezone`" \
    --update --selinux-relabel \
    --install "git,clang,llvm,m4,kernel-headers,cmake,bpftool,elfutils-libelf-devel,libpcap-devel,perf,pkg-config,golang,rust,cargo,python3,python3-pip" \
    --upload xdp_iface_setup.sh:/home/xdp_iface_setup.sh \
    --run-command "chmod +x /home/xdp_iface_setup.sh" \
    --run-command "sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/g' /etc/ssh/sshd_config" \
    --run-command "sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/g' /etc/ssh/sshd_config" \
    --append-line '/root/.bashrc:export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/' \
    --append-line '/root/.bashrc:export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib'

# Enable compression on disk
mv "$VM_DISK_IMAGE" "$VM_DISK_IMAGE.old"
qemu-img convert -O qcow2 -c "$VM_DISK_IMAGE.old" "$VM_DISK_IMAGE"
rm -f "$VM_DISK_IMAGE.old"

