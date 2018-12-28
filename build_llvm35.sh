set -e 

function fatal
{
   echo 
   echo FATAL: "$@"
   echo
   exit 1
}


rm -rf llvm-3.5.2.src
rm -f llvm
tar xJf llvm-3.5.2.src.tar.xz
ln -f -s llvm-3.5.2.src llvm

sudo rmdir /opt/llvm-release+assert
sudo mkdir -p /opt/llvm-release+assert
sudo chown $USER /opt/llvm-release+assert
rm -rf build
mkdir -p build
(cd build &&
   ../llvm/configure --prefix=/opt/llvm-release+assert \
	--enable-targets=x86_64 --enable-optimized=YES &&
   make clean && make -j8 && sudo make install) || fatal llvm-release+assert



sudo rmdir /opt/llvm-release
sudo mkdir -p /opt/llvm-release
sudo chown $USER /opt/llvm-release
rm -rf build
mkdir -p build
(cd build && 
    ../llvm/configure --prefix=/opt/llvm-release \
	--enable-targets=x86_64 --enable-optimized=YES --enable-assertions=NO &&
    make clean && make -j8 && sudo make install) || fatal llvm-release
 
