tests = [
  'test-checker',
  # 'test-entry',
  # 'test-text-view',
  # 'test-text-view-basic'
]

foreach test_name: tests
  test_src = test_name + '.c'
  test_bin = executable(test_name, test_src,
    dependencies: libgspell_dep,
    include_directories: root_inc,
    c_args: [
      '-DG_DISABLE_DEPRECATED',
    ],
  )

  test(test_name, test_bin,
    env: [
      'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
      'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
    ],
  )
endforeach