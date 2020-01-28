#ifndef _PROTO_H
#define _PROTO_H

#include "window.h"

struct proto_status {
    int success;
    int has_questions;
    enum QuestionType question_type;
    char question[64];
};

struct proto_status send_create_session(const char *username);
struct proto_status send_start_session(const char *username);
struct proto_status send_auth_answer(const char *answer);
struct proto_status send_cancel_session();

int send_login(const char *username, const char * password, const char *command);
int send_shutdown(const char *action);

#endif