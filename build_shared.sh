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


function pass 
{
   echo '[ok]'
}

function fail
{
   echo '[fail]'
   if [ $1 ]; then echo "$@"; fi
}

##########################
[ "$MY_TOOLCHAIN_DIR" == "$TOOLCHAIN_DIR" ]  || fatal "please set TOOLCHAIN_DIR to this $DIR"


##########################
# rm -rf installed  # do not RM
mkdir -p installed
mkdir -p out
TARGETDIR=$DIR/installed
export PATH="$TARGETDIR/bin:$PATH"

##########################
echo -n 'proj: ...........'
(tar xf proj.4-4.9.3.tar.gz && cd proj.4-4.9.3 \
  && mkdir -p build && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TOOLCHAIN_DIR/installed .. \
  && make -j8  \
  && make install ) >& out/geos.out && pass || fail


##########################
echo -n 'geos: ...........'
(tar xf geos-3.4.2.tar.gz && cd geos-3.4.2 \
  && mkdir -p build && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TOOLCHAIN_DIR/installed .. \
  && make -j8  \
  && make install ) >& out/geos.out && pass || fail

