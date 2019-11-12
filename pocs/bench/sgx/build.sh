apt-get install -y build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git
apt-get install -y libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper

git clone --depth=1 https://github.com/intel/linux-sgx.git
cd linux-sgx
./download_prebuilt.sh
git apply ../sgx-diff.patch
cd SampleCode/LocalAttestation
source /opt/intel/sgxsdk/sgxsdk/environment
make SGX_PRERELEASE=1 SGX_DEBUG=0
