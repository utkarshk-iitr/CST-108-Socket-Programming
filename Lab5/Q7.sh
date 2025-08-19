#! /bin/bash

sudo iptables -A INPUT -p tcp --dport 22 -m conntrack --ctstate NEW -m recent --name SSH --rcheck --seconds 60 --hitcount 3 --rsource -j DROP