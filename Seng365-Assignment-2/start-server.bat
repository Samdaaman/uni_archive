@echo off
start C:\Windows\System32\OpenSSH\ssh.exe -v -t -L3306:db2.csse.canterbury.ac.nz:3306 -R 1340:localhost:4200 sho116@linux.cosc.canterbury.ac.nz "ssh -v -L 0.0.0.0:4200:localhost:1340 localhost"
timeout 1
cd ..
cd assignment2-server
node server.js
