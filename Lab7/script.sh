#!/bin/bash

# This command generates a 2048-bit RSA private key and saves it into the file server.key
openssl genrsa -out server.key 2048

# This command creates a Certificate Signing Request (CSR) using the private key and the configuration file
openssl req -new -key server.key -out server.csr -config config.conf

# This command generates a self-signed SSL certificate using the CSR and the private key
# The certificate is valid for 365 days and will include the extensions defined in the configuration file
openssl req -x509 -key server.key -in server.csr -out server.crt -days 365 -extensions v3_req -config config.conf -copy_extensions none

echo "SSL certificate has been created"