set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MY_TOOLCHAIN_DIR=$DIR


function fatal
{
   echo
   echo ERROR: "$@"
   echo
   exit 1
}


STARTTIME=0
function start
{
   echo -n $1
   STARTTIME=$(date +%s)
}


function pass 
{
   ENDTIME=$(date +%s)
   ELAPSED=$(($ENDTIME - $STARTTIME))
   echo "[ok  $ELAPSED sec]"
}

function fail
{
   echo '[fail]'
   if [ $1 ]; then echo "$@"; fi
}



##########################
(cd mendota >& /dev/null) || fatal please symlink mendota
# (cd grpc >& /dev/null) || fatal please run bootstrap.sh to download grpc

##########################
[ "$MY_TOOLCHAIN_DIR" == "$TOOLCHAIN_DIR" ]  || fatal "please set TOOLCHAIN_DIR to $DIR"
TARGETDIR=$DIR/installed
export PATH="$TARGETDIR/bin:$PATH"
mkdir -p out

##########################
rm -rf installed 
mkdir -p installed

##########################
start 'protobuf: .......'
rm -rf protobuf-3.5.1
(tar xfz protobuf-all-3.5.1.tar.gz && cd protobuf-3.5.1 \
    && ./configure --prefix=$TARGETDIR --enable-shared=no \
    && make clean \
    && make -j8 \
    && make install) >& out/protobuf.out && pass || fail

##########################
start 'tomlc99: ........'
(cd tomlc99 && make clean && make install prefix=$TARGETDIR) >& out/tomlc99.out && pass || fail

##########################
start 'event: ..........'
HINT=
if [ -e /opt/local/include/openssl ] && ! [ -e /usr/local/include/openssl ] ; then
    HINT='HINT: try (cd /usr/local/include; ln -s /opt/local/include/openssl)'
fi
(cd libevent-2.0.22-stable && ./configure --prefix=$TARGETDIR && make clean && make -j8 && make install) >& out/event.out && pass || fail $HINT

##########################
# start 'cmake: ..........'
if (cmake3 >& /dev/null); then 
    ln -s $(which cmake3) $TARGETDIR/bin/cmake
elif ! (cmake --version 2>&1 | grep 'version 3'); then 
    (cd cmake-3.5.2 && ./configure --prefix=$TARGETDIR &&
            make clean && make -j8 && make install) >& out/cmake.out && pass || fail
fi

##########################
start 'googletest: .....'
(cd googletest && rm -rf build && mkdir build &&
        cd build && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGETDIR .. &&
	make all install) >& out/googletest.out && pass || fail


##########################
start 'bzip2: ..........'
(cd bzip2-1.0.6 && make clean &&
        make -j8 && make install PREFIX=$TARGETDIR) >& out/bzip2.out && pass || fail

##########################
# start 'openssl: ........'
# (cd openssl-1.0.2k && ./config --prefix=$TARGETDIR/openssl shared \
#     && make clean && make && make install) >& out/openssl.out && pass || fail

##########################
start 'curl: ...........'
rm -rf curl-7.59.0
(tar xfz curl-7.59.0.tar.gz && cd curl-7.59.0 &&
        ./configure --prefix=$TARGETDIR --enable-shared=no \
                    --with-ssl --without-librtmp --disable-ldap --disable-ldaps &&
        make clean && make -j8 && make install) >& out/curl.out && pass || fail

##########################
start 'yaml: ...........'
rm -rf yaml-0.1.7
(tar xfz yaml-0.1.7.tar.gz && cd yaml-0.1.7 &&
        ./configure --prefix=$TARGETDIR --enable-shared=no &&
        make clean && make -j8 && make install) >& out/yaml.out && pass || fail

##########################
start 'libuuid: ........'
rm -rf libuuid-1.0.3
(tar xfx libuuid-1.0.3.tar.gz && cd libuuid-1.0.3 &&
        ./configure --prefix=$TARGETDIR --enable-shared=no &&
        make clean && make -j8 && make install) >& out/libuuid.out && pass || fail


##########################
start 'aws: ............'
(cd aws-sdk-cpp-master && rm -fr build && mkdir build && cd build &&
        cmake -DCMAKE_INSTALL_PREFIX=$TOOLCHAIN_DIR/installed -DCMAKE_BUILD_TYPE=Release \
              -DBUILD_ONLY="s3" -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF .. &&
        make && make install) >& out/aws.out && pass || fail

##########################
# (cd gperftools-2.4; ./configure --prefix=$TARGETDIR --enable-frame-pointers; make clean; make -j8 ; make install)


##########################
start 'lz4: ............'
(cd lz4-r129/lib && make clean && make -j8 &&
        make install PREFIX=$TARGETDIR) >& out/lz4.out && pass || fail

##########################
start 'zstd: ...........'
(cd zstd-master && make clean && make -j8 &&
        make install PREFIX=$TARGETDIR) >& out/zstd.out && pass || fail


##########################
start 'rapidjson: ......'
(cp -r rapidjson/include/rapidjson $TARGETDIR/include) >& out/rapidjson.out && pass || fail



##########################
start 'gflags: .........'
(cd gflags-2.1.2 && cmake -DCMAKE_INSTALL_PREFIX=$TARGETDIR . &&
        make clean && make -j8 && make install) >& out/gflags.out && pass || fail


##########################
start 'glog: ...........'
(cd glog-0.3.4 && ./configure --prefix=$TARGETDIR &&
        make clean && make -j8 && make install) >& out/glog.out && pass || fail


##########################
# we don't need snappy at this moment.
# echo "Making snappy ..."
# (cd snappy-1.1.1 && ./configure --with-pic --prefix=$TARGETDIR && make clean && make -j8 && make install) >& out/snappy.out && pass || fail


##########################
start 'decNumber: ......'
(cd decNumber && make clean && make -j8 && make install) >& out/decnumber.out && pass || fail

##########################
start 'highwayhash: ....'
# note: do not do make all; only make install works. 
(cd highwayhash \
    && make clean \
    && make -j8 \
    && make install) >& out/highwayhash.out && pass || fail

##########################
start 're2: ............'
(cd re2-master && make clean &&
        make -j8 && make install) >& out/re2.out && pass || fail


##########################
start 'apr: ............'
(cd apr-1.5.2 && ./configure --prefix=$TARGETDIR &&
        make clean && make -j8 && make install) >& out/apr.out && pass || fail


##########################
start 'intelfp: ........'
(cd IntelRDFPMathLib20U1/LIBRARY && make clean &&
        make -j8 CC=gcc CFLAGS=-O3 CALL_BY_REF=0 GLOBAL_RND=0 \
             GLOBAL_FLAGS=0 UNCHANGED_BINARY_FLAGS=0 &&
        mv libbid.a ../../installed/lib) >& out/intelfp.out && pass || fail

##########################
start 'rm *.so: ........'
(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
start 'libgsasl: .......'
(cd libgsasl-1.8.0 && ./configure --prefix=$TARGETDIR \
	--with-gssapi-impl=mit \
	--enable-shared=no &&
        make clean && make -j8 && make install) >& out/libgsasl.out && pass || fail

##########################
start 'gss: ............'
rm -rf gss-1.0.3
(tar xfz gss-1.0.3.tar.gz && cd gss-1.0.3 &&
        ./configure --prefix=$TARGETDIR --enable-shared=no &&
        make clean && make -j8 && 
	make install)  >& out/libgss.out && pass || fail


##########################
 start 'kerboros: .......'
 (cd krb5-1.14.3/src \
   && ./configure --prefix=$TARGETDIR --enable-static --disable-shared \
   && make clean && make -j8 && make install) >& out/krb.out && pass || fail

##########################
# DON'T NEED THIS SHIT
#echo -n 'boost: ..........'
#(cd boost_1_61_0 \
#  && ./bootstrap.sh --with-python=no --prefix=$TARGETDIR \
#  && ./b2 install) >& out/boost.out && pass || fail

##########################
echo 
echo '*** shared ***'
echo 
bash build_shared.sh

##########################
echo 
echo '*** local ***'
echo 
bash build_local.sh

