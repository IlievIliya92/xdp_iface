#!/bin/bash -e

USER=root
USER_PASS=toor
IP="192.168.122.2"

echo "Running XDP Iface classes seltests"
sshpass -p $USER_PASS ssh "$USER@$IP" "/home/xdp_iface_work_dir/xdp_iface/build/xdpiface_selftest"
echo "Running C++ bindings tests"
sshpass -p $USER_PASS ssh "$USER@$IP" "/home/xdp_iface_work_dir/xdp_iface/build/bindings/cpp/xdpiface_app_cpp"
echo "Running Python bindings tests"
sshpass -p $USER_PASS ssh "$USER@$IP" "/usr/bin/python3 /home/xdp_iface_work_dir/xdp_iface/bindings/python/xdpiface_app.py"
echo "Running Go bindings tests"
sshpass -p $USER_PASS ssh "$USER@$IP" "cd /home/xdp_iface_work_dir/xdp_iface/bindings/go/xdpiface/ && go test -c && ./xdpiface.test"
echo "Running RUST bindings tests"
sshpass -p $USER_PASS ssh "$USER@$IP" "cd /home/xdp_iface_work_dir/xdp_iface/bindings/rust//libxdpiface-sys/ && cargo test"

sudo virsh shutdown f37vm
until sudo virsh domstate f37vm | grep shut; do
    sleep 10
done
