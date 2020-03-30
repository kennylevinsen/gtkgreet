#include <time.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "proto.h"
#include "window.h"
#include "gtkgreet.h"
#include "actions.h"
#include "config.h"

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
        if (win == NULL)
            return FALSE;

        if (win != gtkgreet->focused_window) {
            window_set_focus_layer_shell(win);
            window_set_focus(win);
        }

        gtkgreet->focused_window = win;
        return FALSE;
    }

    static void window_setup_layershell(struct Window *ctx) {
            gtk_widget_add_events(ctx->window, GDK_ENTER_NOTIFY_MASK);
        if (ctx->enter_notify_handler > 0) {
            g_signal_handler_disconnect(ctx->window, ctx->enter_notify_handler);
            ctx->enter_notify_handler = 0;
        }
        ctx->enter_notify_handler = g_signal_connect(ctx->window, "enter-notify-event", G_CALLBACK(window_enter_notify), NULL);

        gtk_layer_init_for_window(GTK_WINDOW(ctx->window));
        gtk_layer_set_layer(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_LAYER_TOP);
        gtk_layer_set_monitor(GTK_WINDOW(ctx->window), ctx->monitor);
        gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(ctx->window));
        gtk_layer_set_margin(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_LEFT, 0);
        gtk_layer_set_margin(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_RIGHT, 0);
        gtk_layer_set_margin(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_TOP, 0);
        gtk_layer_set_margin(GTK_WINDOW(ctx->window), GTK_LAYER_SHELL_EDGE_BOTTOM, 0);

        GdkRectangle rect;
        gdk_monitor_get_workarea(ctx->monitor, &rect);
        gtk_widget_set_size_request(ctx->window, rect.width, rect.height);
    }

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

void window_setup_question(struct Window *ctx, enum QuestionType type, char* question, char* error) {
    if (ctx->input_box != NULL) {
        gtk_widget_destroy(ctx->input_box);
        ctx->input_box = NULL;

        // Children of the box
        ctx->input_field = NULL;
        ctx->command_selector = NULL;
    }
    ctx->input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *question_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(question_box, GTK_ALIGN_END);
    switch (type) {
        case QuestionTypeInitial:
        case QuestionTypeVisible:
        case QuestionTypeSecret: {
            GtkWidget *label = gtk_label_new(question);
            gtk_widget_set_halign(label, GTK_ALIGN_END);
            gtk_container_add(GTK_CONTAINER(question_box), label);

            ctx->input_field = gtk_entry_new();
            if (type == QuestionTypeSecret) {
                gtk_entry_set_input_purpose((GtkEntry*)ctx->input_field, GTK_INPUT_PURPOSE_PASSWORD);
                gtk_entry_set_visibility((GtkEntry*)ctx->input_field, FALSE);
            }
            g_signal_connect(ctx->input_field, "activate", G_CALLBACK(action_answer_question), ctx);
            gtk_widget_set_size_request(ctx->input_field, 384, -1);
            gtk_widget_set_halign(ctx->input_field, GTK_ALIGN_END);
            gtk_container_add(GTK_CONTAINER(question_box), ctx->input_field);
            break;
        }
        case QuestionTypeInfo:
        case QuestionTypeError: {
            GtkWidget *label = gtk_label_new(question);
            gtk_widget_set_halign(label, GTK_ALIGN_END);
            gtk_container_add(GTK_CONTAINER(question_box), label);
            break;
        }
    }

    gtk_container_add(GTK_CONTAINER(ctx->input_box), question_box);

    if (type == QuestionTypeInitial) {
        ctx->command_selector = gtk_combo_box_text_new_with_entry();
        gtk_widget_set_size_request(ctx->command_selector, 384, -1);
        config_update_command_selector(ctx->command_selector);
        gtk_widget_set_halign(ctx->command_selector, GTK_ALIGN_END);
        gtk_combo_box_set_active((GtkComboBox*)ctx->command_selector, 0);

        GtkWidget *selector_entry = gtk_bin_get_child((GtkBin*)ctx->command_selector);
        gtk_entry_set_placeholder_text((GtkEntry*)selector_entry, "Command to run on login");
        g_signal_connect(selector_entry, "activate", G_CALLBACK(action_answer_question), ctx);

        gtk_container_add(GTK_CONTAINER(ctx->input_box), ctx->command_selector);
    }

    gtk_container_add(GTK_CONTAINER(ctx->body), ctx->input_box);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(ctx->input_box), button_box);


    if (error != NULL) {
        GtkWidget *label = gtk_label_new(error);
        char err[128];
        snprintf(err, 128, "<span color=\"red\">%s</span>", error);
        gtk_label_set_markup((GtkLabel*)label, err);
        gtk_widget_set_halign(label, GTK_ALIGN_END);
        gtk_container_add(GTK_CONTAINER(button_box), label);
    }

    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    switch (type) {
        case QuestionTypeInitial: {
            g_signal_connect(cancel_button, "clicked", G_CALLBACK(action_quit), ctx);
            break;
        }
        case QuestionTypeVisible:
        case QuestionTypeSecret:
        case QuestionTypeInfo:
        case QuestionTypeError: {
            g_signal_connect(cancel_button, "clicked", G_CALLBACK(action_cancel_question), ctx);
            break;
        }
    }

    gtk_widget_set_halign(cancel_button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(button_box), cancel_button);

    GtkWidget *continue_button = gtk_button_new_with_label("Log in");
    g_signal_connect(continue_button, "clicked", G_CALLBACK(action_answer_question), ctx);
    GtkStyleContext *continue_button_style = gtk_widget_get_style_context(continue_button);
    gtk_style_context_add_class(continue_button_style, "suggested-action");

    gtk_widget_set_halign(continue_button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(button_box), continue_button);

    gtk_widget_show_all(ctx->window);

    if (ctx->input_field != NULL) {
        gtk_widget_grab_focus(ctx->input_field);
    }
}

static void window_empty(struct Window *ctx) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(ctx->window));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    ctx->body = NULL;
    ctx->input_box = NULL;
    ctx->input_field = NULL;
    ctx->command_selector = NULL;
    ctx->info_label = NULL;
    ctx->clock_label = NULL;
}

static void window_setup(struct Window *ctx) {
    // Clean up old elements
    window_empty(ctx);

    // Create new elements
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

    ctx->body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(ctx->body, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(ctx->body, 384, -1);
    gtk_container_add(GTK_CONTAINER(window_box), ctx->body);

    gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, INITIAL_QUESTION, NULL);

    gtk_widget_show_all(ctx->window);
}

static void window_destroy_notify(GtkWidget *widget, gpointer data) {
    gtkgreet_remove_window_by_widget(gtkgreet, widget);
}

void window_set_focus(struct Window* win) {
    assert(win != NULL);
    if (gtkgreet->focused_window != NULL && gtkgreet->focused_window != win) {
        struct Window* old = gtkgreet->focused_window;
        if (old->input_field != NULL && win->input_field != NULL) {
            // Get previous cursor position
            gint cursor_pos = 0;
            g_object_get((GtkEntry*)old->input_field, "cursor-position", &cursor_pos, NULL);

            // Move content
            gtk_entry_set_text((GtkEntry*)win->input_field, gtk_entry_get_text((GtkEntry*)old->input_field));
            gtk_entry_set_text((GtkEntry*)old->input_field, "");

            // Update new cursor position
            g_signal_emit_by_name((GtkEntry*)win->input_field, "move-cursor", GTK_MOVEMENT_BUFFER_ENDS, -1, FALSE);
            g_signal_emit_by_name((GtkEntry*)win->input_field, "move-cursor", GTK_MOVEMENT_LOGICAL_POSITIONS, cursor_pos, FALSE);
        }
    }
}

static void window_configure(struct Window *w) {
#ifdef LAYER_SHELL
    if (gtkgreet->use_layer_shell) {
        window_setup_layershell(w);
    }
#endif

    window_setup(w);
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
    gtk_window_set_title(GTK_WINDOW(w->window), "Greeter");
    gtk_window_set_default_size(GTK_WINDOW(w->window), 200, 200);

    window_configure(w);
}
