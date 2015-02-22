echo "============BINUTILS============"
cd /usr/src/build-binutils

#../binutils-2.23.1/configure --target=$TARGET --prefix=$PREFIX --disable-nls

#http://cross-lfs.org/view/svn/x86_64-64/cross-tools/binutils.html
../binutils-2.23/configure --prefix=$PREFIX --host=${CLFS_HOST} --target=${CLFS_TARGET} --with-sysroot=${CLFS} --disable-nls --enable-shared --disable-static --enable-64-bit-bfd --disable-multilib --enable-plugins

#make -j4
make all
make install