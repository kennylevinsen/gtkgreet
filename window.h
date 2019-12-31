#ifndef _WINDOW_H
#define _WINDOW_H

#include <gtk/gtk.h>

struct Window {
    GdkMonitor *monitor;

    GtkWidget *window;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *info_label;
    GtkWidget *clock_label;
    GtkWidget *target_combo_box;
};

void create_window(GdkMonitor *monitor);
void window_set_focus(struct Window* win);

#endif