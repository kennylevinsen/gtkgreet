#ifndef _WINDOW_H
#define _WINDOW_H

#include <gtk/gtk.h>

// Defined in gtkgreet.h
enum QuestionType;

struct Window {
    GdkMonitor *monitor;

    GtkWidget *window;
    GtkWidget *body;
    GtkWidget *input_box;
    GtkWidget *input_field;
    GtkWidget *command_selector;
    GtkWidget *info_label;
    GtkWidget *clock_label;

#ifdef LAYER_SHELL
    gulong enter_notify_handler;
#endif
};

void create_window(GdkMonitor *monitor);
void window_set_focus(struct Window* win);
void window_setup_question(struct Window *ctx, enum QuestionType type, char* question, char* error);

#endif