#!/bin/bash -e

# Test script

IP="192.168.122.2"

wait-for-it "$IP:22" -t 900 -s -- echo ready

set -x
sshpass -p fedora ssh -o "StrictHostKeyChecking=no" "root@$IP" uname -a
sshpass -p fedora ssh "root@$IP" "/home/xdp_iface_work_dir/xdp_iface/build/xdpiface_selftest"

sudo virsh shutdown f37vm
until sudo virsh domstate f37vm | grep shut; do
    sleep 10
done
