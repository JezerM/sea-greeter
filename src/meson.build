configure_file(output: 'config.h', configuration: conf)

web_extension_sources = ['extensions.c']

gtk3 = dependency('gtk+-3.0', version: '>=3.24')
webkit2 = dependency('webkit2gtk-4.0', version: '>=2.34')
webkit2_webext = dependency('webkit2gtk-web-extension-4.0', version: '>=2.34')

yaml = dependency('yaml-0.1', version: '>=0.2')
glib = dependency('glib-2.0', version: '>=2.68')

web_extension = library(
  'lightdm-sea-greeter-webext',
  web_extension_sources,
  dependencies: [webkit2_webext],
  install: true,
  install_dir: '/usr/lib/sea-greeter/'
)

greeter_sources = ['greeter.c', 'settings.c', 'theme.c']

greeter = executable(
  'sea-greeter',
  greeter_sources,
  dependencies: [webkit2, gtk3, yaml, glib],
  install: true,
)
