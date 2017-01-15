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

rm -rf $MY_TOOLCHAIN_DIR/llvm


##########################
start 'llvm debug: .....'
# NB: this is actually a Release build with assertions ON
(rm -rf llvm-3.9.1.src && tar xf llvm-3.9.1.src.tar.xz && cd llvm-3.9.1.src \
	&& rm -rf build \
	&& mkdir build \
	&& cd build \
	&& cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=$MY_TOOLCHAIN_DIR/llvm/debug \
		-DLLVM_TARGETS_TO_BUILD=X86 \
		-DCMAKE_BUILD_TYPE=Release \
		-DLLVM_ENABLE_ASSERTIONS=On \
		.. \
	&& (make -j8  || make -j8 || make) \
	&& make install) >& out/llvm-debug.out && pass || fail

##########################
start 'llvm release: ...'
(rm -rf llvm-3.9.1.src && tar xf llvm-3.9.1.src.tar.xz && cd llvm-3.9.1.src \
	&& rm -rf build \
	&& mkdir build \
	&& cd build \
	&& cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX=$MY_TOOLCHAIN_DIR/llvm/release \
		-DLLVM_TARGETS_TO_BUILD=X86 \
		-DCMAKE_BUILD_TYPE=Release \
		.. \
	&& (make -j8  || make -j8 || make) \
	&& make install) >& out/llvm-release.out && pass || fail
