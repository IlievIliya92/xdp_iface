FROM ubuntu
ARG DEBIAN_FRONTEND=noninteractive
MAINTAINER "Iliya Iliev"
#Install git
RUN apt-get update && \
     apt-get install --no-install-recommends --no-install-suggests -y git clang llvm libelf-dev m4 libpcap-dev gcc-multilib build-essential cmake linux-tools-common linux-tools-generic linux-cloud-tools-generic linux-tools-5.19.0-38-generic linux-cloud-tools-5.19.0-38-generic pkg-config clang python3 python3-pip && \
     pip install sphinx==4.3.2 sphinx_rtd_theme

# Install libxdp & libbpf
RUN mkdir /home/xdp_iface && \
     cd /home/xdp_iface && \
     git clone https://github.com/xdp-project/xdp-tools && \
     cd xdp-tools && \
     git submodule init && git submodule update && \
     ./configure && \
     make && make install && \
     cd lib/libbpf/src && make install

RUN cd /home/xdp_iface && \
     git clone https://github.com/IlievIliya92/xdp_iface.git && \
     cd xdp_iface && \
     mkdir build && cd build && \
     cmake .. && \
     make

ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/
RUN ldconfig

#Set working directory
WORKDIR /home/xdp_iface
CMD /bin/bash
