#!/bin/bash
#  this is replacing the function of Make
#  With the speed of today's compilers, the optimization
#  of avoiding recompile of the code is of little value
#  and the added flexibility of a script is of
#  significant value
# the build is currently targeted to 3 platforms
#  Darwin - mac osx
#  Linux  - various forms of linux
#  Mingw  - Mingw 32 on Windows
#
#  it will use different environments based upon access
#  to the libraries supplied by installing qOp from Dwave
#
#  if the libraries are present links will be made to those
#   libraries via a link
#  if the libraries are not present
#   a file with stubs to those called routines will be
#   supplied as part of the compile and link
#  the libraries will be assumed to exist if $DWAVE_HOME
#   is defined, if it is defined, and libraries don't exist
#   this will likely fail
#
#
os=$( uname | tr '[:upper:]' '[:lower:]' )
case $os in
    *darwin*) CC=clang;;
    *linux*)  CC=clang;;
    *ming*)   CC=gcc;;
    *) CC=gcc;;
esac

if [ "${1}" == "" ]
then
    # compile and link assuming qOp system not available
    env="LOCAL"
else
    # compile and link assuming qOp system is available
    env="QOP"
fi

if [ -e ${DWAVE_HOME}/libepqmi.a ]
then
    env="QOP"
fi

# always a clean build. Why not, compilers are fast enough for such a small program.

c_flags="-Wall -O3 -Wextra -std=gnu99 -I ../src -I ../cmd -I ../include "
#"-fopenmp-use-tls"

if [ "${env}" == "LOCAL" ]
then   # make a set of stubs to satisfy the qOp calls
    dw_lib=""
    link_lib=""
    epqmi_lib=""
    echo "local build"
    c_flags="${c_flags} -D LOCAL"
else
    c_flags="${c_flags} -D QOP"
    dw_lib="-L ${DWAVE_HOME}"
    epqmi_lib="-lepqmi"
    case $os in
        *darwin*) link_lib=/usr/local/lib/libdwave_sapi.dylib;;
        *linux*) link_lib=${DWAVE_HOME}/libdwave_sapi.so;;
        *ming*) link_lib=${DWAVE_HOME}/dwave_sapi.dll;;
        *) link_lib="./";;
    esac
    echo "qOp build"
fi
if [ "$os" == "ming" ]
then
    c_flags="${c_flags} -D WIN"
    echo "build for windoze"
fi


l_flags="-lm ${epqmi_lib} ${dw_lib} ${link_lib} "

echo "${CC} ${c_flags} -o qbsolv *.c ../cmd/*.c ${l_flags} "
`${CC} ${c_flags} -o qbsolv *.c ../cmd/*.c ${l_flags} `
