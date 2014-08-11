#!/usr/bin/env bash

# builds the cmake files into the build directory

set -e

if [ -d "./build/" ]; then
	echo "Build folder found, cleaning up..."
	rm -rf -- build/
	mkdir build
	cd build
	cmake ../
else
	echo "Build not found, making directory..."
	mkdir -p ./build/
	cd ./build/
	cmake ../
fi
