project(
  'sea-greeter', 'c',
  version: '0.1',
  license: 'GPL-3.0-or-later',
  default_options: ['warning_level=3']
)

as_version = meson.project_version()

conf = configuration_data()

conf.set('VERSION', '"@0@"'.format(as_version))
conf.set('API_VERSION', '"@0@"'.format('1.0.0'))
conf.set('WEB_EXTENSIONS_DIR', get_option('with-webext-dir'))

subdirs = ['src', 'data']

foreach s : subdirs
  subdir(s)
endforeach
