#ifndef _WINDOW_H
#define _WINDOW_H

#include <gtk/gtk.h>


struct Window {
    GdkMonitor *monitor;

    GtkWidget *window;
    GtkWidget *input_box;
    GtkWidget *input;
    GtkWidget *input_field;
    GtkWidget *info_label;
    GtkWidget *clock_label;
};

void create_window(GdkMonitor *monitor);
void window_set_focus(struct Window* win);

#endif