FROM docker.io/library/debian:sid

RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install --no-install-recommends -y \
            ca-certificates \
            ccache \
            check \
            gcc \
            gettext \
            git \
            gtk-doc-tools \
            hwdata \
            intltool \
            itstool \
            libarchive-dev \
            libgirepository1.0-dev \
            libglib2.0-dev \
            libjson-glib-dev \
            libsoup2.4-dev \
            libxml2-dev \
            libxml2-utils \
            libxslt1-dev \
            locales \
            make \
            meson \
            ninja-build \
            pkgconf \
            python3 \
            python3-lxml \
            python3-pytest \
            python3-requests \
            valac \
            wget \
            xz-utils && \
    apt-get autoremove -y && \
    apt-get autoclean -y && \
    sed -Ei 's,^# (en_US\.UTF-8 .*)$,\1,' /etc/locale.gen && \
    dpkg-reconfigure locales && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
