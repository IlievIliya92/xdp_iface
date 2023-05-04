#!/bin/bash

virt-install --name f37vm --ram 4096 \
    --disk path=f37vm.qcow2,format=qcow2,cache=writeback \
    --nographics --os-variant fedora35 --import \
    --network bridge=virbr0 \
    --noautoconsole

virsh list
