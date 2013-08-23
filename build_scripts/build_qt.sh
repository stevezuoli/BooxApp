#./src/corelib/global/qconfig-minimal.h
#change configure to disable ipv6, I can not find the parameter can be passed to configure script

# make distclean

echo "yes" | ./configure -prefix ${PREFIX} -release -shared -no-largefile -no-exceptions -no-accessibility -stl -no-qt3support -no-phonon -no-svg  -xplatform qws/linux-arm-g++  -embedded arm -qt-zlib -qt-gif  -qt-libtiff  -qt-libpng -qt-libmng -qt-libjpeg -openssl -no-nis -no-cups -dbus -qt-freetype -depths 8 -plugin-gfx-linuxfb  -plugin-gfx-transformed -plugin-gfx-vnc -plugin-mouse-tslib  -no-glib   -nomake tools -nomake examples -nomake demos -no-openssl -I${PREFIX}/include -L${PREFIX}/lib 


#./configure -release -prefix /usr/local/Qt -datadir /usr/local/share/Qt -embedded arm -no-largefile -no-accessibility -no-stl  -xplatform qws/linux-arm-g++   -little-endian  -depths 8  -qconfig minimal  -no-qt3support -qvfb  -nomake example -nomake tools -qt-mouse-tslib -I/usr/local/arm/oe/arm-linux/include -L/usr/local/arm/oe/arm-linux/lib

#make
#make install
