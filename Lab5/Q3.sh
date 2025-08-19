#! /bin/bash

sudo iptables -A INPUT -i eth0 -j ACCEPT
sudo iptables -A OUTPUT -o eth0 -j ACCEPT
sudo iptables -A INPUT -i wlan0 -j ACCEPT
sudo iptables -A OUTPUT -o wlan0 -j ACCEPT
sudo iptables -L -v -n