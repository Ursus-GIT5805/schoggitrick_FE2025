.PHONY: all test clean

all:
	make cmake
	cd build && make

cmake:
	mkdir -p build
	cd build && cmake ..

rpi:
	mkdir -p build
	cd build && cmake ..

run:
	make all
	build/bin/schoggitrick

test:
	make all
	build/bin/cv_test

release:
	mkdir -p build
	cd build && cmake --config Release ..
	cd build && make
