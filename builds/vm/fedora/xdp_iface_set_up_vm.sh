# Test script
ifconfig virbr0

USER=root
USER_PASS=toor
IP="192.168.122.2"

wait-for-it "$IP:22" -t 900 -s -- echo ready

set -x
sshpass -p $USER_PASS ssh -o "StrictHostKeyChecking=no" "$USER@$IP" uname -a

# Update xdp iface repo (This will be moved to a separate step)
sshpass -p $USER_PASS ssh "$USER@$IP" "/bin/bash /home/xdp_iface_setup.sh"
