#pragma once

#include "message.h"

#define IPC_ANY     0
#define IPC_DENY    -1

void ipc_share_buffer(char *buf, int len);
int ipc_copy(const char *name, char *buf);
int ipc_send(const char *name, struct message *msg);
int ipc_receive(int src, struct message *msg);
int ipc_call(const char *name, struct message *msg);