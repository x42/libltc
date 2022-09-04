install_headers('ltc.h')

ltc_src = files([
  'decoder.c',
  'encoder.c',
  'ltc.c',
  'timecode.c'
])

ltc_lib = library('ltc',
                  ltc_src,
                  include_directories : inc,
                  dependencies : m_dep,
                  version : '11.1.0',
                  install : true)
