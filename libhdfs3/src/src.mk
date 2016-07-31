CXXFLAGS += -I$(srcdir)/../../installed/include
CXXFLAGS += -I$(srcdir)/../../installed/include/libxml2
CXXFLAGS += -I$(srcdir)/common -I.. -I../proto
CXXFLAGS += -Wall -msse4.2 -std=c++11 
#CXXFLAGS += -O0
CXXFLAGS += -O2 -g -DNDEBUG

all: $(OBJS)
	ld -r $(OBJS) -o subobj.o

clean:
	rm -f $(GENFILES) $(OBJS) *.o
