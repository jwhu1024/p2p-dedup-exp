#!/bin/bash

function setup_linux_package ()
{
	apt-get install python-software-properties -y
	#add-apt-repository ppa:ubuntu-toolchain-r/test
	#apt-get update
	apt-get install g++-4.8 gcc-4.8 -y
	update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50
	cd /tmp && wget https://bootstrap.pypa.io/get-pip.py
	python get-pip.py
	apt-get install nodejs npm libgmp3-dev openssl libssl-dev pkg-config libgmp-dev lzip curl libtool autoconf -y
}

function setup_nodejs ()
{
    # apt-get install nodejs

<<BUILDFROMSRC
    cd /tmp
	wget -v https://nodejs.org/dist/v4.4.1/node-v4.4.1.tar.gz
	tar xvf node-v4.4.1.tar.gz
	cd node-v4.4.1
	./configure
	make
	make install
	node -v
	npm -v
BUILDFROMSRC
}

function setup_zeromq ()
{
	cd /tmp
	git clone git://github.com/jedisct1/libsodium.git
	cd libsodium
	./autogen.sh && ./configure && make check
	
	make install
	ldconfig
	
	cd /tmp
	git clone git://github.com/zeromq/libzmq.git
	cd libzmq
	./autogen.sh && ./configure && make check
	make install
	ldconfig
	
	cd /tmp
	git clone git://github.com/zeromq/czmq.git
	cd czmq
	./autogen.sh && ./configure && make check
	make install
	ldconfig

	cd /tmp
	git clone git://github.com/zeromq/zyre.git
	cd zyre
	./autogen.sh && ./configure && make check
	make install
	ldconfig
}

function setup_httpie ()
{
	pip install --upgrade pip setuptools
	pip install --upgrade httpie
}

function setup_node_dep ()
{
    cd js && npm install
}

#function setup_my_project ()
#{
#	cd ~
#	git clone https://github.com/jwhu1024/test.git
#	cd test
#	./build_cmd
#}

setup_linux_package

python --version
node -v
npm -v
curl --version
#setup_nodejs
#setup_zeromq
#setup_httpie
#setup_my_project
