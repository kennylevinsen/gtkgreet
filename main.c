#include <gtk/gtk.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <time.h>

#include "proto.h"

struct context {
    GtkWidget *window;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *info_label;
    GtkWidget *clock_label;
    GtkWidget *target_combo_box;
};

struct context ctx;

static gboolean draw_clock(gpointer data) {
    struct context *ctx = (struct context*)data;
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

static void login(GtkWidget *widget, gpointer data) {
    struct context *ctx = (struct context*)data;
    gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span>Logging in</span>");

    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("login"));
    json_object_object_add(login_req, "username", json_object_new_string(gtk_entry_get_text((GtkEntry*)ctx->username_entry)));
    json_object_object_add(login_req, "password", json_object_new_string(gtk_entry_get_text((GtkEntry*)ctx->password_entry)));

    gchar* selection = gtk_combo_box_text_get_active_text((GtkComboBoxText*)ctx->target_combo_box);

    struct json_object* cmd = json_object_new_array();
    json_object_array_add(cmd, json_object_new_string(selection));
    json_object_object_add(login_req, "command", cmd);

    struct json_object* env = json_object_new_object();
    json_object_object_add(env, "XDG_SESSION_DESKTOP", json_object_new_string(selection));
    json_object_object_add(env, "XDG_CURRENT_DESKTOP", json_object_new_string(selection));
    json_object_object_add(login_req, "env", env);

    struct json_object* resp = roundtrip(login_req);

    json_object_put(login_req);

    struct json_object* type;

    json_object_object_get_ex(resp, "type", &type);

    const char* typestr = json_object_get_string(type);

    if (typestr == NULL || strcmp(typestr, "success") != 0) {
        gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span color=\"darkred\">Login failed</span>");
    } else {
        exit(0);
    }

    json_object_put(resp);
    g_free(selection);
}


static void shutdown_action(struct context *ctx, char *action) {
    gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span>Logging in</span>");

    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("shutdown"));
    json_object_object_add(login_req, "action", json_object_new_string(action));
    struct json_object* resp = roundtrip(login_req);

    json_object_put(login_req);

    struct json_object* type;

    json_object_object_get_ex(resp, "type", &type);

    const char* typestr = json_object_get_string(type);

    if (typestr == NULL || strcmp(typestr, "success") != 0) {
        gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span color=\"darkred\">Exit action failed</span>");
    } else {
        exit(0);
    }

    json_object_put(resp);
}

static void shutdown_prompt(GtkWidget *widget, gpointer data) {
    struct context *ctx = (struct context*)data;
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

static void activate(GtkApplication *app, gpointer user_data) {
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);

    ctx.window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(ctx.window), "Greeter");
    gtk_window_set_default_size(GTK_WINDOW(ctx.window), 200, 200);

    GtkWidget *window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(ctx.window), window_box);
    gtk_widget_set_valign(window_box, GTK_ALIGN_CENTER);

    ctx.clock_label = gtk_label_new("");
    g_object_set(ctx.clock_label, "margin-bottom", 10, NULL);
    gtk_container_add(GTK_CONTAINER(window_box), ctx.clock_label);
    g_timeout_add(5000, draw_clock, &ctx);
    draw_clock(&ctx);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(input_box, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(input_box, 512, -1);
    gtk_container_add(GTK_CONTAINER(window_box), input_box);

    ctx.username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)ctx.username_entry, "Username");
    gtk_container_add(GTK_CONTAINER(input_box), ctx.username_entry);

    ctx.password_entry = gtk_entry_new();
    g_signal_connect(ctx.password_entry, "activate", G_CALLBACK(login), &ctx);
    gtk_entry_set_placeholder_text((GtkEntry*)ctx.password_entry, "Password");
    gtk_entry_set_input_purpose((GtkEntry*)ctx.password_entry, GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility((GtkEntry*)ctx.password_entry, FALSE);
    gtk_container_add(GTK_CONTAINER(input_box), ctx.password_entry);

    ctx.target_combo_box = gtk_combo_box_text_new_with_entry();
    update_env_combobox(ctx.target_combo_box);
    GtkWidget *combo_box_entry = gtk_bin_get_child((GtkBin*)ctx.target_combo_box);
    gtk_entry_set_placeholder_text((GtkEntry*)combo_box_entry, "Command to run");
    gtk_combo_box_set_active((GtkComboBox*)ctx.target_combo_box, 0);
    gtk_container_add(GTK_CONTAINER(input_box), ctx.target_combo_box);

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(input_box), bottom_box);

    ctx.info_label = gtk_label_new("");
    gtk_widget_set_halign(ctx.info_label, GTK_ALIGN_START);
    g_object_set(ctx.info_label, "margin-right", 10, NULL);
    gtk_container_add(GTK_CONTAINER(bottom_box), ctx.info_label);

    GtkWidget *shutdown_button = gtk_button_new_with_label("Shutdown");
    g_signal_connect(shutdown_button, "clicked", G_CALLBACK(shutdown_prompt), &ctx);
    gtk_widget_set_halign(shutdown_button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), shutdown_button);

    GtkWidget *button = gtk_button_new_with_label("Login");
    g_signal_connect(button, "clicked", G_CALLBACK(login), &ctx);
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), button);

    gtk_widget_show_all(ctx.window);
}

int main (int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("wtf.kl.gtkgreet", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
