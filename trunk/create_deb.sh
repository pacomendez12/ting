#!/bin/sh

dst="debian_package/usr/include/ting"

mkdir -p $dst

cp src/ting/*.hpp $dst

cp -r DEBIAN debian_package

dpkg -b debian_package libting-dev_0.3.0-1_i386.deb
