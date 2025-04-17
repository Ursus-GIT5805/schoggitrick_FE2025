.PHONY: all test clean

all:
	make cmake
	cd build && make

cmake:
	mkdir -p build
	cd build && cmake ..

run:
	make all
	build/bin/schoggitrick
