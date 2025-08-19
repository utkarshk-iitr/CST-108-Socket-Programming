#! /bin/bash

sudo bash -c "echo 1 > /proc/sys/net/ipv4/ip_forward"
sudo iptables -t nat -A PREROUTING -p tcp --dport 108 -j REDIRECT --to-port 22
sudo iptables -A INPUT -p tcp --dport 108 -j ACCEPT