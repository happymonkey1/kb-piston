@echo off
cd ./vendor/v8
fetch
git pull && gclient sync
