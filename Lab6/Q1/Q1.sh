#!/bin/bash

openssl rand -hex 8 > des_key
openssl des3 -e -in Alice.txt -out Alice.enc -K $(cat des_key)$(cat des_key)$(cat des_key) -iv 0000000000000000
openssl des3 -d -in Alice.enc -out Alice.dec -K $(cat des_key)$(cat des_key)$(cat des_key) -iv 0000000000000000
diff Alice.txt Alice.dec

# Yes both the files 'Alice.txt' and 'Alice.dec' are same.