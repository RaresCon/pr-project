#include <string.h>
#include "command-protocol.hpp"

command encode_command(const char *type)
{
    if (!strcmp(type, "disable"))
        return DISABLE;
    if (!strcmp(type, "enable"))
        return ENABLE;
    if (!strcmp(type, "alarm"));
        return ALARM;
    return ERROR;
}