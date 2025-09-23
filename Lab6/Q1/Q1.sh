#!/bin/bash

openssl rand -hex 7 > des_key

KEY7=$(cat des_key)
KEY8=${KEY7}00
KEY24=${KEY8}${KEY8}${KEY8}
# Although we should compute the parity bits but for simplicity we are not doing that
# We are appending the 7 byte key with an additional byte of 00 to make it 8 bytes
# Since we need a 24 byte key for 3DES, so we are also repeating the 8 byte key 3 times

openssl des3 -e -in Alice.txt -out Alice.enc -K $KEY24 -iv 0000000000000000
openssl des3 -d -in Alice.enc -out Alice.dec -K $KEY24 -iv 0000000000000000
diff -qs Alice.txt Alice.dec

# Yes both the files 'Alice.txt' and 'Alice.dec' are same.