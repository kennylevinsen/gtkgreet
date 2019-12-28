#include <gtk/gtk.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <time.h>

#include "proto.h"

struct context {
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

    char time[14];
    g_snprintf(time, 14, "Login - %02d:%02d", now_tm->tm_hour, now_tm->tm_min);
    gtk_label_set_text((GtkLabel*)ctx->clock_label, time);

    return TRUE;
}

static void login(GtkWidget *widget, gpointer data) {
    struct context *ctx = (struct context*)data;
    gtk_label_set_markup((GtkLabel*)ctx->info_label, "<span>Logging in</span>");

    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("login"));
    json_object_object_add(login_req, "username", json_object_new_string(gtk_entry_get_text((GtkEntry*)ctx->username_entry)));
    json_object_object_add(login_req, "password", json_object_new_string(gtk_entry_get_text((GtkEntry*)ctx->password_entry)));

    char* selection = gtk_combo_box_text_get_active_text((GtkComboBoxText*)ctx->target_combo_box);

    struct json_object* cmd = json_object_new_array();
    json_object_array_add(cmd, json_object_new_string(selection));
    json_object_object_add(login_req, "command", cmd);

    struct json_object* env = json_object_new_object();
    json_object_object_add(env, "XDG_SESSION_DESKTOP", json_object_new_string(selection));
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
}

static void quit(GtkWidget *widget, gpointer data) {
    exit(0);
}

static void activate(GtkApplication *app, gpointer user_data) {
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Greeter");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    GtkWidget *window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), window_box);
    gtk_widget_set_valign(window_box, GTK_ALIGN_CENTER);

    ctx.clock_label = gtk_label_new("Login - 12:34");
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
    gtk_combo_box_text_append((GtkComboBoxText*)ctx.target_combo_box, NULL, "sway");
    gtk_combo_box_text_append((GtkComboBoxText*)ctx.target_combo_box, NULL, "bash");
    gtk_combo_box_set_active((GtkComboBox*)ctx.target_combo_box, 0);
    gtk_container_add(GTK_CONTAINER(input_box), ctx.target_combo_box);

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(input_box), bottom_box);

    ctx.info_label = gtk_label_new("");
    gtk_widget_set_halign(ctx.info_label, GTK_ALIGN_START);
    g_object_set(ctx.info_label, "margin-right", 10, NULL);
    gtk_container_add(GTK_CONTAINER(bottom_box), ctx.info_label);

    GtkWidget *quit_button = gtk_button_new_with_label("Quit");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(quit), &ctx);
    gtk_widget_set_halign(quit_button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), quit_button);

    GtkWidget *button = gtk_button_new_with_label("Login");
    g_signal_connect(button, "clicked", G_CALLBACK(login), &ctx);
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), button);

    gtk_widget_show_all(window);
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
