#!/bin/bash
set -e
#------------------------------------------------------------------------------
# cleanup any previously created files
rm -rf keys

#------------------------------------------------------------------------------
# create a certificate for the ESP (hostname: "myesp")

mkdir keys
cd keys

# create a private key
openssl genrsa -des3 -out myCert.key 2048
# create certificate signing request
cat > myCert.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = US
ST = State
L = Location
O = MyCompany
CN = hostname.local
EOF
openssl req -new -x509 -days 36500 -key myCert.key -config myCert.conf -out myCert.crt

# convert private key and certificate into DER format
openssl rsa -in myCert.key -outform DER -out myCert.key.DER
openssl x509 -in myCert.crt -outform DER -out myCert.crt.DER

# create header files
echo "#ifndef CERT_H_" > ./cert.h
echo "#define CERT_H_" >> ./cert.h
xxd -i myCert.crt.DER >> ./cert.h
echo "#endif" >> ./cert.h

echo "#ifndef PRIVATE_KEY_H_" > ./private_key.h
echo "#define PRIVATE_KEY_H_" >> ./private_key.h
xxd -i myCert.key.DER >> ./private_key.h
echo "#endif" >> ./private_key.h

echo ""
echo "Certificates created!"
echo "---------------------"
echo ""
echo "  Private key:      private_key.h"
echo "  Certificate data: cert.h"
echo ""
echo "Make sure to have both files available for inclusion when running the examples."
echo "The files have been copied to all example directories, so if you open an example"
echo " sketch, you should be fine."
