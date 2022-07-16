<div align="center">
  <h1><strong>Sea Greeter</strong></h1>
  <p>
    <strong>Another LightDM greeter made with WebKitGTK2</strong>
  </p>
  <p>
    <a href="#">
      <img alt="License Information" src="https://img.shields.io/github/license/JezerM/sea-greeter.svg">
    </a>
  </p>
</div>

Sea (like C) greeter for LightDM allows to create themes with web technologies,
the same as [web-greeter][web-greeter], [nody-greeter][nody-greeter] and the
outdated [lightdm-webkit2-greeter][webkit2-greeter]. This project aims to be
similar to the last one, as sea-greeter is made with webkit2 instead of chromium.

## Known issues

- [x] Window does not start in fullscreen when no window manager available
- [ ] No multi-monitor support yet
- [x] No command line arguments support
- [ ] No brightness and battery features support
- [ ] No `greeter_config.layouts` support
- [x] Debug mode is always active (for testing)
- [ ] Memory management might not be correct (possible memory leaks)
- [ ] No themes preinstalled, you may need to install [web-greeter][web-greeter] or [nody-greeter][nody-greeter]
- [ ] No config preinstalled, you may need to install [web-greeter][web-greeter] or [nody-greeter][nody-greeter]
- [x] The webView freezes when a .face image fails to be loaded.

Besides that, you can login to your session :D

## Dependencies

- libgtk3
- webkit2gtk
- libwebkit2gtk-web-extension
- libyaml-0.1
- libglib-2.0
- liblightdm-gobject-1-dev

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

[web-greeter]: https://github.com/JezerM/web-greeter "Web Greeter"
[nody-greeter]: https://github.com/JezerM/nody-greeter "Nody Greeter"
[webkit2-greeter]: https://github.com/Antergos/web-greeter/tree/stable "LightDM WebKit2 Greeter"
