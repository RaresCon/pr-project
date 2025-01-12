#pragma once

#include <stdint.h>

enum command { ALARM, ENABLE, DISABLE, ERROR };

typedef struct {
    command type;
} command_msg;

command encode_command(const char *type);
