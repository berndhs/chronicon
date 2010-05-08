#!/bin/bash

for R in central github sf
do
  echo "push to ${R}"
  git push ${R} master
done
