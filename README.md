# zyre-dedup 
discovery peers in local network by using zyre

# Setup
Please refer to setup.sh

# Build
cd js && npm install
cd ../ && cmake -H. -Boutput -DCMAKE_BUILD_TYPE=Debug
cd output && make clean && make && ./bin/p2p-dedup
