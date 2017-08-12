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
(cd mendota >& /dev/null) || fatal please symlink mendota

##########################
[ "$MY_TOOLCHAIN_DIR" == "$TOOLCHAIN_DIR" ]  || fatal "please set TOOLCHAIN_DIR to this $DIR"


##########################
# rm -rf installed  # do not RM
mkdir -p installed
mkdir -p out
TARGETDIR=$DIR/installed
export PATH="$TARGETDIR/bin:$PATH"

##########################
echo -n 'viper: ..........'
(cd mendota/viper && make clean && make -j8 && make install prefix="$TARGETDIR" ) >& out/viper.out && pass || fail

##########################
echo -n 'xjit: ...........'
(cd mendota/xjit && make clean && make -j8 && make install prefix="$TARGETDIR" ) >& out/xjit.out && pass || fail

##########################
echo -n 'sparquet: .......'
(cd mendota/sparquet && make clean && make && make install prefix="$TARGETDIR" ) >& out/sparquet.out && pass || fail

##########################
echo -n 'exx: ............'
(cd mendota/exx && make clean && make -j8 \
	&& make install prefix=$TARGETDIR ) >& out/exx && pass || fail

##########################
echo -n 'libhdfs3: .......'
# Use good old Makefiles
(cd mendota/libhdfs3/src && make clean && make -j8 \
	&& make install prefix=$TARGETDIR) >& out/libhdfs3.out && pass || fail

