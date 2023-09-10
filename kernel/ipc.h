#pragma once

#include "message.h"

int ipc_send(const char *name, struct message *msg);
int ipc_receive(const char *name, struct message *msg);
int ipc_call(const char *name, struct message *msg);