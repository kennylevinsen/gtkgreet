#include <time.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "proto.h"
#include "window.h"
#include "gtkgreet.h"
#include "auth_question.h"

#ifdef LAYER_SHELL
#   include <gtk-layer-shell.h>

    static void window_set_focus_layer_shell(struct Window *win) {
        if (gtkgreet->focused_window != NULL) {
            gtk_layer_set_keyboard_interactivity(GTK_WINDOW(gtkgreet->focused_window->window), FALSE);
        }
        gtk_layer_set_keyboard_interactivity(GTK_WINDOW(win->window), TRUE);
    }

    static gboolean window_enter_notify(GtkWidget *widget, gpointer data) {
        struct Window *win = gtkgreet_window_by_widget(gtkgreet, widget);
        window_set_focus(win);
        return FALSE;
    }

    static void window_setup_layershell(struct Window *ctx) {
        if (gtkgreet->use_layer_shell) {

            gtk_widget_add_events(ctx->window, GDK_ENTER_NOTIFY_MASK);
            g_signal_connect(ctx->window, "enter-notify-event", G_CALLBACK(window_enter_notify), NULL);

            gtk_layer_init_for_window(GTK_WINDOW(ctx->window));
            gtk_layer_set_layer(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_LAYER_TOP);
            gtk_layer_set_monitor(GTK_WINDOW(ctx->window), ctx->monitor);
            gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(ctx->window));
        }
    }

#else
    static void window_setup_layershell(struct Window *ctx) {}
    static void window_set_focus_layer_shell(struct Window *win) {}
#endif

static gboolean draw_clock(gpointer data) {
    struct Window *ctx = (struct Window*)data;
    time_t now = time(&now);
    struct tm *now_tm = localtime(&now);
    if (now_tm == NULL) {
        return TRUE;
    }

    char time[48];
    g_snprintf(time, 48, "<span size='xx-large'>%02d:%02d</span>", now_tm->tm_hour, now_tm->tm_min);
    gtk_label_set_markup((GtkLabel*)ctx->clock_label, time);

    return TRUE;
}

static void window_setup(struct Window *ctx) {
    window_setup_layershell(ctx);

    gtk_window_set_title(GTK_WINDOW(ctx->window), "Greeter");
    gtk_window_set_default_size(GTK_WINDOW(ctx->window), 200, 200);

    GtkWidget *window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    g_object_set(window_box, "margin-bottom", 100, NULL);
    g_object_set(window_box, "margin-top", 100, NULL);
    g_object_set(window_box, "margin-left", 100, NULL);
    g_object_set(window_box, "margin-right", 100, NULL);
    gtk_container_add(GTK_CONTAINER(ctx->window), window_box);
    gtk_widget_set_valign(window_box, GTK_ALIGN_CENTER);

    ctx->clock_label = gtk_label_new("");
    g_object_set(ctx->clock_label, "margin-bottom", 10, NULL);
    gtk_container_add(GTK_CONTAINER(window_box), ctx->clock_label);
    g_timeout_add(5000, draw_clock, ctx);
    draw_clock(ctx);

    ctx->input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(ctx->input_box, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(ctx->input_box, 384, -1);
    gtk_container_add(GTK_CONTAINER(window_box), ctx->input_box);

    setup_question(ctx, QuestionTypeInitial, "Username:", NULL);

    gtk_widget_show_all(ctx->window);
}

static void window_destroy_notify(GtkWidget *widget, gpointer data) {
    gtkgreet_remove_window_by_widget(gtkgreet, widget);
}


void window_set_focus(struct Window* win) {
    assert(win != NULL);
    window_set_focus_layer_shell(win);
    gtkgreet->focused_window = win;
}

void create_window(GdkMonitor *monitor) {
    struct Window *w = calloc(1, sizeof(struct Window));
    if (w == NULL) {
        fprintf(stderr, "failed to allocate Window instance\n");
        exit(1);
    }
    w->monitor = monitor;
    g_array_append_val(gtkgreet->windows, w);

    w->window = gtk_application_window_new(gtkgreet->app);
    g_signal_connect(w->window, "destroy", G_CALLBACK(window_destroy_notify), NULL);

    window_setup(w);
}
