MY_TOOLCHAIN_DIR=$PWD

if ! (cd sparquet); then
   echo Please symlink sparquet 
   exit 1
fi


# (cd gperftools-2.4; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed --enable-frame-pointers; make ; make install)

echo "Making lz4 ..."
(cd lz4-r127/lib && make && PREFIX=$MY_TOOLCHAIN_DIR/installed make install) >& lz4.out && echo ok || echo failed

echo "Making rapidjson ..."
(cp -r rapidjson/include/rapidjson $MY_TOOLCHAIN_DIR/installed/include) >& rapidjson.out && echo ok || echo failed

echo "Making gtest ..." 
(cd gtest-1.7.0 && ./configure && make) >& gtest.out && echo ok || echo failed

echo "Making protobuf ..."
(cd protobuf-2.5.0 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make && make install) >& protobuf.out && echo ok || echo failed

echo "Making gflags ..."
(cd gflags-2.1.2 && cmake -DCMAKE_INSTALL_PREFIX=$MY_TOOLCHAIN_DIR/installed . && make && make install) >& gflags.out && echo ok || echo failed

echo "Making glog ..."
(cd glog-0.3.4 && ./configure --prefix=$MY_TOOLCHAIN_DIR/installed && make && make install) >& glog.out && echo ok || echo failed

# we don't need snappy at this moment.
# echo "Making snappy ..."
# (cd snappy-1.1.1 && ./configure --with-pic --prefix=$MY_TOOLCHAIN_DIR/installed && make && make install) >& snappy.out && echo ok || echo failed

echo "Making decNumber ..."
(cd decNumber && make clean && make && make install) >& decnumber.out && echo ok || echo failed

echo "Making sparquet ..."
(cd sparquet/src && make clean && make install) >& sparquet.out && echo ok || echo failed

