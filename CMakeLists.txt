cmake_minimum_required (VERSION 2.6)

# setting project name
project(p2p-dedup)

# compiler option for debug mode (binary about 315K)
SET(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -D__DEBUG__")

# compiler option for release mode (binary about 7.9K)
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

# setting header path
include_directories(
	${CMAKE_SOURCE_DIR}/inc
)

link_directories(
	${LIB_DIR}
)

# compiler flags
add_definitions(-Wall -Wundef
		-Wstrict-prototypes
		-Wno-trigraphs
		-fno-inline
		-fno-short-enums
		-fpic)

# output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./bin")

# generate binary
add_executable(p2p-dedup
	./src/p2p-main.c
	./src/p2p-cmd-handler.c
	./src/p2p-event-handler.c
	./src/p2p-encrypt.c
	./src/p2p-common.c
)

# setting library to link
target_link_libraries(p2p-dedup libzyre.so libzmq.so libczmq.so libpthread.so librt.so libcrypto.so)
