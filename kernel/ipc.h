#pragma once

#include "message.h"

int ipc_send(int dst_tid, struct message *msg);
int ipc_receive(int src_tid, struct message *msg);