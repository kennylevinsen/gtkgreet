# gtkgreet

GTK based greeter for greetd, to be run under cage or similar.

See the [wiki](https://man.sr.ht/~kennylevinsen/greetd) for FAQ, guides for common configurations, and troubleshooting information.


![screenshot](https://git.sr.ht/~kennylevinsen/gtkgreet/blob/master/assets/screenshot.png)

## How to use

See the wiki.

## How to build

```
meson build
ninja -C build
```
Layer-shell support will be enabled automatically if [gtk-layer-shell](https://github.com/wmww/gtk-layer-shell) development files are installed.

## How to build without gtk-layer-shell

```
meson build -Dlayershell=disabled
ninja -C build
```

# How to discuss

Go to #kennylevinsen @ irc.libera.chat to discuss, or use [~kennylevinsen/greetd-devel@lists.sr.ht](https://lists.sr.ht/~kennylevinsen/greetd-devel).
