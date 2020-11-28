gtkgreet(1)

# NAME

gtkgreet - A Gtk-based greeter for greetd

# SYNOPSIS

*gtkgreet* [options]

# OPTIONS

*-h, --help*
	Show help message and quit.

*-c, --command <command>*
	Specifies the command to run on successful login. gtkgreet will ask if none
	is specified.

*-l, --layer-shell*
	Use wlr-layer-shell to display a window on every output. Must be enabled at
	compile-time.

*-b, --background*
	Specifies a background image to be used by gtkgreet. JPG and PNG are known
	to work. The image will be anchored to the upper left corner, and is
	rendered 1:1 when DPI scaling is 1.

*-s, --style <file.css>*
	Specifies a custom CSS stylesheet with additional styles for the gtkgreet
	window. See https://developer.gnome.org/gtk3/stable/theming.html for the
	description of CSS subset supported in GTK.

# DESCRIPTION

gtkgreet is a Gtk-based, graphical greeter for *greetd*(1).

gtkgreet allows selection of which application to start at login. See the
ENVIRONMENTS section for more information on how to configure the provided
options. The user can also specify the application directly at login by
writing the command line they wish.

gtkgreet requires a Wayland compositor to run, such as *cage*(1) or *sway*(1).

# SETUP

To use gtkgreet, configure a Wayland compositor of your choice to be your
greeter. Make the compositor start gtkgreet, and importantly, shut itself down
once gtkgreet terminates.

Using *cage*, the command-line simply becomes "cage gtkgreet".

Using *sway*(1), one can use a sway config containing "exec 'gtkgreet; swaymsg
exit'", and a command-line similar to "sway --config /etc/greetd/sway-config".
The "; swaymg exit" component is important, as that makes sway terminate once
gtkgreet has finalized a login.

See *greetd*(5) for information on how to configure greetd. Read the
documentation of your chosen compositor for information on how to use it
correctly.

# ENVIRONMENTS

gtkgreet populates the user options for applications to start at login using
both a configuration file and the '-c' command-line argument.

The configuration file is read from /etc/greetd/environments, and is a
newline-seperated list of of options, each being a command line to run on
login if selected.

# AUTHORS

Maintained by Kenny Levinsen <contact@kl.wtf>. For more information about
gtkgreet development, see https://git.sr.ht/~kennylevinsen/gtkgreet.

# SEE ALSO
*greetd*(1)
