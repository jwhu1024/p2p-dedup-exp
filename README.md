# zyre-dedup   [![Build Status](https://travis-ci.org/jwhu1024/p2p-dedup-exp.svg?branch=master)](https://travis-ci.org/jwhu1024/p2p-dedup-exp)
discovery peers in local network by using zyre

# Setup
Please refer to setup.sh

# Build
1. cd js && npm install
2. cd ../ && cmake -H. -Boutput -DCMAKE_BUILD_TYPE=Debug
3. cd output && make clean && make && ./bin/p2p-dedup
