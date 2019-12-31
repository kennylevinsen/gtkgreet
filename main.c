#include <assert.h>
#include <gtk/gtk.h>

#include "window.h"
#include "gtkgreet.h"

#ifdef LAYER_SHELL
static gboolean use_layer_shell = FALSE;
#endif

static GOptionEntry entries[] =
{

#ifdef LAYER_SHELL
  { "layer-shell", 'l', 0, G_OPTION_ARG_NONE, &use_layer_shell, "Use layer shell", NULL},
#endif

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
                // We already have this monitor, remove from dead_windows list and
                // go to the next monitor.
                for (guint ydx = 0; ydx < dead_windows->len; ydx++) {
                    if (w == g_array_index(dead_windows, struct Window*, ydx)) {
                        g_array_remove_index_fast(dead_windows, ydx);
                        break;
                    }
                }
                continue;
            }

            create_window(monitor);
        }

        // Remove all windows left behind
        for (guint idx = 0; idx < dead_windows->len; idx++) {
            struct Window *w = g_array_index(dead_windows, struct Window*, idx);
            fprintf(stderr, "deleting window %p\n", w);
            gtk_widget_destroy(w->window);
            if (gtkgreet->focused_window == w) {
                gtkgreet->focused_window = NULL;
            }
        }

        if (gtkgreet->focused_window == NULL &&
                gtkgreet->windows->len > 0) {
            struct Window *w = g_array_index(gtkgreet->windows, struct Window*, gtkgreet->windows->len-1);
            assert(w != NULL);
            window_set_focus(w);
        }
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
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);
    if (!setup_layer_shell()) {
        create_window(NULL);
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

    gtkgreet = calloc(1, sizeof(struct GtkGreet));
    gtkgreet->windows = g_array_new(FALSE, TRUE, sizeof(struct Window*));
    gtkgreet->app = gtk_application_new("wtf.kl.gtkgreet", G_APPLICATION_FLAGS_NONE);

#ifdef LAYER_SHELL
    gtkgreet->use_layer_shell = use_layer_shell;
#endif

    g_signal_connect(gtkgreet->app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(gtkgreet->app), argc, argv);
    g_object_unref(gtkgreet->app);

    return status;
}
