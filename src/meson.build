configure_file(output: 'config.h', configuration: conf)

web_extension_sources = [
  'extensions.c',
  'lightdm-extension.c',
  'settings.c',
  'utils/ipc-renderer.c',
  'utils/utils.c',

  'extension/lightdm.c',
  'extension/lightdm-signal.c',
  'extension/greeter_config.c',
  'extension/theme_utils.c',

  'bridge/lightdm-objects.c',
  'bridge/utils.c',
  #'bridge/lightdm-signal.c',
]

gtk3 = dependency('gtk+-3.0', version: '>=3.24')
webkit2 = dependency('webkit2gtk-4.0', version: '>=2.34')
webkit2_webext = dependency('webkit2gtk-web-extension-4.0', version: '>=2.34')

yaml = dependency('yaml-0.1', version: '>=0.2')
glib = dependency('glib-2.0', version: '>=2.68')
lightdm = dependency('liblightdm-gobject-1')

web_extension = library(
  'lightdm-sea-greeter-webext',
  web_extension_sources,
  dependencies: [webkit2_webext, lightdm, yaml],
  install: true,
  install_dir: '/usr/lib/sea-greeter/'
)

greeter_sources = [
  'greeter.c',
  'settings.c', 
  'theme.c',

  'browser.c',
  'browser-web-view.c',
  'browser-commands.c',

  'utils/ipc-main.c',
  'utils/utils.c',

  'bridge/lightdm.c',
  'bridge/greeter_config.c',
  'bridge/theme_utils.c',
  'bridge/utils.c',
  #'bridge/lightdm-signal.c',
  'bridge/lightdm-objects.c',
]

gnome = import('gnome')

greeter_sources += gnome.compile_resources('sea_greeter-resources',
  'sea-greeter.gresource.xml',
  c_name: 'sea_greeter'
)

greeter = executable(
  'sea-greeter',
  greeter_sources,
  dependencies: [webkit2, gtk3, yaml, glib, lightdm],
  install: true,
)
