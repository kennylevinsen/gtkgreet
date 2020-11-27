#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <gtk/gtk.h>

#include "window.h"
#include "gtkgreet.h"

struct GtkGreet *gtkgreet = NULL;

static char* command = NULL;
static char* background = NULL;

#ifdef LAYER_SHELL
static gboolean use_layer_shell = FALSE;
#endif

static GOptionEntry entries[] =
{

#ifdef LAYER_SHELL
  { "layer-shell", 'l', 0, G_OPTION_ARG_NONE, &use_layer_shell, "Use layer shell", NULL},
#endif
  { "command", 'c', 0, G_OPTION_ARG_STRING, &command, "Command to run", "sway"},
  { "background", 'b', 0, G_OPTION_ARG_STRING, &background, "Background image to use", NULL},
  { NULL }
};

#ifdef LAYER_SHELL
    static void reload_outputs() {
        GdkDisplay *display = gdk_display_get_default();

        // Make note of all existing windows
        GArray *dead_windows = g_array_new(FALSE, TRUE, sizeof(struct Window*));
        for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
            struct Window *ctx = g_array_index(gtkgreet->windows, struct Window*, idx);
            g_array_append_val(dead_windows, ctx);
        }

        // Go through all monitors
        for (int i = 0; i < gdk_display_get_n_monitors(display); i++) {
            GdkMonitor *monitor = gdk_display_get_monitor(display, i);
            struct Window *w = gtkgreet_window_by_monitor(gtkgreet, monitor);
            if (w != NULL) {
                // We already have this monitor, remove from dead_windows list
                for (guint ydx = 0; ydx < dead_windows->len; ydx++) {
                    if (w == g_array_index(dead_windows, struct Window*, ydx)) {
                        g_array_remove_index_fast(dead_windows, ydx);
                        break;
                    }
                }
            } else {
                create_window(monitor);
            }
        }

        // Remove all windows left behind
        for (guint idx = 0; idx < dead_windows->len; idx++) {
            struct Window *w = g_array_index(dead_windows, struct Window*, idx);
            gtk_widget_destroy(w->window);
            if (gtkgreet->focused_window == w) {
                gtkgreet->focused_window = NULL;
            }
        }

        for (guint idx = 0; idx < gtkgreet->windows->len; idx++) {
            struct Window *win = g_array_index(gtkgreet->windows, struct Window*, idx);
            window_configure(win);
        }

        g_array_unref(dead_windows);
    }

    static void monitors_changed(GdkDisplay *display, GdkMonitor *monitor) {
        reload_outputs();
    }

    static gboolean setup_layer_shell() {
        if (gtkgreet->use_layer_shell) {
            reload_outputs();
            GdkDisplay *display = gdk_display_get_default();
            g_signal_connect(display, "monitor-added", G_CALLBACK(monitors_changed), NULL);
            g_signal_connect(display, "monitor-removed", G_CALLBACK(monitors_changed), NULL);
            return TRUE;
        } else {
            return FALSE;
        }
    }
#else
    static gboolean setup_layer_shell() {
        return FALSE;
    }
#endif

static void activate(GtkApplication *app, gpointer user_data) {
    gtkgreet_activate(gtkgreet);
    if (!setup_layer_shell()) {
        struct Window *win = create_window(NULL);
        gtkgreet_focus_window(gtkgreet, win);
        window_configure(win);
    }
}

int main (int argc, char **argv) {
    GError *error = NULL;
    GOptionContext *option_context = g_option_context_new("- GTK-based greeter for greetd");
    g_option_context_add_main_entries(option_context, entries, NULL);
    g_option_context_add_group(option_context, gtk_get_option_group(TRUE));
    if (!g_option_context_parse(option_context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(1);
    }

    gtkgreet = create_gtkgreet();

#ifdef LAYER_SHELL
    gtkgreet->use_layer_shell = use_layer_shell;
#endif
    gtkgreet->command = command;

    if (background != NULL) {
        gtkgreet->background = gdk_pixbuf_new_from_file(background, &error);
        if (gtkgreet->background == NULL) {
            g_print("background loading failed: %s\n", error->message);
        }
    }

    g_signal_connect(gtkgreet->app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(gtkgreet->app), argc, argv);

    gtkgreet_destroy(gtkgreet);

    return status;
}
