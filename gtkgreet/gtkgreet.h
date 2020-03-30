#ifndef _GTKGREET_H
#define _GTKGREET_H

#include <gtk/gtk.h>

#define INITIAL_QUESTION "Username:"

enum QuestionType {
	QuestionTypeInitial = 0,
	QuestionTypeVisible = 1,
	QuestionTypeSecret = 2,
	QuestionTypeInfo = 3,
	QuestionTypeError = 4,
};

// Defined in window.h
struct Window;

struct GtkGreet {
    GtkApplication *app;
    GArray *windows;

    struct Window *focused_window;

#ifdef LAYER_SHELL
    gboolean use_layer_shell;
#endif
    char* command;

    char* selected_command;
    enum QuestionType question_type;
    char *question;
    char *error;
};

struct GtkGreet *gtkgreet;

struct Window* gtkgreet_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *window);
struct Window* gtkgreet_window_by_monitor(struct GtkGreet *gtkgreet, GdkMonitor *monitor);
void gtkgreet_remove_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *widget);
void gtkgreet_setup_question(struct GtkGreet *gtkgreet, enum QuestionType type, char* question, char* error);

#endif