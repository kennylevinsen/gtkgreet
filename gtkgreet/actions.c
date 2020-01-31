#include <time.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "actions.h"
#include "proto.h"
#include "gtkgreet.h"
#include "window.h"

void action_answer_question(GtkWidget *widget, gpointer data) {
    struct Window *ctx = data;
    struct response resp;
    switch (gtkgreet->question_type) {
        case QuestionTypeInitial: {
            if (gtkgreet->selected_command) {
                free(gtkgreet->selected_command);
                gtkgreet->selected_command = NULL;
            }
            gtkgreet->selected_command = strdup(gtk_combo_box_text_get_active_text((GtkComboBoxText*)ctx->command_selector));

            struct request req = {
                .request_type = request_type_create_session,
            };
            if (ctx->input_field != NULL) {
                strncpy(req.body.request_create_session.username, gtk_entry_get_text((GtkEntry*)ctx->input_field), 127);
            }
            resp = roundtrip(req);
            break;
        }
        case QuestionTypeSecret:
        case QuestionTypeVisible: {
            struct request req = {
                .request_type = request_type_post_auth_message_response,
            };
            if (ctx->input_field != NULL) {
                strncpy(req.body.request_post_auth_message_response.response, gtk_entry_get_text((GtkEntry*)ctx->input_field), 127);
            }
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
            strncpy(req.body.request_start_session.cmd, gtkgreet->selected_command, 127);
            resp = roundtrip(req);
            if (resp.response_type == response_type_success) {
                exit(0);
            }
            req.request_type = request_type_cancel_session,
            resp = roundtrip(req);

            char* error = NULL;
            if (resp.response_type == response_type_error) {
                if (resp.body.response_error.error_type == error_type_auth) {
                    error = "Login failed";
                } else {
                    error = resp.body.response_error.description;
                }
            }
            gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, INITIAL_QUESTION, error);
            break;
        }
        case response_type_auth_message: {
            gtkgreet_setup_question(gtkgreet,
                (enum QuestionType)resp.body.response_auth_message.auth_message_type,
                resp.body.response_auth_message.auth_message,
                NULL);
            break;
        }
        case response_type_roundtrip_error:
        case response_type_error: {
            struct request req = {
                .request_type = request_type_cancel_session,
            };

            char* error = NULL;
            if (resp.response_type == response_type_error) {
                if (resp.body.response_error.error_type == error_type_auth) {
                    error = "Login failed";
                } else {
                    error = resp.body.response_error.description;
                }
            }
            roundtrip(req);
            gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, INITIAL_QUESTION, error);
            break;
        }
    }
}

void action_quit(GtkWidget *widget, gpointer data) {
    exit(0);
}
void action_cancel_question(GtkWidget *widget, gpointer data) {
    struct request req = {
        .request_type = request_type_cancel_session,
    };
    struct response resp = roundtrip(req);
    if (resp.response_type != response_type_success) {
        exit(1);
    }

    gtkgreet_setup_question(gtkgreet, QuestionTypeInitial, INITIAL_QUESTION, NULL);
}
