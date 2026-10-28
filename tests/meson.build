test_lib = executable('test-rclib',
  'test-rclib.c',
  dependencies: [rclib_dep, json_dep],
  include_directories: config_inc,
)
test('rclib', test_lib, env: ['G_DEBUG=fatal-criticals'])
