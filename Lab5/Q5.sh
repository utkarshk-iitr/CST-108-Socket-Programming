#! /bin/bash

sudo iptables -A OUTPUT -p tcp --dport 80 -m string --string "facebook" -j DROP
sudo iptables -A INPUT -p tcp --sport 80 -m string --string "facebook" -j DROP
sudo iptables -A OUTPUT -p tcp --dport 443 -m string --string "facebook" -j DROP
sudo iptables -A INPUT -p tcp --sport 443 -m string --string "facebook" -j DROP
