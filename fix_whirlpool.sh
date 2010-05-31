#!/bin/bash

sed -e 's/OBJ_whirlpool$/OBJ_whirlpool_dont_use/g' qca-ossl.cpp > tmp.cpp
mv tmp.cpp qca-ossl.cpp


