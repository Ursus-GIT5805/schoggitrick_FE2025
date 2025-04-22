.PHONY: all test clean

all:
	make cmake
	cd build && make

cmake:
	mkdir -p build
	cd build && cmake ..

rpi:
	mkdir -p build
	cd build && cmake .. -DRPI=1

run:
	make all
	build/bin/schoggitrick

test:
	make all
	build/bin/cv_test
