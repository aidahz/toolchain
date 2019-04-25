set -e

function fatal
{
   echo 
   echo FATAL: "$@"
   echo
   exit 1
}

sudo mkdir -p /opt/llvm7-{release,release+assert} 
sudo chown $USER /opt/llvm7-{release,release+assert}


# llvm-7 requires python 2.7 during build
pyver=$(python -V 2>&1 | sed 's/.* \([0-9]\).\([0-9]\).*/\1\2/')
if [ "$pyver" -lt "27" ]; then
    wget https://www.python.org/ftp/python/2.7.10/Python-2.7.10.tgz
    tar xzf Python-2.7.10.tgz
    (cd Python-2.7.10 && ./configure) || fatal make-python27-configure
    (cd Python-2.7.10 && sudo make -j8 altinstall) || fatal make-python-27
    alias python=python2.7
fi


##########################
(rm -rf llvm-7.0.1.src && tar xf llvm-7.0.1.src.tar.xz &&
	cd llvm-7.0.1.src &&
	mkdir build &&
	cd build &&
	CXXFLAGS=-I$TOOLCHAIN_DIR/installed/include LDFLAGS=-L$TOOLCHAIN_DIR/installed/lib \
		cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=/opt/llvm7-release \
			-DLLVM_TARGETS_TO_BUILD=AArch64 \
			-DCMAKE_BUILD_TYPE=Release \
			-DLLVM_ENABLE_ASSERTIONS=Off \
			.. &&
	make -j8 && make install) || fatal llvm7-release


##########################
(rm -rf llvm-7.0.1.src && tar xf llvm-7.0.1.src.tar.xz && 
	cd llvm-7.0.1.src &&
	mkdir build && 
	cd build &&
	CXXFLAGS=-I$TOOLCHAIN_DIR/installed/include LDFLAGS=-L$TOOLCHAIN_DIR/installed/lib \
		cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=/opt/llvm7-release+assert \
			-DLLVM_TARGETS_TO_BUILD=AArch64 \
			-DCMAKE_BUILD_TYPE=Release \
			-DLLVM_ENABLE_ASSERTIONS=On \
			.. &&
	make -j8 && make install)  || fatal llvm7-release+assert

