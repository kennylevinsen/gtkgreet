#ifndef _GTKGREET_H
#define _GTKGREET_H
#include <gtk/gtk.h>

#include "window.h"

struct GtkGreet {
    GtkApplication *app;
    GArray *windows;

    struct Window *focused_window;

#ifdef LAYER_SHELL
    gboolean use_layer_shell;
#endif

};

struct GtkGreet *gtkgreet;

struct Window* gtkgreet_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *window);
struct Window* gtkgreet_window_by_monitor(struct GtkGreet *gtkgreet, GdkMonitor *monitor);
void gtkgreet_remove_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *widget);

#endif