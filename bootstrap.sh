#!/bin/bash 

(rm -rf grpc && git clone -b v1.7.x https://github.com/grpc/grpc \
	&& cd grpc \
	&& git submodule update --init )
