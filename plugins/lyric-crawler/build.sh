#!/bin/sh

echo 'Build Baidu Crawler Module....'
gcc -o crawler_baidu.so crawler_baidu.c \
    `pkg-config --cflags --libs glib-2.0 gmodule-2.0 libcurl` -Wall -O2 \
    -shared -fPIC -export-dynamic

echo 'Build TTPlayer Crawler Module....'
gcc -o crawler_ttplayer.so crawler_ttplayer.c \
    `pkg-config --cflags --libs glib-2.0 gmodule-2.0 libcurl` -Wall -O2 \
    -shared -fPIC -export-dynamic

echo 'Build LRC123 Crawler Module....'
gcc -o crawler_lrc123.so crawler_lrc123.c \
    `pkg-config --cflags --libs glib-2.0 gmodule-2.0 libcurl` -Wall -O2 \
    -shared -fPIC -export-dynamic
