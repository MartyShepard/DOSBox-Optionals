#!/bin/bash
chmod +x configure
rm ./src/winres.o
make -j8 || exit 1