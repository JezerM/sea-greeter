project('sea-greeter', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

as_version = meson.project_version()

conf = configuration_data()

conf.set('VERSION', as_version)
conf.set('WEB_EXTENSIONS_DIR', '"/usr/lib/sea-greeter/"')

subdirs = ['src', 'data']

foreach s : subdirs
  subdir(s)
endforeach
