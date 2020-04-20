#!/bin/sh

set -e
set -v

INSTALL_ROOT="$PWD/install"
RPMBUILD_ROOT="$PWD/rpmbuild"

# Make things clean.
rm -rf build "$INSTALL_ROOT" "$RPMBUILD_ROOT"

meson build/native \
    --werror \
    -Denable-gtk-doc=true \
    -Denable-tests=true \
    -Denable-introspection=enabled \
    -Denable-vala=enabled

ninja -C build/native
DESTDIR="$INSTALL_ROOT/native" ninja -C build/native install
ninja -C build/native test
ninja -C build/native dist

if test -x /usr/bin/rpmbuild; then
  rpmbuild --nodeps \
     --define "_topdir $RPMBUILD_ROOT" \
     --define "_sourcedir `pwd`/build/native/meson-dist/" \
     -ba --clean build/native/libosinfo.spec
fi

# Test mingw32 cross-compile
if test -x /usr/bin/i686-w64-mingw32-gcc && \
   test -r /usr/share/mingw/toolchain-mingw32.meson ; then
  meson build/win32 \
        --werror \
        -Denable-gtk-doc=false \
        -Denable-tests=false \
        -Denable-introspection=disabled \
        -Denable-vala=disabled \
        --cross-file="/usr/share/mingw/toolchain-mingw32.meson"

  ninja -C build/win32
  DESTDIR="$INSTALL_ROOT/win32" ninja -C build/win32 install
fi

# Test mingw64 cross-compile
if test -x /usr/bin/x86_64-w64-mingw32-gcc && \
   test -r /usr/share/mingw/toolchain-mingw64.meson ; then
  meson build/win64 \
        --werror \
        -Denable-gtk-doc=false \
        -Denable-tests=false \
        -Denable-introspection=disabled \
        -Denable-vala=disabled \
        --cross-file="/usr/share/mingw/toolchain-mingw64.meson"

  ninja -C build/win64
  DESTDIR="$INSTALL_ROOT/win64" ninja -C build/win64 install
fi

if test -x /usr/bin/i686-w64-mingw32-gcc && \
   test -r /usr/share/mingw/toolchain-mingw32.meson && \
   test -x /usr/bin/x86_64-w64-mingw32-gcc && \
   test -r /usr/share/mingw/toolchain-mingw64.meson && \
   test -x /usr/bin/rpmbuild; then
  rpmbuild --nodeps \
     --define "_topdir $RPMBUILD_ROOT" \
     --define "_sourcedir `pwd`/build/native/meson-dist/" \
     -ba --clean build/native/mingw-libosinfo.spec
fi
