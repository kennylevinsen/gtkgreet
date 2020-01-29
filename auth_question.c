#include <time.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "auth_question.h"
#include "proto.h"
#include "window.h"
#include "gtkgreet.h"

static void answer_question_action(GtkWidget *widget, gpointer data) {
    struct response resp;
    switch (gtkgreet->question_type) {
        case QuestionTypeInitial: {
            struct request req = {
                .request_type = request_type_create_session,
            };
            strncpy(req.body.request_create_session.username, gtk_entry_get_text((GtkEntry*)widget), 127);
            resp = roundtrip(req);
            break;
        }
        case QuestionTypeSecret:
        case QuestionTypeVisible: {
            struct request req = {
                .request_type = request_type_post_auth_message_response,
            };
            strncpy(req.body.request_post_auth_message_response.response, gtk_entry_get_text((GtkEntry*)widget), 127);
            resp = roundtrip(req);
            break;
        }
        case QuestionTypeInfo:
        case QuestionTypeError: {
            struct request req = {
                .request_type = request_type_post_auth_message_response,
            };
            req.body.request_post_auth_message_response.response[0] = '\0';
            resp = roundtrip(req);
            break;
        }
    }

    switch (resp.response_type) {
        case response_type_success: {
            struct request req = {
                .request_type = request_type_start_session,
            };
            strncpy(req.body.request_start_session.cmd, gtkgreet->command, 127);
            resp = roundtrip(req);
            if (resp.response_type == response_type_success) {
                exit(0);
            }
            req.request_type = request_type_cancel_session,
            resp = roundtrip(req);

            char* error = NULL;
            if (resp.response_type == response_type_error) {
                error = resp.body.response_error.description;
            }
            gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, "Username:", error);
            break;
        }
        case response_type_auth_message: {
            gtkgreet_setup_question(gtkgreet,
                resp.body.response_auth_message.auth_message_type,
                resp.body.response_auth_message.auth_message,
                NULL);
            break;
        }
        case response_type_roundtrip_error:
        case response_type_error: {
            struct request req = {
                .request_type = request_type_cancel_session,
            };
            roundtrip(req);
            gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, "Username:", resp.body.response_error.description);
            break;
        }
    }
}

static void cancel_initial_action(GtkWidget *widget, gpointer data) {
    exit(0);
}
static void cancel_question_action(GtkWidget *widget, gpointer data) {
    struct request req = {
        .request_type = request_type_cancel_session,
    };
    struct response resp = roundtrip(req);
    if (resp.response_type != response_type_success) {
        exit(1);
    }

    gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, "Username:", NULL);
}

void setup_question(struct Window *ctx, enum QuestionType type, char* question, char* error) {
    if (ctx->input != NULL) {
        gtk_widget_destroy(ctx->input);
        ctx->input = NULL;
        ctx->input_field = NULL;
    }
    ctx->input = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *question_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(question_box, GTK_ALIGN_CENTER);
    switch (type) {
        case QuestionTypeInitial:
        case QuestionTypeVisible: {

            GtkWidget *label = gtk_label_new(question);
            gtk_widget_set_halign(label, GTK_ALIGN_START);
            gtk_container_add(GTK_CONTAINER(question_box), label);

            ctx->input_field = gtk_entry_new();
            g_signal_connect(ctx->input_field, "activate", G_CALLBACK(answer_question_action), ctx);
            gtk_widget_set_size_request(ctx->input_field, 384, -1);
            gtk_container_add(GTK_CONTAINER(question_box), ctx->input_field);
            break;
        }
        case QuestionTypeSecret: {
            GtkWidget *label = gtk_label_new(question);
            gtk_widget_set_halign(label, GTK_ALIGN_START);
            gtk_container_add(GTK_CONTAINER(question_box), label);

            ctx->input_field = gtk_entry_new();
            gtk_entry_set_input_purpose((GtkEntry*)ctx->input_field, GTK_INPUT_PURPOSE_PASSWORD);
            gtk_entry_set_visibility((GtkEntry*)ctx->input_field, FALSE);
            g_signal_connect(ctx->input_field, "activate", G_CALLBACK(answer_question_action), ctx);
            gtk_widget_set_size_request(ctx->input_field, 384, -1);
            gtk_container_add(GTK_CONTAINER(question_box), ctx->input_field);
            break;
        }
        case QuestionTypeInfo:
        case QuestionTypeError:
            break;
    }

    gtk_container_add(GTK_CONTAINER(ctx->input), question_box);
    gtk_container_add(GTK_CONTAINER(ctx->input_box), ctx->input);

    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(ctx->input), bottom_box);

    if (error != NULL) {
        GtkWidget *label = gtk_label_new(error);
        char err[128];
        snprintf(err, 128, "<span color=\"red\">%s</span>", error);
        gtk_label_set_markup((GtkLabel*)label, err);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_container_add(GTK_CONTAINER(bottom_box), label);
    }

    GtkWidget *button = gtk_button_new_with_label("Cancel");
    switch (type) {
        case QuestionTypeInitial: {
            g_signal_connect(button, "clicked", G_CALLBACK(cancel_initial_action), ctx);
            break;
        }
        case QuestionTypeVisible:
        case QuestionTypeSecret:
        case QuestionTypeInfo:
        case QuestionTypeError: {
            g_signal_connect(button, "clicked", G_CALLBACK(cancel_question_action), ctx);
            break;
        }
    }

    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(bottom_box), button);

    gtk_widget_show_all(ctx->window);

    if (ctx->input_field != NULL) {
        gtk_widget_grab_focus(ctx->input_field);
    }
}