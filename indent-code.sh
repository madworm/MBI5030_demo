#!/bin/bash

find ./ -regex "^.*\.\(cpp\|h\|ino\)$" -exec indent -linux {} \;
