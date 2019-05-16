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

#sudo rm -rf /opt/llvm35-release+assert
#sudo mkdir -p /opt/llvm35-release+assert
#sudo chown $USER /opt/llvm35-release+assert
#rm -rf build
#mkdir -p build
#(cd build &&
#   ../llvm/configure --prefix=/opt/llvm35-release+assert \
#	--enable-targets=x86_64 --enable-optimized=YES &&
#   make clean && make -j8 && sudo make install) || fatal llvm-release+assert


sudo rm -rf /opt/llvm35-release
sudo mkdir -p /opt/llvm35-release
sudo chown $USER /opt/llvm35-release
rm -rf build
mkdir -p build
(cd build && 
    ../llvm/configure --prefix=/opt/llvm35-release \
	--enable-targets=x86_64 --enable-optimized=YES --enable-assertions=NO &&
    make clean && make -j8 && sudo make install) || fatal llvm-release
 
