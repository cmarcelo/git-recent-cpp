#!/usr/bin/bash

cd "$(dirname "$(realpath "$0")")"

if [ -d abseil-cpp ]; then
  old_hash=$(cat abseil-cpp/ORIGINAL_HASH)
  mv abseil-cpp abseil-cpp-${old_hash}
fi

tmp_dir=$(mktemp -d)

git clone --depth 1 --single-branch https://github.com/abseil/abseil-cpp $tmp_dir
git -C ${tmp_dir} archive --format=tar --prefix=abseil-cpp/ HEAD | tar xf -
git -C ${tmp_dir} rev-parse HEAD > abseil-cpp/ORIGINAL_HASH
