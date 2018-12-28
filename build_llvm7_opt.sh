set -e

function fatal
{
   echo 
   echo FATAL: "$@"
   echo
   exit 1
}


rm -rf llvm-7.0.1.src
rm -rf llvm
tar xJf llvm-7.0.1.src.tar.xz
ln -f -s llvm-7.0.1.src llvm

sudo mkdir -p /opt/llvm7-{release,release+assert} 
sudo chown $USER /opt/llvm7-{release,release+assert}

##########################
(rm -rf llvm-7.0.1.src && tar xf llvm-7.0.1.src.tar.xz &&
	cd llvm-7.0.1.src &&
	mkdir build &&
	cd build &&
	CXXFLAGS=-I$TOOLCHAIN_DIR/installed/include LDFLAGS=-L$TOOLCHAIN_DIR/installed/lib \
		cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=/opt/llvm7-release \
			-DLLVM_TARGETS_TO_BUILD=X86 \
			-DCMAKE_BUILD_TYPE=Release \
			-DLLVM_ENABLE_ASSERTIONS=Off \
			.. &&
	make -j8 && make install) || fatal llvm7-release

