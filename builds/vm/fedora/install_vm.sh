#!/bin/bash

virsh list
ifconfig virbr0

virt-install --name f37vm --ram 8192 \
    --vcpus "${VCPUS:-8}" \
    --disk path=f37vm.qcow2,format=qcow2,cache=writeback \
    --nographics --os-variant fedora35 --import \
    --network default \
    --noautoconsole

virsh list
