#!/bin/bash
if [ $# -ne 0 ]
then
#mp3库
#启用--enable-shared --enable-libmp3lame
sudo yum install lame lame-devel lame-libs -y
#启用字幕 --enable-libass
sudm yum install libass-devel -y
fi

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
cd ffmpeg-5.0/

./configure --prefix=/usr/local/ffmpeg --enable-libx264 --enable-shared --enable-static   --enable-libx264 --enable-gpl --enable-pthreads --enable-libfreetype  --enable-avfilter --enable-libfontconfig --enable-libmp3lame --enable-libass
 
make -j4 && sudo make install

