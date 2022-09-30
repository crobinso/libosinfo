# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool manifest ci/manifest.yml
#
# https://gitlab.com/libvirt/libvirt-ci

FROM docker.io/library/debian:11-slim

RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y eatmydata && \
    eatmydata apt-get dist-upgrade -y && \
    eatmydata apt-get install --no-install-recommends -y \
                      ca-certificates \
                      ccache \
                      check \
                      gcc \
                      gettext \
                      git \
                      gtk-doc-tools \
                      hwdata \
                      libgirepository1.0-dev \
                      libglib2.0-dev \
                      libsoup2.4-dev \
                      libxml2-dev \
                      libxml2-utils \
                      libxslt1-dev \
                      locales \
                      make \
                      meson \
                      ninja-build \
                      osinfo-db-tools \
                      pkgconf \
                      python3 \
                      python3-lxml \
                      python3-pytest \
                      python3-requests \
                      valac \
                      wget \
                      xz-utils && \
    eatmydata apt-get autoremove -y && \
    eatmydata apt-get autoclean -y && \
    sed -Ei 's,^# (en_US\.UTF-8 .*)$,\1,' /etc/locale.gen && \
    dpkg-reconfigure locales && \
    dpkg-query --showformat '${Package}_${Version}_${Architecture}\n' --show > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/gcc

ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"
