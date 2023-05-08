#!/bin/bash -e

# Test script
ifconfig virbr0

IP="192.168.122.2"

wait-for-it "$IP:22" -t 900 -s -- echo ready

set -x
sshpass -p toor ssh -o "StrictHostKeyChecking=no" "root@$IP" uname -a

sshpass -p toor ssh "root@$IP" "/home/xdp_iface_work_dir/xdp_iface/build/xdpiface_selftest"
sshpass -p toor ssh "root@$IP" "/usr/bin/python3 /home/xdp_iface_work_dir/xdp_iface/bindings/python/xdpiface_app.py"
sshpass -p toor ssh "root@$IP" "cd /home/xdp_iface_work_dir/xdp_iface/bindings/go/xdpiface/ && go test -c && ./xdpiface.test"

sudo virsh shutdown f37vm
until sudo virsh domstate f37vm | grep shut; do
    sleep 10
done
