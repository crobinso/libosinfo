test(
    'syntax-check',
    find_program('@0@/build-aux/syntax-check'.format(meson.source_root())),
    suite: 'style',
    env: [
        'MESON_SOURCE_ROOT=@0@'.format(meson.source_root()),
    ],
)

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
    'test-entity.c',
    'test-filter.c',
    'test-firmware.c',
    'test-image.c',
    'test-install-script.c',
    'test-list.c',
    'test-loader.c',
    'test-media.c',
    'test-os.c',
    'test-platform.c',
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
