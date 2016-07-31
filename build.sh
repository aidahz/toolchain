set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MY_TOOLCHAIN_DIR=$DIR

function fatal
{
   echo
   echo ERROR: $1
   echo
   exit 1
}


function pass 
{
   echo '[ok]'
}

function fail
{
   echo '[fail]'
   if [ $1 ]; then echo $*; fi
}



##########################
(cd sparquet >& /dev/null) || fatal 'please symlink sparquet'


##########################
[ "$MY_TOOLCHAIN_DIR" == "$TOOLCHAIN_DIR" ]  || fatal "please set TOOLCHAIN_DIR to this $DIR"


##########################
rm -rf installed
mkdir installed
mkdir -p out
TARGETDIR=$DIR/installed
export PATH="$TARGETDIR/bin:$PATH"
export PKG_CONFIG_PATH="$TARGETDIR/lib/pkgconfig:$PKG_CONFIG_PATH"


##########################
echo -n 'event: ..........'
HINT=
if [ -e /opt/local/include/openssl ] && ! [ -e /usr/local/include/openssl ] ; then
    HINT='HINT: try (cd /usr/local/include; ln -s /opt/local/include/openssl)'
fi
(cd libevent-2.0.22-stable && ./configure --prefix=$TARGETDIR && make clean && make -j8 && make install) >& out/event.out && pass || fail $HINT

##########################
echo -n 'cmake: ..........'
(cd cmake-3.5.2 \
  && ./configure --prefix=$TARGETDIR  \
  && make clean && make -j8 && make install) >& out/cmake.out && pass || fail

##########################
echo -n 'googletest: .....'
(cd googletest && rm -rf build && mkdir build && cd build \
	&& cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGETDIR .. \
	&& make all install) >& out/googletest.out && pass || fail


##########################
echo -n 'bzip2: ..........'
(cd bzip2-1.0.6 \
  && make clean && make -j8  \
  && make install PREFIX=$TARGETDIR) >& out/bzip2.out && pass || fail

##########################
echo -n 'curl: ...........'
(cd curl-7.49.0 \
  && ./configure --prefix=$TARGETDIR --enable-shared=no \
  && make clean && make -j8 && make install) >& out/curl.out && pass || fail

##########################
echo -n 'yaml: ...........'
(cd yaml-0.1.5 \
  && (autoreconf --force --install || true) \
  && ./configure --prefix=$TARGETDIR --enable-shared=no \
  && make clean && make -j8 && make install) >& out/yaml.out && pass || fail


##########################
# (cd gperftools-2.4; ./configure --prefix=$TARGETDIR --enable-frame-pointers; make clean; make -j8 ; make install)


##########################
echo -n 'lz4: ............'
(cd lz4-r129/lib && make clean && make -j8 && PREFIX=$TARGETDIR make install) >& out/lz4.out && pass || fail

##########################
echo -n 'rapidjson: ......'
(cp -r rapidjson/include/rapidjson $TARGETDIR/include) >& out/rapidjson.out && pass || fail



##########################
echo -n 'gflags: .........'
(cd gflags-2.1.2 && cmake -DCMAKE_INSTALL_PREFIX=$TARGETDIR . && make clean && make -j8 && make install) >& out/gflags.out && pass || fail


##########################
echo -n 'glog: ...........'
(cd glog-0.3.4 && ./configure --prefix=$TARGETDIR && make clean && make -j8 && make install) >& out/glog.out && pass || fail


##########################
# we don't need snappy at this moment.
# echo "Making snappy ..."
# (cd snappy-1.1.1 && ./configure --with-pic --prefix=$TARGETDIR && make clean && make -j8 && make install) >& out/snappy.out && pass || fail


##########################
echo -n 'decNumber: ......'
(cd decNumber && make clean && make -j8 && make install) >& out/decnumber.out && pass || fail

##########################
echo -n 'highwayhash: ....'
(cd highwayhash && make clean && make -j8 && make install) >& out/highwayhash.out && pass || fail

##########################
echo -n 're2: ............'
(cd re2-master && make clean && make -j8 && make install) >& out/re2.out && pass || fail


##########################
echo -n 'apr: ............'
(cd apr-1.5.2 && ./configure --prefix=$TARGETDIR && make clean && make -j8 && make install) >& out/apr.out && pass || fail


##########################
echo -n 'intelfp: ........'
(cd IntelRDFPMathLib20U1/LIBRARY && make clean && make CC=gcc CFLAGS=-O3 CALL_BY_REF=0 GLOBAL_RND=0 GLOBAL_FLAGS=0 UNCHANGED_BINARY_FLAGS=0 -j8 && mv libbid.a ../../installed/lib) >& out/intelfp.out && pass || fail

##########################
echo -n 'rm *.so: ........'
(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
echo -n 'sparquet: .......'
(cd sparquet/src && make clean && make -j8 && make install) >& out/sparquet.out && pass || fail


##########################
(cd xdrive >& /dev/null) || fatal 'please symlink xdrive'


##########################
echo -n 'protobuf: .......'
(cd protobuf-3.0.0 \
	&& ./autogen.sh \
	&& ./configure --prefix=$TARGETDIR --enable-shared=no \
	&& make clean && make -j8 \
	&& make install) >& out/protobuf.out && pass || fail

##########################
echo -n 'grpc: ...........'
(cd grpc && make clean \
	&& make -j8 prefix=$TARGETDIR \
	&& make prefix=$TARGETDIR install) >& out/grpc.out && pass || fail

##########################
echo -n 'rm *.so: ........'
(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
echo -n 'libgsasl: .......'
(cd libgsasl-1.8.0 \
  && ./configure --prefix=$TARGETDIR --enable-shared=no \
  && make clean && make -j8 && make install) >& out/libgsasl.out && pass || fail


##########################
echo -n 'libuuid: ........'
(cd libuuid-1.0.3 \
  && ./configure --prefix=$TARGETDIR --enable-shared=no \
  && make clean && make -j8 && make install) >& out/libuuid.out && pass || fail


##########################
echo -n 'kerboros: .......'
(cd krb5-1.14.2/src \
  && ./configure --prefix=$TARGETDIR --enable-static --disable-shared \
  && make clean && make -j8 && make install) >& out/krb.out && pass || fail

##########################
# DON'T NEED THIS SHIT
#echo -n 'boost: ..........'
#(cd boost_1_61_0 \
#  && ./bootstrap.sh --with-python=no --prefix=$TARGETDIR \
#  && ./b2 install) >& out/boost.out && pass || fail


##########################
echo -n 'libxml2: ........'
(cd libxml2-2.9.4 \
  && ./configure --prefix=$TARGETDIR --without-python --enable-shared=no \
  && make clean && make -j8 && make install) >& out/libxml2.out && pass || fail

##########################
#echo -n 'rm *.so: ........'
#(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
echo -n 'libhdfs3: .......'
# Use good old Makefiles
( (cd libhdfs3/src && make clean && make -j8) \
	&& cp libhdfs3/src/libhdfs3.a installed/lib/ \
	&& cp libhdfs3/src/client/hdfs.h installed/include/ ) >& out/libhdfs3.out && pass || fail

##########################
echo -n 'rm *.so: ........'
(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
echo -n 'xdrive: .........'
( (cd xdrive && make clean && make -j8) \
	&& mkdir -p installed/include/xdrive \
	&& cp xdrive/client/xdrclnt.h installed/include/xdrive/ \
	&& cp xdrive/server/xdrive installed/bin/ \
	&& cp xdrive/client/libxdrive.a installed/lib ) >& out/xdrive.out && pass || fail


