#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "proto.h"
#include "window.h"

struct header {
    uint32_t magic;
    uint32_t version;
    uint32_t payload_len;
};

static int write_req(int fd, struct json_object* req) {
    const char* reqstr = json_object_get_string(req);
    uint32_t len = strlen(reqstr);
    if (write(fd, &len, 4) != 4) {
        return -1;
    }

    ssize_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, &reqstr[off], len-off);
        if (n < 0) {
            return -1;
        }
        off += n;
    }
 
    return 0;
}

static struct json_object* read_resp(int fd) {
    struct json_object* resp = NULL;
    char *respstr = NULL;
    uint32_t len;
    ssize_t off = 0;

    while (off < 4) {
        char* headerp = (char*)&len;
        ssize_t n = read(fd, &headerp[off], 4-off);
        if (n < 0) {
            goto end;
        }
        off += n;
    }

    off = 0;
    respstr = (char*)calloc(1,len+1);
    while (off <len) {
        int n = read(fd, &respstr[off],len-off);
        if (n < 0) {
            goto end;
        }
        off += n;
    }

    resp = json_tokener_parse(respstr);

end:
    if (respstr != NULL) {
        free(respstr);
    }
    return resp;
}

static int connectto(const char* path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "unable to open socket\n");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "connect: error: %d\n", errno);
        close(fd);
        return -1;
    }

    return fd;
}

static struct json_object* roundtrip(struct json_object* req) {
    struct json_object* resp = NULL;
    int fd;
    char* greetd_sock = getenv("GREETD_SOCK");

    if (greetd_sock == NULL) {
        fprintf(stderr, "GREETD_SOCK not set\n");
        return NULL;
    }

    if ((fd = connectto(greetd_sock)) < 0) {
        fprintf(stderr, "unable to connect to socket\n");
        goto end;
    }

    if (write_req(fd, req) != 0) {
        fprintf(stderr, "unable to write request to socket\n");
        goto end;
    }

    if ((resp = read_resp(fd)) == NULL) {
        fprintf(stderr, "unable to read response from socket\n");
        goto end;
    }

end:
    if (fd > 0) {
        close(fd);
    }
    return resp;
}

struct proto_status send_create_session(const char *username) {
    struct json_object* req = json_object_new_object();
    json_object_object_add(req, "type", json_object_new_string("create_session"));
    json_object_object_add(req, "username", json_object_new_string(username));
    struct json_object* resp = roundtrip(req);

    struct proto_status ret;

    struct json_object* type = json_object_object_get(resp, "type");
    const char* typestr = json_object_get_string(type);
    if (typestr == NULL || strcmp(typestr, "error") == 0) {
        ret.success = 0;
        goto done;
    }

    if (strcmp(typestr, "success") == 0) {
        ret.success = 1;
        ret.has_questions = 0;
        goto done;
    }

    if (strcmp(typestr, "auth_question") == 0) {
        ret.success = 1;
        ret.has_questions = 1;
        struct json_object *style = json_object_object_get(resp, "style");
        const char* stylestr = json_object_get_string(style);
        if (strcmp(stylestr, "visible") == 0) {
            ret.question_type = QuestionTypeVisible;
        } else if (strcmp(stylestr, "secret") == 0) {
            ret.question_type = QuestionTypeSecret;
        } else if (strcmp(stylestr, "info") == 0) {
            ret.question_type = QuestionTypeInfo;
        } else if (strcmp(stylestr, "error") == 0) {
            ret.question_type = QuestionTypeError;
        } else {
            ret.success = 0;
            goto done;
        }

        struct json_object *question = json_object_object_get(resp, "question");
        const char* questionstr = json_object_get_string(question);
        strncpy(ret.question, questionstr, 63);
        goto done;
    }

    ret.success = 0;

done:
    json_object_put(resp);
    return ret;
}

struct proto_status send_auth_answer(const char *answer) {
    struct json_object* req = json_object_new_object();
    json_object_object_add(req, "type", json_object_new_string("answer_auth_question"));
    json_object_object_add(req, "answer", json_object_new_string(answer));
    struct json_object* resp = roundtrip(req);

    struct proto_status ret;

    struct json_object* type = json_object_object_get(resp, "type");
    const char* typestr = json_object_get_string(type);
    if (typestr == NULL || strcmp(typestr, "error") == 0) {
        ret.success = 0;
        goto done;
    }

    if (strcmp(typestr, "success") == 0) {
        ret.success = 1;
        ret.has_questions = 0;
        goto done;
    }

    if (strcmp(typestr, "auth_question") == 0) {
        ret.success = 1;
        ret.has_questions = 1;
        struct json_object *style = json_object_object_get(resp, "style");
        const char* stylestr = json_object_get_string(style);
        if (strcmp(stylestr, "visible") == 0) {
            ret.question_type = QuestionTypeVisible;
        } else if (strcmp(stylestr, "secret") == 0) {
            ret.question_type = QuestionTypeSecret;
        } else if (strcmp(stylestr, "info") == 0) {
            ret.question_type = QuestionTypeInfo;
        } else if (strcmp(stylestr, "error") == 0) {
            ret.question_type = QuestionTypeError;
        } else {
            ret.success = 0;
            goto done;
        }

        struct json_object *question = json_object_object_get(resp, "question");
        const char* questionstr = json_object_get_string(question);
        strncpy(ret.question, questionstr, 63);
        goto done;
    }

    ret.success = 0;

done:
    json_object_put(resp);
    return ret;
}


struct proto_status send_start_session(const char *command) {
    struct json_object* req = json_object_new_object();
    json_object_object_add(req, "type", json_object_new_string("start_session"));

    struct json_object* cmd = json_object_new_array();
    json_object_array_add(cmd, json_object_new_string(command));
    json_object_object_add(req, "command", cmd);

    struct json_object* env = json_object_new_array();

    char buf[128];
    snprintf(buf, 128, "XDG_SESSION_DESKTOP=%s", command);
    json_object_array_add(env, json_object_new_string(buf));
    snprintf(buf, 128, "XDG_CURRENT_DESKTOP=%s", command);
    json_object_array_add(env, json_object_new_string(buf));

    json_object_object_add(req, "env", env);

    struct json_object* resp = roundtrip(req);

    struct proto_status ret;

    struct json_object* type = json_object_object_get(resp, "type");
    const char* typestr = json_object_get_string(type);
    if (typestr == NULL || strcmp(typestr, "error") == 0) {
        ret.success = 0;
        goto done;
    }

    if (strcmp(typestr, "success") == 0) {
        ret.success = 1;
        ret.has_questions = 0;
        goto done;
    }

    ret.success = 0;

done:
    json_object_put(resp);
    return ret;
}

struct proto_status send_cancel_session() {
    struct json_object* req = json_object_new_object();
    json_object_object_add(req, "type", json_object_new_string("cancel_session"));
    struct json_object* resp = roundtrip(req);

    struct proto_status ret;

    struct json_object* type = json_object_object_get(resp, "type");
    const char* typestr = json_object_get_string(type);
    if (typestr == NULL || strcmp(typestr, "error") == 0) {
        ret.success = 0;
        goto done;
    }

    if (strcmp(typestr, "success") == 0) {
        ret.success = 1;
        ret.has_questions = 0;
        goto done;
    }

    ret.success = 0;

done:
    json_object_put(resp);
    return ret;
}

int send_login(const char *username, const char * password, const char *command) {
    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("login"));
    json_object_object_add(login_req, "username", json_object_new_string(username));
    json_object_object_add(login_req, "password", json_object_new_string(password));

    struct json_object* cmd = json_object_new_array();
    json_object_array_add(cmd, json_object_new_string(command));
    json_object_object_add(login_req, "command", cmd);

    struct json_object* env = json_object_new_array();

    char buf[128];
    snprintf(buf, 128, "XDG_SESSION_DESKTOP=%s", command);
    json_object_array_add(env, json_object_new_string(buf));
    snprintf(buf, 128, "XDG_CURRENT_DESKTOP=%s", command);
    json_object_array_add(env, json_object_new_string(buf));

    json_object_object_add(login_req, "env", env);

    struct json_object* resp = roundtrip(login_req);

    json_object_put(login_req);

    struct json_object* type;

    json_object_object_get_ex(resp, "type", &type);

    const char* typestr = json_object_get_string(type);

    int ret = typestr != NULL && strcmp(typestr, "success") == 0;
    json_object_put(resp);
    return ret;
}

int send_shutdown(const char *action) {
    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("shutdown"));
    json_object_object_add(login_req, "action", json_object_new_string(action));
    struct json_object* resp = roundtrip(login_req);

    json_object_put(login_req);

    struct json_object* type;

    json_object_object_get_ex(resp, "type", &type);

    const char* typestr = json_object_get_string(type);

    int ret = typestr != NULL && strcmp(typestr, "success") == 0;
    json_object_put(resp);
    return ret;
}