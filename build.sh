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


##########################
echo -n 'event: ..........'
HINT=
if [ -e /opt/local/include/openssl ] && ! [ -e /usr/local/include/openssl ] ; then
    HINT='HINT: try (cd /usr/local/include; ln -s /opt/local/include/openssl)'
fi
(cd libevent-2.0.22-stable && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/event.out && pass || fail $HINT

##########################
echo -n 'cmake: ..........'
(cd cmake-3.5.2 \
  && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed  \
  && make clean && make -j8 && make install) >& out/cmake.out && pass || fail

##########################
echo -n 'bzip2: ..........'
(cd bzip2-1.0.6 \
  && make clean && make -j8  \
  && make install PREFIX=$MY_TOOLCHAIN_DIR/installed) >& out/bzip2.out && pass || fail

##########################
echo -n 'curl: ...........'
(cd curl-7.49.0 \
  && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed --enable-shared=no \
  && make clean && make -j8 && make install) >& out/curl.out && pass || fail

##########################
echo -n 'yaml: ...........'
(cd yaml-0.1.5 \
  && (autoreconf --force --install || true) \
  && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed --enable-shared=no \
  && make clean && make -j8 && make install) >& out/yaml.out && pass || fail


##########################
# (cd gperftools-2.4; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed --enable-frame-pointers; make clean; make -j8 ; make install)


##########################
echo -n 'lz4: ............'
(cd lz4-r129/lib && make clean && make -j8 && PREFIX=$MY_TOOLCHAIN_DIR/installed make install) >& out/lz4.out && pass || fail


##########################
echo -n 'rapidjson: ......'
(cp -r rapidjson/include/rapidjson $MY_TOOLCHAIN_DIR/installed/include) >& out/rapidjson.out && pass || fail


##########################
echo -n 'gtest: ..........'
(cd gtest-1.7.0 && ./configure && make clean && make -j8 ) >& out/gtest.out && pass || fail


##########################
# disable protobuf. expect it to come with grpc that
# echo -n 'protobuf: .......'
# (cd protobuf-2.5.0 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/protobuf.out && pass || fail
# 

##########################
echo -n 'gflags: .........'
(cd gflags-2.1.2 && cmake -DCMAKE_INSTALL_PREFIX=$MY_TOOLCHAIN_DIR/installed . && make clean && make -j8 && make install) >& out/gflags.out && pass || fail


##########################
echo -n 'glog: ...........'
(cd glog-0.3.4 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/glog.out && pass || fail


##########################
# we don't need snappy at this moment.
# echo "Making snappy ..."
# (cd snappy-1.1.1 && ./configure --with-pic --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/snappy.out && pass || fail


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
(cd apr-1.5.2 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/apr.out && pass || fail

 
##########################
echo -n 'sparquet: .......'
(cd sparquet/src && make clean && make -j8 && make install) >& out/sparquet.out && pass || fail


##########################
echo -n 'intelfp: ........'
(cd IntelRDFPMathLib20U1/LIBRARY && make clean && make CC=gcc CFLAGS=-O3 CALL_BY_REF=0 GLOBAL_RND=0 GLOBAL_FLAGS=0 UNCHANGED_BINARY_FLAGS=0 && mv libbid.a ../../installed/lib) >& out/intelfp.out && pass || fail

echo -n 'rm *.so: ........'
(cd $MY_TOOLCHAIN_DIR/installed/lib && rm -f *.so *.so.*) && pass || fail
