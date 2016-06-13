# zyre-dedup 
discovery peers in local network by using zyre

# Setup
Please refer to setup.sh

# Build
1. cd test && cmake -H. -Boutput -DCMAKE_BUILD_TYPE=[Debug|Release]
2. cd output && make clean && make && ./bin/p2p-dedup 
