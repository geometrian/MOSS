echo "============GCC============"
cd /usr/src/build-gcc
export PATH=$PATH:$PREFIX/bin
../gcc-4.7.2/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
make all-gcc
make install-gcc