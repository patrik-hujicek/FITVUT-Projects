#!/bin/bash

echo -n "shell pid[$$]: "
cat /proc/$$/status | grep SigCgt
echo
time ping6 -c 1 -w 1 2001:67c:1220:80c:21a:4aff:fe97:5e13
