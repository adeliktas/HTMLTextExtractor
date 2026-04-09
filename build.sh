#!/bin/bash

# find notepad-plus-plus/ -type f -name '*.h' -exec cp {} HTMLTextExtractor_plugin/ \;

# Check what headers are actually used:
# x86_64-w64-mingw32-g++ -H -c HTMLTextExtractor.cpp -o temp.o 2> includes.txt

arch=$(uname -m)
if [[ $arch == x86_64* ]]; then
	echo "X64 Architecture"
	x86_64-w64-mingw32-g++ -shared -o HTMLTextExtractor.dll HTMLTextExtractor.cpp -static-libgcc -static-libstdc++ -DUNICODE -D_UNICODE
elif [[ $arch == i*86 ]]; then
	echo "X32 Architecture"
	i686-w64-mingw32-g++ -shared -o HTMLTextExtractor.dll HTMLTextExtractor.cpp -static-libgcc -static-libstdc++ -DUNICODE -D_UNICODE
elif  [[ $arch == arm* ]]; then
    echo "ARM Architecture"
fi

# install
mkdir ~/.wine/drive_c/Program\ Files/Notepad++/plugins/HTMLTextExtractor
cp HTMLTextExtractor.dll ~/.wine/drive_c/Program\ Files/Notepad++/plugins/HTMLTextExtractor/

