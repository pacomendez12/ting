#!/bin/sh

packageName=libting-dev

baseDir=debian/out/$packageName
mkdir -p $baseDir

#copy files
incDir=$baseDir/usr/include/ting
mkdir -p $incDir

cp src/ting/*.hpp $incDir



#create dir where the output 'control' will be placed
mkdir -p $baseDir/DEBIAN

#remove substvars
rm debian/substvars

#generate final control file
dpkg-gencontrol -p$packageName -P$baseDir

dpkg -b $baseDir ../file.deb
dpkg-name ../file.deb #rename file to proper debian format (package_version_arch.deb)
