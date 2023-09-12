#pragma once

#include "message.h"

#define IPC_ANY     0
#define IPC_DENY    -1

int ipc_send(const char *name, struct message *msg);
int ipc_receive(int src, struct message *msg);
int ipc_call(const char *name, struct message *msg);