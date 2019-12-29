# gtkgreet

GTK based greeter for greetd, to be run under cage or similar.

![screenshot](https://git.sr.ht/~kennylevinsen/gtkgreet/blob/master/assets/screenshot.png)

## Example use

1. Install the requirements:

    1. greetd
    2. cage
    3. gtkgreet

2. Set the greetd greeter to `cage gtkgreet` in `/etc/greetd/config.toml` like so:

```
vt = 2
greeter = "cage gtkgreet"
greeter_user = "greeter"
```

3. (Re-)start greetd.
