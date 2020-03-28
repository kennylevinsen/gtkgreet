# gtkgreet

GTK based greeter for greetd, to be run under cage or similar.

See the [wiki](https://man.sr.ht/~kennylevinsen/greetd) for FAQ, guides for common configurations, and troubleshooting information.


![screenshot](https://git.sr.ht/~kennylevinsen/gtkgreet/blob/master/assets/screenshot.png)

## How to use

See the wiki.

## How to build with gtk-layer-shell (recommended)

```
meson -Dlayershell=true build
ninja -C build
```

## How to build without gtk-layer-shell

```
meson build
ninja -C build
```