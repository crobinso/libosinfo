# osinfo-detect
executable(
    'osinfo-detect',
    sources: 'osinfo-detect.c',
    dependencies: [
        glib_dep,
        gio_dep,
        gobject_dep,
        libxml_dep,
        libosinfo_dep,
    ],
    install: true,
)

# osinfo-install-script
executable(
    'osinfo-install-script',
    sources: 'osinfo-install-script.c',
    dependencies: [
        glib_dep,
        gio_dep,
        gobject_dep,
        libosinfo_dep,
    ],
    install: true,
)

# osinfo-query
executable(
    'osinfo-query',
    sources: 'osinfo-query.c',
    dependencies: [
        glib_dep,
        gio_dep,
        gobject_dep,
        libxml_dep,
        libosinfo_dep,
    ],
    install: true,
)

# man pages
pod2man = find_program('pod2man')
if pod2man.found()
    files = [
        'osinfo-detect',
        'osinfo-install-script',
        'osinfo-query',
    ]

    foreach file: files
        custom_target(
            file + '.1',
            output: file + '.1',
            input: file + '.c',
            install: true,
            install_dir: join_paths(libosinfo_datadir, 'man', 'man1'),
            build_by_default: true,
            command: [
                pod2man,
                '-c', 'Libosinfo',
                '-r', meson.project_name() + '-' + meson.project_version(),
                '--name', file,
                '@INPUT@', '@OUTPUT@',
            ]
        )
    endforeach
endif
