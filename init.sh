#!/usr/bin/env sh

set -e

OLD_DIR=$(pwd)
cd $(git rev-parse --show-toplevel)

# don't accidentally push to the boilerplate repo again
rm -rf .git
git init

# add vendored copy of raylib to this source tree
git clone https://github.com/raysan5/raylib ./vendor/raylib
echo "https://github.com/raysan5/raylib $(cd ./vendor/raylib && git rev-parse HEAD)" >> ./vendor/hashes.txt
# don't make it a submodule
rm -rf ./vendor/raylib/.git

cd $OLD_DIR
