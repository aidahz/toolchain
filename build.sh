MY_TOOLCHAIN_DIR=$PWD
# (cd snappy-1.1.2; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed; make; make install)
# (cd gperftools-2.4; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed --enable-frame-pointers; make; make install)
echo "Making lz4 ..."
(cd lz4-r127/lib; make; PREFIX=$MY_TOOLCHAIN_DIR/installed make install)
echo "Making rapidjson ..."
(cp -r rapidjson/include/rapidjson $MY_TOOLCHAIN_DIR/installed/include)
echo "Making gtest ..." 
(cd gtest-1.7.0; ./configure; make)
echo "Making protobuf ..."
(cd protobuf-2.5.0; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed; make; make install)
echo "Making gflags ..."
(cd gflags-2.1.2; cmake -DCMAKE_INSTALL_PREFIX=$MY_TOOLCHAIN_DIR/installed .; make; make install)
echo "Making glog ..."
(cd glog-0.3.4; ./configure --prefix=$MY_TOOLCHAIN_DIR/installed; make; make install)
