FROM registry.fedoraproject.org/fedora:33

RUN dnf update -y && \
    dnf install -y \
        ca-certificates \
        ccache \
        check-devel \
        gcc \
        gettext \
        git \
        glib2-devel \
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
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV NINJA "/usr/bin/ninja"
ENV PYTHON "/usr/bin/python3"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
