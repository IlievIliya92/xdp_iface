---
hide-toc: true
---

# XDP Interface

[![Fedora Test Build](https://github.com/IlievIliya92/xdp_iface/actions/workflows/fedora_test_build_install.yml/badge.svg)](https://github.com/IlievIliya92/xdp_iface/actions/workflows/fedora_test_build_install.yml)

The purpouse of this documentation is to provide guidance on how to install and use
the xdpiface shared library.

The project consits of:

* XDP interface library written in C
* Higher level language bindings: python3
* Cmake build system
* Documentaion

## Project Goals

The goal of the xdpiface is to provide an easy to use simple library which allows
basic input/output of data over eBPF programs. It is based on the guidance and tutorials
provided by the [xdp-project](https://github.com/xdp-project). Usefull information
on the `libxdp` usage and its internals can be found [here](https://www.mankier.com/3/libxdp).
Information on how to develop and use `eBPF` programs is available under the official Linux
[kernel achives](https://www.kernel.org/doc/html/latest/bpf/libbpf/index.html).


```{toctree}
:caption: Usage
:hidden:

quickstart
api/index
examples
```

```{toctree}
:caption: Development
:hidden:

contributing/index
changelog
license
```

```{toctree}
:caption: Support
:hidden:

support
```
