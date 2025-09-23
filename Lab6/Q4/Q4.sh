#!/bin/bash

g++ base64_enc_dec.cpp -o base64_enc_dec
./base64_enc_dec -e < Alice.txt > out.pem

openssl base64 -d -in out.pem -out out2.txt
diff -qs Alice.txt out2.txt

# Yes, both the files are same

openssl base64 -in in.txt -out out3.txt

./base64_enc_dec -d < out3.txt > out4.txt
diff -qs in.txt out4.txt

# Yes, both the files are same