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


(cd sparquet >& /dev/null) || fatal "please symlink sparquet"

[ "$MY_TOOLCHAIN_DIR" == "$TOOLCHAIN_DIR" ]  || fatal "please set TOOLCHAIN_DIR to this $DIR"


# (cd gperftools-2.4; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed --enable-frame-pointers; make -j8 ; make install)

echo "Making lz4 ..."
(cd lz4-r127/lib && make -j8 && PREFIX=$MY_TOOLCHAIN_DIR/installed make install) >& lz4.out && echo ok || echo failed

echo "Making rapidjson ..."
(cp -r rapidjson/include/rapidjson $MY_TOOLCHAIN_DIR/installed/include) >& rapidjson.out && echo ok || echo failed

echo "Making gtest ..." 
(cd gtest-1.7.0 && ./configure && make -j8 ) >& gtest.out && echo ok || echo failed

echo "Making protobuf ..."
(cd protobuf-2.5.0 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make -j8 && make install) >& protobuf.out && echo ok || echo failed

echo "Making gflags ..."
(cd gflags-2.1.2 && cmake -DCMAKE_INSTALL_PREFIX=$MY_TOOLCHAIN_DIR/installed . && make -j8 && make install) >& gflags.out && echo ok || echo failed

echo "Making glog ..."
(cd glog-0.3.4 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make -j8 && make install) >& glog.out && echo ok || echo failed

# we don't need snappy at this moment.
# echo "Making snappy ..."
# (cd snappy-1.1.1 && ./configure --with-pic --prefix=$MY_TOOLCHAIN_DIR/installed && make -j8 && make install) >& snappy.out && echo ok || echo failed

echo "Making decNumber ..."
(cd decNumber && make clean && make -j8 && make install) >& decnumber.out && echo ok || echo failed

echo "Making apr ..."
(cd apr-1.5.2 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make -j8 && make install) >& apr.out && echo ok || echo failed

echo "Making libevent ..."
(cd libevent-2.0.22-stable && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make -j8 && make install) >& apr.out && echo ok || echo failed

echo "Making sparquet ..."
(cd sparquet/src && make clean && make -j8 && make install) >& sparquet.out && echo ok || echo failed

