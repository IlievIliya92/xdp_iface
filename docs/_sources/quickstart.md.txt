# QuickStart

```{contents}
:depth: 3
```

## Dependencies

* libxp
* libbpf

## Initial Setup

1. Build & install dependencies

In order to build the `xdpiface` you would need to clone and install the `libxp`
and `libbpf` shared libraries. For a Debian based machine you can start by following the instructions bellow:

```console
$ apt-get update
$ apt-get install --no-install-recommends --no-install-suggests -y git clang llvm libelf-dev m4 libpcap-dev gcc-multilib build-essential cmake linux-tools-common linux-tools-generic linux-cloud-tools-generic linux-tools-5.19.0-38-generic linux-cloud-tools-5.19.0-38-generic pkg-config
```

Additional and complete information on the setup can be found in the official [xdp-tutorial](https://github.com/xdp-project/xdp-tutorial/blob/master/setup_dependencies.org)

To build & install `libxp` and `libbpf`:

```console
$ git clone https://github.com/xdp-project/xdp-tools
$ cd xdp-tools
$ git submodule init && git submodule update
$ ./configure
$ make
$ sudo make install
$ cd lib/libbpf/src
$ sudo make install
```

```{note}
    Note that `libbpf` build artefacts may be deployed in `/usr/lib64`.
```

To add `/usr/lib64` to the `LD_LIBRARY_PATH` use the following commands:

```console
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/
$ sudo ldconfig
```

### Build & install xdpiface lib

The process to build and install the `xdpiface` is pretty standard for a cmake build
system.

```console
$ git https://github.com/IlievIliya92/xdp_iface.git
$ cd xdp_iface
$ mkdir build
$ cmake ..
$ make
$ sudo make install
```

### Validate the installation

In the resulting `build` directory there should be a test application named: `xdpiface_app`
This application simply loads the `xdp_sock_bpf.o` object to the `lo` interface and
sends a batch of packets which is received and printed on the stdout. To run the application:

```console
$ cd build
$ sudo ./xdpiface_app
```

To quit the application use `Ctrl + C`

```{note}
    In order to load the bpf program and bind the xdp socket you would need administrative privileges.
```

### Bindings usage

To test the python bindings there is a similar software which resides in the
`bindings/python/` directory. To run it use the following command:

```console
cd bindings/python/
sudo python3 xdpiface_app.py
```
