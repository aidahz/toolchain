CXXFLAGS := -std=c++11 -O3 -mavx2 -Wall -I.

OBJS := $(addprefix highwayhash/, \
	sip_hash.o \
	sip_tree_hash.o \
	scalar_sip_tree_hash.o \
	highway_tree_hash.o \
	scalar_highway_tree_hash.o \
	sse41_highway_tree_hash.o \
	sip_hash_main.o)

MAIN := sip_hash_main

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(MAIN) -lrt

libhighwayhash.a: $(OBJS)
	ar -rcs libhighwayhash.a $(OBJS)

install: libhighwayhash.a
	mkdir -p ../installed/include/highwayhash
	cp highwayhash/*.h ../installed/include/highwayhash/
	cp libhighwayhash.a ../installed/lib


.PHONY: clean
clean:
	$(RM) $(OBJS) $(MAIN) *.a
