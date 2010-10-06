#!/bin/bash

NAME=chronicon
VERSION=0.2.5
PACKDIR=${HOME}/packaging/chronicon

makearchive.sh ${NAME}-${VERSION} master
cp ${NAME}-${VERSION}.tar.gz ${PACKDIR}
echo ${NAME} > ${PACKDIR}/pack-name
echo ${VERSION} > ${PACKDIR}/pack-version
ls -l ${PACKDIR}/${NAME}-${VERSION}.tar.gz

if [ x$1 == "xmake" ]
then
  cd ${PACKDIR}
  make
fi
