#!/bin/bash
#
# This script disables some code in qca-ossl.cpp that will not build
# with some old versions of openssl. 
#
# Only use this if you have problems compiling qca-ossl 
# related to EVP_whirlpool()
# 

sed -e 's/OBJ_whirlpool$/OBJ_whirlpool_dont_use/g' qca-ossl.cpp > tmp.cpp
mv tmp.cpp qca-ossl.cpp


