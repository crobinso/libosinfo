libosinfo_include += [include_directories('.')]

# generate osinfo_version.h
version_info = meson.project_version().split('.')
version_major = '@0@'.format(version_info[0])
version_minor = '@0@'.format(version_info[1])
version_micro = '@0@'.format(version_info[2])

version_data = configuration_data()
version_data.set('LIBOSINFO_MAJOR_VERSION', version_major)
version_data.set('LIBOSINFO_MINOR_VERSION', version_minor)
version_data.set('LIBOSINFO_MICRO_VERSION', version_micro)
osinfo_version = configure_file(
    input: 'osinfo_version.h.in',
    output: 'osinfo_version.h',
    configuration: version_data,
    install_dir: join_paths(libosinfo_includedir, 'libosinfo-1.0', 'osinfo'),
)

libosinfo_headers = [
    osinfo_version,
    'osinfo.h',
    'osinfo_avatar_format.h',
    'osinfo_db.h',
    'osinfo_loader.h',
    'osinfo_datamap.h',
    'osinfo_datamaplist.h',
    'osinfo_device.h',
    'osinfo_devicelink.h',
    'osinfo_devicelist.h',
    'osinfo_devicelinklist.h',
    'osinfo_devicelinkfilter.h',
    'osinfo_device_driver.h',
    'osinfo_device_driverlist.h',
    'osinfo_entity.h',
    'osinfo_filter.h',
    'osinfo_firmware.h',
    'osinfo_firmwarelist.h',
    'osinfo_install_config.h',
    'osinfo_install_config_param.h',
    'osinfo_install_config_paramlist.h',
    'osinfo_install_script.h',
    'osinfo_install_scriptlist.h',
    'osinfo_product.h',
    'osinfo_productfilter.h',
    'osinfo_productlist.h',
    'osinfo_platform.h',
    'osinfo_platformlist.h',
    'osinfo_list.h',
    'osinfo_os.h',
    'osinfo_oslist.h',
    'osinfo_os_variant.h',
    'osinfo_os_variantlist.h',
    'osinfo_deployment.h',
    'osinfo_deploymentlist.h',
    'osinfo_media.h',
    'osinfo_medialist.h',
    'osinfo_resources.h',
    'osinfo_resourceslist.h',
    'osinfo_tree.h',
    'osinfo_treelist.h',
    'osinfo_image.h',
    'osinfo_imagelist.h',
]

# generate osinfo-enum-types.[ch]
gnome = import('gnome')
libosinfo_enum_types = gnome.mkenums_simple(
    'osinfo_enum_types',
    sources: libosinfo_headers,
    install_header: true,
    install_dir: join_paths(libosinfo_includedir, 'libosinfo-1.0', 'osinfo'))

install_headers(libosinfo_headers, subdir: join_paths('libosinfo-1.0', 'osinfo'))

libosinfo_sources = [
    'osinfo_avatar_format.c',
    'osinfo_datamap.c',
    'osinfo_datamaplist.c',
    'osinfo_device.c',
    'osinfo_devicelink.c',
    'osinfo_devicelist.c',
    'osinfo_devicelinklist.c',
    'osinfo_devicelinkfilter.c',
    'osinfo_device_driver.c',
    'osinfo_device_driverlist.c',
    'osinfo_entity.c',
    'osinfo_filter.c',
    'osinfo_firmware.c',
    'osinfo_firmwarelist.c',
    'osinfo_install_config.c',
    'osinfo_install_config_param.c',
    'osinfo_install_config_paramlist.c',
    'osinfo_install_script.c',
    'osinfo_install_scriptlist.c',
    'osinfo_product.c',
    'osinfo_productfilter.c',
    'osinfo_productlist.c',
    'osinfo_platform.c',
    'osinfo_platformlist.c',
    'osinfo_list.c',
    'osinfo_oslist.c',
    'osinfo_os.c',
    'osinfo_os_variant.c',
    'osinfo_os_variantlist.c',
    'osinfo_deployment.c',
    'osinfo_deploymentlist.c',
    'osinfo_media.c',
    'osinfo_medialist.c',
    'osinfo_resources.c',
    'osinfo_resourceslist.c',
    'osinfo_tree.c',
    'osinfo_treelist.c',
    'osinfo_image.c',
    'osinfo_imagelist.c',
    'osinfo_db.c',
    'osinfo_loader.c',
    'osinfo_util_private.c',
]

libosinfo_private_headers = [
    'osinfo_device_driver_private.h',
    'osinfo_install_script_private.h',
    'osinfo_product_private.h',
    'osinfo_media_private.h',
    'osinfo_resources_private.h',
    'osinfo_util_private.h',
    'ignore-value.h',
]

libosinfo = library(
    'osinfo-1.0',
    sources: [
        libosinfo_sources,
        libosinfo_enum_types,
    ],
    dependencies: libosinfo_dependencies,
    include_directories: libosinfo_include,
    version: libosinfo_version,
    darwin_versions: libosinfo_darwin_versions,
    soversion: libosinfo_soversion,
    install: true,
)

libosinfo_dep = declare_dependency(
    sources: [
        libosinfo_enum_types,
    ],
    link_with: libosinfo,
    dependencies: libosinfo_dependencies,
    include_directories: libosinfo_include,
)

pkgdata = configuration_data()
pkgdata.set('datarootdir', join_paths(libosinfo_prefix, 'data'))
pkgdata.set('sysconfdir', join_paths(libosinfo_prefix, 'etc'))
pkgdata.set('exec_prefix', libosinfo_prefix)
pkgdata.set('includedir', libosinfo_includedir)
pkgdata.set('libdir', libosinfo_libdir)
pkgdata.set('prefix', libosinfo_prefix)
pkgdata.set('VERSION', meson.project_version())
configure_file(
    input: 'libosinfo-1.0.pc.in',
    output: 'libosinfo-1.0.pc',
    configuration: pkgdata,
    install_dir: libosinfo_pkgconfdir,
)

libosinfo_gir_sources = [
    libosinfo_headers,
    libosinfo_sources,
    libosinfo_enum_types,
]

if host_machine.system() != 'windows'
    if enable_introspection
        libosinfo_gir = gnome.generate_gir(
            libosinfo,
            sources: libosinfo_gir_sources,
            nsversion: '1.0',
            namespace: 'Libosinfo',
            export_packages: 'libosinfo-1.0',
            includes: [
                'GObject-2.0',
                'Gio-2.0',
                'libxml2-2.0',
            ],
            dependencies: libosinfo_dependencies,
            header: 'osinfo/osinfo.h',
            identifier_prefix: 'Osinfo',
            symbol_prefix: 'osinfo',
            install: true,
            install_dir_gir: libosinfo_girdir,
            install_dir_typelib: libosinfo_typelibdir,
        )

        libosinfo_gir_dep = declare_dependency(
            sources: libosinfo_gir,
        )

        if enable_vapi
            libosinfo_vapi = gnome.generate_vapi(
                'libosinfo-1.0',
                install: true,
                packages: [
                    'gobject-2.0',
                    'gio-2.0',
                    'libxml-2.0',
                ],
                sources: libosinfo_gir[0],
            )
        endif
    endif
endif
