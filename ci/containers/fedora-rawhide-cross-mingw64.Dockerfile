# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool manifest ci/manifest.yml
#
# https://gitlab.com/libvirt/libvirt-ci

FROM registry.fedoraproject.org/fedora:rawhide

RUN dnf update -y --nogpgcheck fedora-gpg-keys && \
    dnf install -y nosync && \
    echo -e '#!/bin/sh\n\
if test -d /usr/lib64\n\
then\n\
    export LD_PRELOAD=/usr/lib64/nosync/nosync.so\n\
else\n\
    export LD_PRELOAD=/usr/lib/nosync/nosync.so\n\
fi\n\
exec "$@"' > /usr/bin/nosync && \
    chmod +x /usr/bin/nosync && \
    nosync dnf distro-sync -y && \
    nosync dnf install -y \
               ca-certificates \
               ccache \
               check-devel \
               cppi \
               git \
               glibc-langpack-en \
               gtk-doc \
               hwdata \
               libxml2 \
               make \
               meson \
               ninja-build \
               osinfo-db-tools \
               python3 \
               python3-lxml \
               python3-pytest \
               python3-requests \
               rpm-build \
               vala \
               wget \
               xz && \
    nosync dnf autoremove -y && \
    nosync dnf clean all -y

ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"

RUN nosync dnf install -y \
               mingw64-gcc \
               mingw64-gettext \
               mingw64-glib2 \
               mingw64-libsoup \
               mingw64-libxml2 \
               mingw64-libxslt \
               mingw64-pkg-config && \
    nosync dnf clean all -y && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/x86_64-w64-mingw32-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/x86_64-w64-mingw32-gcc

ENV ABI "x86_64-w64-mingw32"
ENV MESON_OPTS "--cross-file=/usr/share/mingw/toolchain-mingw64.meson"
