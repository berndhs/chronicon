#!/bin/bash

for R in central github sf
do
  git push ${R} master
done
