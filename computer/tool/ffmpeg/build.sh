#!/bin/bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
./configure --prefix=/usr/local/ffmpeg --enable-libx264 --enable-shared --enable-static   --enable-libx264 --enable-gpl --enable-pthreads --enable-libfreetype  --enable-avfilter --enable-libfontconfig --enable-libass
make -j4 && sudo make install
