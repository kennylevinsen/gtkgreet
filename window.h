#ifndef _WINDOW_H
#define _WINDOW_H

#include <gtk/gtk.h>

enum QuestionType {
	QuestionTypeInitial = 0,
	QuestionTypeVisible = 1,
	QuestionTypeSecret = 2,
	QuestionTypeInfo = 3,
	QuestionTypeError = 4,
};

struct Window {
    GdkMonitor *monitor;

    GtkWidget *window;
    GtkWidget *input_box;
    GtkWidget *input;
    GtkWidget *info_label;
    GtkWidget *clock_label;

    enum QuestionType question_type;
};

void create_window(GdkMonitor *monitor);
void window_set_focus(struct Window* win);

#endif