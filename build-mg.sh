#!/bin/bash
# Use this script to build MegaGlest using cmake
# ----------------------------------------------------------------------------
# Written by Mark Vejvoda <mark_vejvoda@hotmail.com>
# Copyright (c) 2011 Mark Vejvoda under GNU GPL v3.0+

LANG=C
NUMCORES=`cat /proc/cpuinfo | grep -cE '^processor'`

mkdir -p build
cd build

if [ -f 'CMakeCache.txt' ]; then rm -f 'CMakeCache.txt'; fi

# This is for regular developers and used by our installer
# For more cmake/build options refer to 
# http://wiki.megaglest.org/Linux_Compiling#Building_using_CMake_by_Hand
cmake -DCMAKE_INSTALL_PREFIX='' -DWANT_STATIC_LIBS=ON ..
if [ $? -ne 0 ]; then 
  echo 'ERROR: CMAKE failed.' >&2; exit 1
fi

make -j$NUMCORES
if [ $? -ne 0 ]; then
  echo 'ERROR: MAKE failed.' >&2; exit 2
fi

cd ..
echo ''
echo 'BUILD COMPLETE.'
echo ''
echo 'To launch MegaGlest from the current directory, use:'
echo '  mk/linux/megaglest --ini-path=mk/linux/ --data-path=mk/linux/'
echo 'Or change into mk/linux and run it from there:'
echo '  ./megaglest --ini-path=./ --data-path=./'
