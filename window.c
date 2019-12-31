#include <time.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "proto.h"
#include "window.h"
#include "gtkgreet.h"


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

static void login_action(GtkWidget *widget, gpointer data) {
    struct Window *ctx = (struct Window*)data;
    gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span>Logging in</span>");

    const char *username = gtk_entry_get_text((GtkEntry*)ctx->username_entry);
    const char *password = gtk_entry_get_text((GtkEntry*)ctx->password_entry);
    gchar* selection = gtk_combo_box_text_get_active_text((GtkComboBoxText*)ctx->target_combo_box);

    if (send_login(username, password, selection)) {
        exit(0);
    } else {
        gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span color=\"darkred\">Login failed</span>");
    }

    g_free(selection);
}

static void shutdown_action(struct Window *ctx, char *action) {
    gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span>Executing shutdown action</span>");
    if (send_shutdown(action)) {
        exit(0);
    } else {
        gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span color=\"darkred\">Exit action failed</span>");
    }
}

static void window_create_shutdown_prompt(GtkWidget *widget, gpointer data) {
    struct Window *ctx = (struct Window*)data;
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(ctx->window),
        GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_NONE,
        "What do you want to do?");

    gtk_dialog_add_button((GtkDialog*)dialog, "Cancel", GTK_RESPONSE_REJECT);
    gtk_dialog_add_button((GtkDialog*)dialog, "Quit", 1);
    GtkWidget *reboot_button = gtk_dialog_add_button((GtkDialog*)dialog, "Reboot", 2);
    gtk_style_context_add_class(gtk_widget_get_style_context(reboot_button), "destructive-action");
    GtkWidget *poweroff_button = gtk_dialog_add_button((GtkDialog*)dialog, "Power off", 3);
    gtk_style_context_add_class(gtk_widget_get_style_context(poweroff_button), "destructive-action");


    switch (gtk_dialog_run((GtkDialog*)dialog)) {
        case 1:
            exit(0);
            break;
        case 2:
            shutdown_action(ctx, "reboot");
            break;
        case 3:
            shutdown_action(ctx, "poweroff");
            break;
        default:
            break;
    }
    gtk_widget_destroy(dialog);
}

static int update_env_combobox(GtkWidget *combobox) {
    char buffer[255];
    FILE *fp = fopen("/etc/greetd/environments", "r");
    if (fp == NULL) {
        return 0;
    }

    int entries = 0;
    while(fgets(buffer, 255, (FILE*) fp)) {
        size_t len = strnlen(buffer, 255);
        if (len > 0 && len < 255 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        gtk_combo_box_text_append((GtkComboBoxText*)combobox, NULL, buffer);
        entries++;
    }

    fclose(fp);
    return entries;
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

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(input_box, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(input_box, 512, -1);
    gtk_container_add(GTK_CONTAINER(window_box), input_box);

    ctx->username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)ctx->username_entry, "Username");
    gtk_container_add(GTK_CONTAINER(input_box), ctx->username_entry);

    ctx->password_entry = gtk_entry_new();
    g_signal_connect(ctx->password_entry, "activate", G_CALLBACK(login_action), ctx);
    gtk_entry_set_placeholder_text((GtkEntry*)ctx->password_entry, "Password");
    gtk_entry_set_input_purpose((GtkEntry*)ctx->password_entry, GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility((GtkEntry*)ctx->password_entry, FALSE);
    gtk_container_add(GTK_CONTAINER(input_box), ctx->password_entry);

    ctx->target_combo_box = gtk_combo_box_text_new_with_entry();
    update_env_combobox(ctx->target_combo_box);
    GtkWidget *combo_box_entry = gtk_bin_get_child((GtkBin*)ctx->target_combo_box);
    gtk_entry_set_placeholder_text((GtkEntry*)combo_box_entry, "Command to run");
    gtk_combo_box_set_active((GtkComboBox*)ctx->target_combo_box, 0);
    gtk_container_add(GTK_CONTAINER(input_box), ctx->target_combo_box);

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(input_box), bottom_box);

    ctx->info_label = gtk_label_new("");
    gtk_widget_set_halign(ctx->info_label, GTK_ALIGN_START);
    g_object_set(ctx->info_label, "margin-right", 10, NULL);
    gtk_container_add(GTK_CONTAINER(bottom_box), ctx->info_label);

    GtkWidget *shutdown_button = gtk_button_new_with_label("Shutdown");
    g_signal_connect(shutdown_button, "clicked", G_CALLBACK(window_create_shutdown_prompt), ctx);
    gtk_widget_set_halign(shutdown_button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), shutdown_button);

    GtkWidget *button = gtk_button_new_with_label("Login");
    g_signal_connect(button, "clicked", G_CALLBACK(login_action), ctx);
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), button);

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
