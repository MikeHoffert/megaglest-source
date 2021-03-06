#!/bin/bash
# Use this script to build MegaGlest Embedded Library Source Code Archive for 
# a Version Release
# ----------------------------------------------------------------------------
# Written by Mark Vejvoda <mark_vejvoda@hotmail.com>
# Copyright (c) 2011 Mark Vejvoda under GNU GPL v3.0+

VERSION=$(./mg-version.sh --version)
RELEASENAME=megaglest-source-embedded
PACKAGE="$RELEASENAME-$VERSION.tar.xz"
CURRENTDIR="$(dirname $(readlink -f $0))"
RELEASEDIR="$CURRENTDIR/release/$RELEASENAME-$VERSION/megaglest-$VERSION"
SOURCEDIR="$CURRENTDIR/../../source/"
REPODIR="$CURRENTDIR/../../"

echo "Creating source package in $RELEASEDIR"

[[ -d "$RELEASEDIR" ]] && rm -rf "$RELEASEDIR"
mkdir -p "$RELEASEDIR"

mkdir -p "$RELEASEDIR/source/shared_lib/sources/libircclient/"
mkdir -p "$RELEASEDIR/source/shared_lib/include/libircclient/"
cd "$RELEASEDIR/source/shared_lib/sources/libircclient/"
#svn export --force "$SOURCEDIR/shared_lib/sources/libircclient" "$RELEASEDIR/source/shared_lib/sources/libircclient"
git archive --remote ${REPODIR} HEAD:source/shared_lib/sources/libircclient | tar x
cd "$RELEASEDIR/source/shared_lib/include/libircclient/"
#svn export --force "$SOURCEDIR/shared_lib/include/libircclient" "$RELEASEDIR/source/shared_lib/include/libircclient"
git archive --remote ${REPODIR} HEAD:source/shared_lib/include/libircclient | tar x

mkdir -p "$RELEASEDIR/source/shared_lib/sources/platform/miniupnpc/"
mkdir -p "$RELEASEDIR/source/shared_lib/include/platform/miniupnpc/"
cd "$RELEASEDIR/source/shared_lib/sources/platform/miniupnpc/"
#svn export --force "$SOURCEDIR/shared_lib/sources/platform/miniupnpc" "$RELEASEDIR/source/shared_lib/sources/platform/miniupnpc"
git archive --remote ${REPODIR} HEAD:source/shared_lib/sources/platform/miniupnpc | tar x
cd "$RELEASEDIR/source/shared_lib/include/platform/miniupnpc/"
#svn export --force "$SOURCEDIR/shared_lib/include/platform/miniupnpc" "$RELEASEDIR/source/shared_lib/include/platform/miniupnpc"
git archive --remote ${REPODIR} HEAD:source/shared_lib/include/platform/miniupnpc | tar x

#mkdir -p "$RELEASEDIR/source/shared_lib/sources/streflop/"
#mkdir -p "$RELEASEDIR/source/shared_lib/include/streflop/"
#svn export --force "$SOURCEDIR/shared_lib/sources/streflop" "$RELEASEDIR/source/shared_lib/sources/streflop"
#svn export --force "$SOURCEDIR/shared_lib/include/streflop" "$RELEASEDIR/source/shared_lib/include/streflop"

mkdir -p "$RELEASEDIR/source/masterserver/flags/"
cd "$RELEASEDIR/source/masterserver/flags/"
#svn export --force "$SOURCEDIR/masterserver/flags" "$RELEASEDIR/source/masterserver/flags"
git archive --remote ${REPODIR} HEAD:source/masterserver/flags | tar x

mkdir -p "$RELEASEDIR/data/core/fonts/"
cd "$RELEASEDIR/data/core/fonts/"
#svn export --force "$SOURCEDIR/../data/glest_game/data/core/fonts" "$RELEASEDIR/data/core/fonts"
git archive --remote ${REPODIR}/data/glest_game/ HEAD:data/core/fonts | tar x

cd "$CURRENTDIR"
echo "Creating $PACKAGE"
[[ -f "$release/$PACKAGE" ]] && rm "release/$PACKAGE"
tar cJf "release/$PACKAGE" -C "$CURRENTDIR/release/$RELEASENAME-$VERSION" "megaglest-$VERSION"

ls -la release/$PACKAGE
