project('libltc', 'c',
        version : '1.3.1',
        license : 'LGPL3')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

inc = include_directories('src')

subdir('src')

if get_option('tests')
  subdir('tests')
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : ltc_lib,
                 version : meson.project_version(),
                 name : 'libltc',
                 filebase : 'ltc',
                 description : 'linear/longitudinal timecode library')
