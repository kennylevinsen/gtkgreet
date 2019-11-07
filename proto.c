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
    char* respstr = (char*)calloc(1, header.payload_len+1);
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

struct json_object* roundtrip(struct json_object* req) {
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
