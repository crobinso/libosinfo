project(
    'libosinfo', 'c',
    version: '1.7.0',
    license: 'LGPLv2+',
    meson_version: '>= 0.49.0',
    default_options: [
        'buildtype=debugoptimized',
        'c_std=gnu99',
        'warning_level=1',
    ],
)

libosinfo_prefix = get_option('prefix')

# those directories have to be know by the project
libosinfo_datadir = join_paths(libosinfo_prefix, get_option('datadir'))
libosinfo_localedir = join_paths(libosinfo_prefix, get_option('localedir'))
libosinfo_pkgdatadir = join_paths(libosinfo_datadir, meson.project_name())
libosinfo_sysconfdir = join_paths(libosinfo_prefix, get_option('sysconfdir'))

# those directories will have files installed in
libsoinfo_bindir = join_paths(libosinfo_prefix, get_option('bindir'))
libsoinfo_docdir = join_paths(libosinfo_datadir, 'doc', meson.project_name())
libosinfo_includedir = join_paths(libosinfo_prefix, get_option('includedir'))
libosinfo_libdir = join_paths(libosinfo_prefix, get_option('libdir'))
libosinfo_licensedir = join_paths(libosinfo_datadir, 'license', meson.project_name())
libosinfo_mandir = join_paths(libosinfo_prefix, get_option('mandir'))
libosinfo_pkgconfdir = join_paths(libosinfo_libdir, 'pkgconfig')

# gobject introspection
gir = find_program('g-ir-scanner', required: get_option('enable-introspection'))
enable_introspection = gir.found() and not meson.is_cross_build()

# vala
vapi_opt = get_option('enable-vala')
enable_vapi = add_languages('vala', required: vapi_opt)
if enable_vapi and not enable_introspection
    enable_vapi = false
    if vapi_opt.enabled()
        error('vala support was requested, but introspection support is mandatory.')
    endif
endif

libosinfo_girdir = join_paths(libosinfo_datadir, 'gir-1.0')
libosinfo_typelibdir = join_paths(libosinfo_libdir, 'girepository-1.0')
libosinfo_pkgconfdir = join_paths(libosinfo_libdir, 'pkgconfig')

# spec files
libosinfo_spec_data = configuration_data()
libosinfo_spec_data.set('VERSION', meson.project_version())

specs = ['libosinfo.spec', 'mingw-libosinfo.spec']
foreach spec: specs
    configure_file(
        input: spec + '.in',
        output: spec,
        configuration: libosinfo_spec_data,
    )
endforeach

# ninja dist helper
meson.add_dist_script('build-aux/dist.sh', meson.source_root(), meson.build_root())

# dependencies
#  glib stuff
glib_version = '2.44'
glib_version_required = 'GLIB_VERSION_2_44'
glib_version_info = '>= @0@'.format(glib_version)
glib_dep = dependency('glib-2.0', version: glib_version_info)
gio_dep = dependency('gio-2.0', version: glib_version_info)
gobject_dep = dependency('gobject-2.0', version: glib_version_info)

#  everything else
libsoup_dep = dependency('libsoup-2.4')
libxml_dep = dependency('libxml-2.0', version: '>= 2.6.0')
libxslt_dep = dependency('libxslt', version: '>= 1.0.0')

#  common dependencies
libosinfo_dependencies = [
    gobject_dep,
    gio_dep,
    glib_dep,
    libsoup_dep,
    libxml_dep,
    libxslt_dep,
]

libosinfo_include = [include_directories('.')]

# arguments
libosinfo_cflags = []

#  glib stuff
libosinfo_cflags += [
    '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_44',
    '-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_44'
]

#  directories used
libosinfo_cflags += [
    '-DPKG_DATA_DIR="@0@"'.format(libosinfo_pkgdatadir),
    '-DDATA_DIR="@0@"'.format(libosinfo_datadir),
    '-DSYS_CONF_DIR="@0@"'.format(libosinfo_sysconfdir),
    '-DLOCALEDIR="@0@"'.format(libosinfo_localedir),
]

#  gettext package name
libosinfo_cflags += ['-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name())]

#  cflags to check whether the compiler supports them or not
libosinfo_check_cflags = [
    '-W',
    '-Waddress',
    '-Waggressive-loop-optimizations',
    '-Wall',
    '-Warray-bounds',
    '-Wattributes',
    '-Wbuiltin-macro-redefined',
    '-Wcast-align',
    '-Wchar-subscripts',
    '-Wclobbered',
    '-Wcomment',
    '-Wcomments',
    '-Wcoverage-mismatch',
    '-Wcpp',
    '-Wdate-time',
    '-Wdeprecated-declarations',
    '-Wdisabled-optimization',
    '-Wdiv-by-zero',
    '-Wempty-body',
    '-Wendif-labels',
    '-Wenum-compare',
    '-Wextra',
    '-Wformat-contains-nul',
    '-Wformat-extra-args',
    '-Wformat-security',
    '-Wformat-y2k',
    '-Wformat-zero-length',
    '-Wfree-nonheap-object',
    '-Wignored-qualifiers',
    '-Wimplicit',
    '-Wimplicit-function-declaration',
    '-Wimplicit-int',
    '-Winit-self',
    '-Winline',
    '-Wint-to-pointer-cast',
    '-Winvalid-memory-model',
    '-Winvalid-pch',
    '-Wjump-misses-init',
    '-Wlogical-op',
    '-Wmain',
    '-Wmaybe-uninitialized',
    '-Wmissing-braces',
    '-Wmissing-declarations',
    '-Wmissing-field-initializers',
    '-Wmissing-include-dirs',
    '-Wmissing-parameter-type',
    '-Wmissing-prototypes',
    '-Wmultichar',
    '-Wnarrowing',
    '-Wnested-externs',
    '-Wnonnull',
    '-Wold-style-declaration',
    '-Wold-style-definition',
    '-Wopenmp-simd',
    '-Woverflow',
    '-Woverlength-strings',
    '-Woverride-init',
    '-Wpacked',
    '-Wpacked-bitfield-compat',
    '-Wparentheses',
    '-Wpointer-arith',
    '-Wpointer-sign',
    '-Wpointer-to-int-cast',
    '-Wpragmas',
    '-Wreturn-local-addr',
    '-Wreturn-type',
    '-Wsequence-point',
    '-Wshadow',
    '-Wsizeof-pointer-memaccess',
    '-Wstack-protector',
    '-Wstrict-aliasing',
    '-Wstrict-overflow',
    '-Wstrict-prototypes',
    '-Wsuggest-attribute=const',
    '-Wsuggest-attribute=format',
    '-Wsuggest-attribute=noreturn',
    '-Wsuggest-attribute=pure',
    '-Wswitch',
    '-Wswitch-default',
    '-Wsync-nand',
    '-Wtrampolines',
    '-Wtrigraphs',
    '-Wtype-limits',
    '-Wuninitialized',
    '-Wunknown-pragmas',
    '-Wunsafe-loop-optimizations',
    '-Wunused',
    '-Wunused-but-set-parameter',
    '-Wunused-but-set-variable',
    '-Wunused-function',
    '-Wunused-label',
    '-Wunused-local-typedefs',
    '-Wunused-parameter',
    '-Wunused-result',
    '-Wunused-value',
    '-Wunused-variable',
    '-Wvarargs',
    '-Wvariadic-macros',
    '-Wvector-operation-performance',
    '-Wvla',
    '-Wvolatile-register-var',
    '-Wwrite-strings',
    '-Wnormalized=nfc',
    '-Wno-sign-compare',
    '-Wno-sign-conversion',
    '-Wno-conversion',
    '-Wno-unused-parameter',
    '-Wjump-misses-init',
    '-Wframe-larger-than=4096',
    '-Wno-overlength-strings',
    '--param=ssp-buffer-size=4',
    '-fexceptions',
    '-fasynchronous-unwind-tables',
    '-fdiagnostics-show-option',
    '-funit-at-a-time',
    '-fipa-pure-const',
    '-Wno-suggest-attribute=pure',
    '-Wno-suggest-attribute=const',
]

if host_machine.system() == 'linux'
    libosinfo_check_cflags += ['-fstack-protector-all']
endif

# Builds os macOS and BSDs will fail without those.
if host_machine.system() != 'linux'
    libosinfo_check_cflags += [
        '-Wno-typedef-redefinition',
        '-Wno-missing-field-initializers',
    ]
endif

if run_command('[', '-d', '.git', ']').returncode() == 0
    libosinfo_check_cflags += ['-Werror']
endif

compiler = meson.get_compiler('c')
foreach cflag: libosinfo_check_cflags
  if compiler.has_argument(cflag)
    libosinfo_cflags += [cflag]
  endif
endforeach

pci_ids_path = get_option('with-pci-ids-path')
if pci_ids_path == ''
    pci_id_files = [
        '/usr/share/pci.ids',
        '/usr/share/misc/pci.ids',
        '/usr/share/hwdata/pci.ids',
        '/usr/local/share/pciids/pci.ids',
    ]

    foreach file: pci_id_files
        if run_command('[', '-f', file, ']').returncode() == 0
            pci_ids_path = file
            break
        endif
    endforeach

    if pci_ids_path == ''
        download = find_program('build-aux/download_pci_ids')
        if download.found()
            r = run_command(download)
            if r.returncode() == 0
                pci_ids_path = join_paths(meson.build_root(), 'pci.ids')
            endif
        endif
    endif
endif
libosinfo_cflags += ['-DPCI_IDS="@0@"'.format(pci_ids_path)]
message('Location of pci.ids: "@0@"'.format(pci_ids_path))

usb_ids_path = get_option('with-usb-ids-path')
if usb_ids_path == ''
    usb_id_files = [
        '/usr/share/usb.ids',
        '/usr/share/misc/usb.ids',
        '/usr/share/hwdata/usb.ids',
        '/usr/local/share/usbids/usb.ids',
    ]

    foreach file: usb_id_files
        if run_command('[', '-f', file, ']').returncode() == 0
            usb_ids_path = file
            break
        endif
    endforeach

    if usb_ids_path == ''
        download = find_program('build-aux/download_usb_ids')
        if download.found()
            r = run_command(download)
            if r.returncode() == 0
                usb_ids_path = join_paths(meson.build_root(), 'usb.ids')
            endif
        endif
    endif
endif
libosinfo_cflags += ['-DUSB_IDS="@0@"'.format(usb_ids_path)]
message('Location of usb.ids: "@0@"'.format(usb_ids_path))

add_project_arguments(libosinfo_cflags, language: 'c')

libosinfo_version = meson.project_version()
libosinfo_major_version = libosinfo_version.split('.')[0].to_int()
libosinfo_minor_version = libosinfo_version.split('.')[1].to_int()
libosinfo_micro_version = libosinfo_version.split('.')[2].to_int()

libosinfo_interface_age = 0
libosinfo_binary_age = 1000 * libosinfo_major_version + libosinfo_minor_version

# Mantain versin scheme with libtool
libosinfo_soversion = 0
libosinfo_version = '@0@.@1@.@2@'.format(
    libosinfo_soversion,
    (libosinfo_binary_age - libosinfo_interface_age),
    libosinfo_interface_age)
osx_current = libosinfo_binary_age - libosinfo_interface_age + 1
libosinfo_darwin_versions = [
    osx_current,
    '@0@.@1@.0'.format(osx_current, libosinfo_interface_age)
]

libosinfo_ldflags = []
if meson.get_compiler('c').has_link_argument('-Wl,--no-undefined')
    libosinfo_ldflags += ['-Wl,--no-undefined']
endif

if host_machine.system() == 'linux'
    libosinfo_ldflags += [
        '-Wl,--version-script.@0@/@1@'.format(
            meson.current_source_dir(),
            'libosinfo.syms'
        )
    ]
endif

subdir('osinfo')
subdir('tools')
subdir('po')
if get_option('enable-gtk-doc') and not meson.is_cross_build()
    if find_program('gtkdoc-scan', required: true).found()
        subdir('docs/reference')
    else
        message('Not building documentation as gtk-doc was not found')
    endif
endif
if get_option('enable-tests')
    subdir('tests')
endif
