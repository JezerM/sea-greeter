# Sea Greeter

Sea (like C) greeter for LightDM allows to create themes with web technologies, the same as web-greeter, nody-greeter and the outdated lightdm-webkit2-greeter. This project aims to be similar to the last one, as sea-greeter is made with webkit2 instead of chromium.

## Dependencies

- libgtk3
- webkit2gtk
- libwebkit2gtk-web-extension
- libyaml-0.1
- libglib-2.0

### Build dependencies

- Meson
- Ninja
- gcc

## Build and install

```sh
git clone https://github.com/JezerM/sea-greeter
cd sea-greeter
meson build
ninja -C build
sudo ninja -C build install
```
