gnome = import('gnome')

mkdb_args = [
    '--xml-mode',
    '--output-format=xml',
]

scan_args = [
    '--rebuild-sections',
    '--rebuild-types',
]

gnome.gtkdoc('Libosinfo',
    main_xml: 'Libosinfo-docs.xml',
    include_directories: libosinfo_include,
    src_dir: [
        join_paths(meson.source_root(), 'osinfo'),
        join_paths(meson.build_root(), 'osinfo'),
    ],
    namespace: 'Libosinfo',
    mkdb_args: mkdb_args,
    scan_args: scan_args,
    dependencies: libosinfo_dep,
    install: true,
)
