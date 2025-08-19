#! /bin/bash

sudo iptables -I OUTPUT -p icmp -j LOG --log-prefix "ICMP_OUT: " --log-level 6
sudo iptables -I INPUT -p icmp -j LOG --log-prefix "ICMP_IN: " --log-level 6

sudo iptables -A OUTPUT -p icmp -j ACCEPT
sudo iptables -A INPUT -p icmp -j ACCEPT