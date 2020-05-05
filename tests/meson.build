ltcencode_exe = executable('ltcencode',
                           'ltcencode.c',
                           include_directories : inc,
                           link_with : ltc_lib)

ltcdecode_exe = executable('ltcdecode',
                           'ltcdecode.c',
                           include_directories : inc,
                           link_with : ltc_lib)

test_sh = find_program('test.sh')

test('All tests',
     test_sh,
     env: ['BLD=' + meson.current_build_dir(),
           'SRC=' + meson.current_source_dir(),
           'PRJ=' + meson.project_name(),
           'VER=' + meson.project_version()])
