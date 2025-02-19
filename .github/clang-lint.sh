#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
# Install clang-format
sudo apt-get -y install clang-format
# Check clang-format output
for f in **/*.{h,c,hpp,cpp,ino} ; do
    if [ -f "$f" ]; then
        echo "Checking file ${f}"
        diff $f <(clang-format -assume-filename=main.cpp $f) 1>&2
    fi
done
