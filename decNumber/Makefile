OBJS = decSingle.o decDouble.o decQuad.o decNumber.o decContext.o

CFLAGS = -Wall -g -O2

libdecnumber.a: $(OBJS)
	ar -rcs libdecnumber.a $(OBJS)

clean:
	rm -f libdecnumber.a $(OBJS)

install:
	mkdir -p ../installed/include/decnumber
	cp *.h ../installed/include/decnumber/ 
	cp libdecnumber.a ../installed/lib/
