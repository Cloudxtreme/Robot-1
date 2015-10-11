#!/bin/bash
#
#
mkdir ARDUINO
cd ARDUINO
git init
git remote add origin https://github.com/dreamshader/Arduino
git pull origin master
git config --global user.email "dreamshader@gmx.de"
git config --global user.name "dreamshader"
cd ..
#
mkdir LINUX
cd LINUX
git init
git remote add origin https://github.com/dreamshader/Linux
git pull origin master
git config --global user.email "dreamshader@gmx.de"
git config --global user.name "dreamshader"
cd ..
#
mkdir RASPBERRY_PI
cd RASPBERRY_PI
git init
git remote add origin https://github.com/dreamshader/Raspberry_PI
git pull origin master
git config --global user.email "dreamshader@gmx.de"
git config --global user.name "dreamshader"
cd ..
#

