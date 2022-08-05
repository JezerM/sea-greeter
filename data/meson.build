tsc = find_program('tsc', '/usr/bin/tsc', '/usr/lib/node_modules/typescript/bin/tsc', required: true)

custom_target(
  'theme_gruvbox',
  build_always_stale: true,
  command: [tsc, '--build', '@INPUT@'],
  console: true,
  input: '../themes/themes/gruvbox/tsconfig.json',
  output: 'gruvbox'
)
custom_target(
  'theme_dracula',
  build_always_stale: true,
  command: [tsc, '--build', '@INPUT@'],
  console: true,
  input: '../themes/themes/dracula/tsconfig.json',
  output: 'dracula'
)

install_subdir(
  '../themes/themes/gruvbox',
  install_dir: '/usr/share/web-greeter/themes',
  strip_directory: false
)
install_subdir(
  '../themes/themes/dracula',
  install_dir: '/usr/share/web-greeter/themes',
  strip_directory: false
)
install_subdir(
  '../themes/themes/simple',
  install_dir: '/usr/share/web-greeter/themes',
  strip_directory: false
)
install_subdir(
  '../themes/themes/_vendor',
  install_dir: '/usr/share/web-greeter/',
  strip_directory: false
)

install_data('web-greeter.yml', install_dir: '/etc/lightdm')
install_data('sea-greeter.desktop', install_dir: '/usr/share/applications/')
install_data('sea-xgreeter.desktop', install_dir: '/usr/share/xgreeters/', rename: 'sea-greeter.desktop')
