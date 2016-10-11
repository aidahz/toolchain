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
echo -n 'rm *.so: ........'
(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
echo -n 'libxml2: ........'
(tar xf libxml2-2.9.4.tar.gz && cd libxml2-2.9.4 \
  && ./configure --prefix=$TARGETDIR --without-python  \
  && make clean && make -j8 && make install) >& out/libxml2.out && pass || fail

##########################
echo -n 'gdal: ...........'
(F=gdal-2.1.1; rm -rf $F && tar xf $F.tar.gz && cd $F \
  && ./configure --prefix=$TOOLCHAIN_DIR/installed \
  && make clean && make -j8  \
  && make install ) >& out/gdal.out && pass || fail


##########################
echo -n 'proj: ...........'
(F=proj.4-4.9.3; rm -rf $F && tar xf $F.tar.gz && cd $F \
  && mkdir -p build && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TOOLCHAIN_DIR/installed .. \
  && make -j8  \
  && make install ) >& out/proj.out && pass || fail


##########################
echo -n 'geos: ...........'
(F=geos-3.4.2; rm -rf $F && tar xf $F.tar.gz && cd $F \
  && mkdir -p build && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TOOLCHAIN_DIR/installed .. \
  && make -j8  \
  && make install ) >& out/geos.out && pass || fail

