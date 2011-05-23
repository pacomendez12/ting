#!/bin/sh

packageName=libting-dev

baseDir=debian/out/$packageName
mkdir -p $baseDir

#copy files
incDir=$baseDir/usr/include/ting
mkdir -p $incDir

cp src/ting/*.hpp $incDir
cp src/ting/*.h $incDir



#create dir where the output 'control' will be placed
mkdir -p $baseDir/DEBIAN

#remove substvars
rm -f debian/substvars

#generate final control file
dpkg-gencontrol -p$packageName -P$baseDir

dpkg -b $baseDir tmp-package.deb
dpkg-name -o -s .. tmp-package.deb #rename package file to proper debian format (package_version_arch.deb)
