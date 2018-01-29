#!/bin/bash 

(rm -rf grpc && git clone https://github.com/grpc/grpc \
	&& cd grpc \
	&& git submodule update --init )

(rm -fr aws-sdk-cpp && git clone https://github.com/aws/aws-sdk-cpp \
	&& cd aws-sdk-cpp \
	&& git checkout tags/1.3.48 )
