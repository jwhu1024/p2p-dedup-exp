# p2p-proto
discovery peers in local network by using zyre

# Setup
1. install zeromq
 1. wget http://download.zeromq.org/zeromq-4.1.4.tar.gz
 2. tar xvf zeromq-4.1.4.tar.gz && cd zeromq-4.1.4
 3. sudo apt-get install pkg-config libtool build-essential autoconf automake uuid-dev
 4. ./configure --without-libsodium
 5. make
 6. sudo make install
 7. sudo ldconfig
2. install czmq
 1. wget https://github.com/zeromq/czmq/archive/v3.0.2.tar.gz
 2. tar xvf v3.0.2.tar.gz && cd czmq-3.0.2
 3. ./autogen.sh
 4. ./configure
 5. make -j 4
 6. make check
 7. sudo make install
 8. sudo ldconfig

# Build
1. cd p2p-proto && cmake -H. -Boutput -DCMAKE_BUILD_TYPE=[Debug|Release]
2. cd output && make
3. ./run.sh
