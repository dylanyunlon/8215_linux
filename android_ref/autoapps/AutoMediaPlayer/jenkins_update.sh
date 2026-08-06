#!/bin/bash 

set +e  # 禁用错误退出 

git submodule update --init --recursive

git checkout origin/COMMON
git pull origin COMMON

cd app-language
git checkout origin/COMMON_app-language
git pull origin COMMON_app-language

cd ../app-overlay
git checkout origin/COMMON_app-overlay
git pull origin COMMON_app-overlay

cd ../

