#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

struct header {
    uint32_t magic;
    uint32_t version;
    uint32_t payload_len;
};

static int write_req(int fd, struct json_object* req) {
    const char* reqstr = json_object_get_string(req);
    struct header header = {
        .magic = 0xAFBFCFDF,
        .version = 1,
        .payload_len = strlen(reqstr),
    };

    if (write(fd, &header, sizeof(struct header)) != sizeof(struct header)) {
        return -1;
    }

    ssize_t off = 0;
    while (off < header.payload_len) {
        ssize_t n = write(fd, &reqstr[off], header.payload_len-off);
        if (n < 0) {
            return -1;
        }
        off += n;
    }
 
    return 0;
}

static struct json_object* read_resp(int fd) {
    struct header header = {};
    struct json_object* resp = NULL;
    char *respstr = NULL;
    ssize_t off = 0;

    while (off < sizeof(struct header)) {
        char* headerp = (char*)&header;
        ssize_t n = read(fd, &headerp[off], sizeof(struct header)-off);
        if (n < 0) {
            goto end;
        }
        off += n;
    }


    if (header.magic != 0xAFBFCFDF ||
        header.version != 1) {
        goto end;
    }

    off = 0;
    respstr = (char*)calloc(1, header.payload_len+1);
    while (off < header.payload_len) {
        int n = read(fd, &respstr[off], header.payload_len-off);
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

int send_login(const char *username, const char * password, const char *command) {
    struct json_object* login_req = json_object_new_object();
    json_object_object_add(login_req, "type", json_object_new_string("login"));
    json_object_object_add(login_req, "username", json_object_new_string(username));
    json_object_object_add(login_req, "password", json_object_new_string(password));

    struct json_object* cmd = json_object_new_array();
    json_object_array_add(cmd, json_object_new_string(command));
    json_object_object_add(login_req, "command", cmd);

    struct json_object* env = json_object_new_object();
    json_object_object_add(env, "XDG_SESSION_DESKTOP", json_object_new_string(command));
    json_object_object_add(env, "XDG_CURRENT_DESKTOP", json_object_new_string(command));
    json_object_object_add(login_req, "env", env);

    struct json_object* resp = roundtrip(login_req);

    json_object_put(login_req);

    struct json_object* type;

    json_object_object_get_ex(resp, "type", &type);

    const char* typestr = json_object_get_string(type);

    int ret = typestr == NULL || strcmp(typestr, "success") != 0;
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

    int ret = typestr == NULL || strcmp(typestr, "success") != 0;
    json_object_put(resp);
    return ret;
}