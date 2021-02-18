#!/bin/bash

# See https://www.gem5.org/documentation/general_docs/building

sudo apt install build-essential bison git m4 scons zlib1g zlib1g-dev \
    libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev \
    python-dev python-six python libboost-all-dev pkg-config

sudo -H pip3 install -r requirements.txt