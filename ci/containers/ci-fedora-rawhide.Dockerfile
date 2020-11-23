FROM registry.fedoraproject.org/fedora:rawhide

RUN dnf update -y --nogpgcheck fedora-gpg-keys && \
    dnf update -y && \
    dnf install -y \
        bash \
        bash-completion \
        ca-certificates \
        ccache \
        check-devel \
        gcc \
        gettext \
        git \
        glib2-devel \
        glibc-devel \
        glibc-langpack-en \
        gobject-introspection-devel \
        gtk-doc \
        hwdata \
        intltool \
        itstool \
        json-glib-devel \
        libarchive-devel \
        libsoup-devel \
        libxml2 \
        libxml2-devel \
        libxslt-devel \
        make \
        meson \
        ninja-build \
        patch \
        perl \
        perl-App-cpanminus \
        pkgconfig \
        python3 \
        python3-lxml \
        python3-pip \
        python3-pytest \
        python3-requests \
        python3-setuptools \
        python3-wheel \
        rpm-build \
        vala \
        wget \
        xz && \
    dnf autoremove -y && \
    dnf clean all -y && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

ENV LANG "en_US.UTF-8"

ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"

ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
