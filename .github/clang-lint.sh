#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
# Install clang-format (if not already installed)
#command -v clang-format >/dev/null 2>&1 || sudo apt-get -y install clang-format
command -v clang-format || sudo apt-get -y install clang-format
clang-format --version
sudo apt-get update
sudo apt-cache search clang-format
sudo apt-get -y install clang-format
clang-format --version
# Check clang-format output
for f in **/*.{h,c,hpp,cpp,ino} ; do
    if [ -f "$f" ] && [[ "$f" != "tests/"* ]]; then
        echo "################################################################"
        echo "Checking file ${f}"
        diff $f <(clang-format -assume-filename=main.cpp $f) 1>&2
        echo -e "################################################################\n"
    fi
done

Ubuntu clang-format version 18.1.3 (1ubuntu1)
Ubuntu clang-format version 18.1.3 (1ubuntu1)
