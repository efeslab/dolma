#!/bin/bash

sudo apt install build-essential bison git m4 scons zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    python3-dev python3-six python-is-python3 libboost-all-dev pkg-config python2 python2-dev

sudo -H pip3 install -r requirements.txt