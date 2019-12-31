#ifndef _PROTO_H
#define _PROTO_H

int send_login(const char *username, const char * password, const char *command);
int send_shutdown(const char *action);

#endif