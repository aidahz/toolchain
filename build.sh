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
echo -n 'yaml: ...........'
(cd yaml-0.1.5 \
  && autoreconf --force --install \
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
echo -n 'protobuf: .......'
(cd protobuf-2.5.0 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/protobuf.out && pass || fail


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
echo -n 'apr: ............'
(cd apr-1.5.2 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/apr.out && pass || fail


##########################
echo -n 'event: ..........'
(cd libevent-2.0.22-stable && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make clean && make -j8 && make install) >& out/event.out && pass || fail

 
##########################
echo -n 'sparquet: .......'
(cd sparquet/src && make clean && make -j8 && make install) >& out/sparquet.out && pass || fail


##########################
echo -n 'intelfp: ........'
(cd IntelRDFPMathLib20U1/LIBRARY && make clean && make CC=gcc CALL_BY_REF=0 GLOBAL_RND=0 GLOBAL_FLAGS=0 UNCHANGED_BINARY_FLAGS=0 && mv libbid.a ../../installed/lib) >& out/intelfp.out && pass || fail

echo -n 'rm *.so: ........'
(cd $MY_TOOLCHAIN_DIR/installed/lib && rm -f *.so *.so.*) && pass || fail
