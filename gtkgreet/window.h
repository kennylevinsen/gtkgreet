#ifndef _WINDOW_H
#define _WINDOW_H

#include <gtk/gtk.h>

// Defined in gtkgreet.h
enum QuestionType;

struct Window {
    GdkMonitor *monitor;

    GtkWidget *window;
    GtkWidget *revealer;
    GtkWidget *window_box;
    GtkWidget *body;
    GtkWidget *input_box;
    GtkWidget *input_field;
    GtkWidget *command_selector;
    GtkWidget *clock_label;

#ifdef LAYER_SHELL
    gulong enter_notify_handler;
#endif
};

struct Window *create_window(GdkMonitor *monitor);
void window_configure(struct Window *win);
void window_setup_question(struct Window *ctx, enum QuestionType type, char* question, char* error);
void window_update_clock(struct Window *ctx);
void window_swap_focus(struct Window *win, struct Window *old);

#endif