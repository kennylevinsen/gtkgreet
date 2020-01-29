#include "gtkgreet.h"
#include "auth_question.h"

struct Window* gtkgreet_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *window) {
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        if (ctx->window == window) {
            return ctx;
        }
    }
    return NULL;
}

struct Window* gtkgreet_window_by_monitor(struct GtkGreet *gtkgreet, GdkMonitor *monitor) {
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        if (ctx->monitor == monitor) {
            return ctx;
        }
    }
    return NULL;
}

void gtkgreet_remove_window_by_widget(struct GtkGreet *gtkgreet, GtkWidget *widget) {
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        if (ctx->window == widget) {
            if (gtkgreet->focused_window) {
                gtkgreet->focused_window = NULL;
            }
            free(ctx);
            g_array_remove_index_fast(gtkgreet->windows, idx);
            return;
        }
    }
}

void gtkgreet_setup_question(struct GtkGreet *gtkgreet, enum QuestionType type, char* question, char* error) {
    gtkgreet->question_type = type;
    for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
        struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
        setup_question(ctx, type, question, error);
    }
}