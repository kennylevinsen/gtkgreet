#ifndef _AUTH_QUESTION_H
#define _AUTH_QUESTION_H

#include "window.h"
#include "gtkgreet.h"

void setup_question(struct Window *ctx, enum QuestionType type, char* question, char* error);

#endif