#!/bin/bash
ROOT_PATH=$(pwd)

export LD_LIBRARY_PATH=$ROOT_PATH/lib; ./httpd 192.168.234.131 8081
