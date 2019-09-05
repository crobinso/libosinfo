if host_machine.system() == 'linux'
    check_sources = [
        'check-symfile',
        'check-symsorting',
    ]

    foreach check: check_sources
        exe = find_program(check)
        test(
            check,
            exe,
            suite: 'symbols',
            env: [
                'MESON_SOURCE_ROOT=@0@'.format(meson.source_root()),
                'MESON_BUILD_ROOT=@0@'.format(meson.build_root()),
            ],
        )
    endforeach
endif

tests_sources = [
    'test-db.c',
    'test-device.c',
    'test-devicelinklist.c',
    'test-devicelist.c',
    'test-entity.c',
    'test-filter.c',
    'test-firmware.c',
    'test-firmwarelist.c',
    'test-image.c',
    'test-imagelist.c',
    'test-install-script.c',
    'test-list.c',
    'test-loader.c',
    'test-media.c',
    'test-os.c',
    'test-oslist.c',
    'test-platform.c',
    'test-platformlist.c',
    'test-product.c',
    'test-productfilter.c',
    'test-tree.c',
]

foreach src: tests_sources
    name = src.split('.')[0]
    exe = executable(name,
        sources: src,
        dependencies: libosinfo_dep,
        c_args: [
            '-DSRCDIR="@0@"'.format(meson.source_root()),
            '-DBUILDDIR="@0@"'.format(meson.build_root()),
        ],
    )

    test(
        name,
        exe,
        suite: 'unit',
    )
endforeach