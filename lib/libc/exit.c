#include "env.h"

__attribute__((noreturn)) void exit(int code) {
    task_exit(code);
}