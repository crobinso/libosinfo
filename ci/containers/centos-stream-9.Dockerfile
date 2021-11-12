# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool manifest ci/manifest.yml
#
# https://gitlab.com/libvirt/libvirt-ci

FROM quay.io/centos/centos:stream9

RUN dnf update -y && \
    dnf install 'dnf-command(config-manager)' -y && \
    dnf config-manager --set-enabled -y crb && \
    dnf install -y \
        ca-certificates \
        check-devel \
        gcc \
        gettext \
        git \
        glib2-devel \
        glibc-langpack-en \
        gobject-introspection-devel \
        gtk-doc \
        hwdata \
        libsoup-devel \
        libxml2 \
        libxml2-devel \
        libxslt-devel \
        make \
        meson \
        ninja-build \
        osinfo-db-tools \
        pkgconfig \
        python3 \
        python3-lxml \
        python3-pytest \
        python3-requests \
        rpm-build \
        vala \
        wget \
        xz && \
    dnf autoremove -y && \
    dnf clean all -y && \
    rpm -qa | sort > /packages.txt

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"
