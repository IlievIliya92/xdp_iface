#!/bin/bash -e

# Install libxdp & libbpf
mkdir /home/xdp_iface_work_dir/
cd /home/xdp_iface_work_dir/
git clone https://github.com/xdp-project/xdp-tools
cd xdp-tools
git submodule init && git submodule update
./configure
make && make install
cd lib/libbpf/src && make install

cd /home/xdp_iface_work_dir/
git clone https://github.com/IlievIliya92/xdp_iface.git
cd xdp_iface
mkdir build && cd build
cmake -DPYTHON_BINDINGS=ON ..
make

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/
ldconfig
