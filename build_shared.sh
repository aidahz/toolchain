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



##########################
[ "$MY_TOOLCHAIN_DIR" == "$TOOLCHAIN_DIR" ]  || fatal "please set TOOLCHAIN_DIR to this $DIR"


##########################
# rm -rf installed  # do not RM
mkdir -p installed
mkdir -p out
TARGETDIR=$DIR/installed
export PATH="$TARGETDIR/bin:$PATH"

##########################
start 'rm *.so: ........'
(cd $TARGETDIR/lib && rm -f *.so *.so.*) && pass || fail

##########################
start 'libxml2: ........'
(tar xf libxml2-2.9.4.tar.gz && cd libxml2-2.9.4 \
  && ./configure --prefix=$TARGETDIR --without-python  \
  && make clean && make -j8 && make install) >& out/libxml2.out && pass || fail

##########################
start 'proj: ...........'
(F=proj.4-4.9.3; rm -rf $F && tar xf $F.tar.gz && cd $F \
  && mkdir -p build && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGETDIR .. \
  && make -j8  \
  && make install ) >& out/proj.out && pass || fail


##########################
start 'geos: ...........'
(F=geos-3.4.2; rm -rf $F && tar xf $F.tar.gz && cd $F \
  && mkdir -p build && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGETDIR .. \
  && make -j8  \
  && make install ) >& out/geos.out && pass || fail

##########################
start 'gdal: ...........'
(F=gdal-2.1.1; rm -rf $F && tar xf $F.tar.gz && cd $F \
  && ./configure --prefix=$TARGETDIR --with-xml2=$TARGETDIR/bin/xml2-config \
	--with-geos=$TARGETDIR/bin/geos-config \
  && make clean && make -j8  \
  && make install ) >& out/gdal.out && pass || fail

