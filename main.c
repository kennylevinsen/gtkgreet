#include <gtk/gtk.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include "proto.h"

struct context {
    GtkWidget *username;
    GtkWidget *password;
    GtkWidget *info;
};

struct context ctx;

static void login(GtkWidget *widget, gpointer data) {
    struct context *ctx = (struct context*)data;
    gtk_label_set_text((GtkLabel*)ctx->info, "Logging in");

    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("login"));
    json_object_object_add(login_req, "username", json_object_new_string(gtk_entry_get_text((GtkEntry*)ctx->username)));
    json_object_object_add(login_req, "password", json_object_new_string(gtk_entry_get_text((GtkEntry*)ctx->password)));

    struct json_object* cmd = json_object_new_array();
    json_object_array_add(cmd, json_object_new_string("sway"));
    json_object_object_add(login_req, "command", cmd);

    struct json_object* env = json_object_new_object();
    json_object_object_add(env, "XDG_SESSION_TYPE", json_object_new_string("wayland"));
    json_object_object_add(env, "XDG_SESSION_DESKTOP", json_object_new_string("sway"));
    json_object_object_add(login_req, "env", env);

    struct json_object* resp = roundtrip(login_req);

    json_object_put(login_req);

    struct json_object* type;

    json_object_object_get_ex(resp, "type", &type);

    const char* typestr = json_object_get_string(type);

    if (typestr == NULL || strcmp(typestr, "success") != 0) {
        gtk_label_set_markup((GtkLabel*)ctx->info, "<span color=\"darkred\">Login failed</span>");
    } else {
        exit(0);
    }

    json_object_put(resp);
}

static void activate (GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Greeter");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    GtkWidget *window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), window_box);
    gtk_widget_set_valign(window_box, GTK_ALIGN_CENTER);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(input_box, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(input_box, 512, -1);
    gtk_container_add(GTK_CONTAINER(window_box), input_box);

    ctx.username = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)ctx.username, "Username");
    gtk_container_add(GTK_CONTAINER(input_box), ctx.username);

    ctx.password = gtk_entry_new();
    g_signal_connect(ctx.password, "activate", G_CALLBACK(login), &ctx);
    gtk_entry_set_placeholder_text((GtkEntry*)ctx.password, "Password");
    gtk_entry_set_input_purpose((GtkEntry*)ctx.password, GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility((GtkEntry*)ctx.password, FALSE);
    gtk_container_add(GTK_CONTAINER(input_box), ctx.password);

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(input_box), bottom_box);

    ctx.info = gtk_label_new("");
    gtk_widget_set_halign(ctx.info, GTK_ALIGN_START);
    g_object_set(ctx.info, "margin-right", 10, NULL);
    gtk_container_add(GTK_CONTAINER(bottom_box), ctx.info);

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
