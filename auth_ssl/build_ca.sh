#!/bin/sh

# Ref: [greeter examples in C++ working under SSL](https://github.com/grpc/grpc/issues/9593#issuecomment-277946137)

script_dir=$(dirname "$(readlink -f "$0")")
cd $script_dir

rm -rf ca

mkdir ca
cd ca

# ## Generate valid CA
mkdir ca
cd ca
openssl genrsa -passout pass:1234 -des3 -out ca.key 4096
openssl req -passin pass:1234 -new -x509 -days 365 -key ca.key -out ca.crt -subj  "/C=SP/ST=Spain/L=Valdepenias/O=Test/OU=Test/CN=Root CA"
cd ..

# ## Generate valid Server Key/Cert
mkdir server
cd server
openssl genrsa -passout pass:1234 -des3 -out server.key 4096
openssl req -passin pass:1234 -new -key server.key -out server.csr -subj  "/C=SP/ST=Spain/L=Valdepenias/O=Test/OU=Server/CN=localhost"
openssl x509 -req -passin pass:1234 -days 365 -in server.csr -CA ../ca/ca.crt -CAkey ../ca/ca.key -set_serial 01 -out server.crt
openssl verify -CAfile ../ca/ca.crt server.crt

# Remove passphrase from the Server Key
openssl rsa -passin pass:1234 -in server.key -out server.key
cd ..

# ## Generate valid Client Key/Cert
mkdir client
cd client
openssl genrsa -passout pass:1234 -des3 -out client.key 4096
openssl req -passin pass:1234 -new -key client.key -out client.csr -subj  "/C=SP/ST=Spain/L=Valdepenias/O=Test/OU=Client/CN=localhost"
openssl x509 -passin pass:1234 -req -days 365 -in client.csr -CA ../ca/ca.crt -CAkey ../ca/ca.key -set_serial 01 -out client.crt
openssl verify -CAfile ../ca/ca.crt client.crt

# Remove passphrase from Client Key
openssl rsa -passin pass:1234 -in client.key -out client.key
cd ..

cd ..
