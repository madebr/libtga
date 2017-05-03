#include "utils.h"

#include <stdio.h>

void print_message(const char *msg, const char *file, unsigned line)
{
	fprintf(stderr, "%s:%u %s\n", file, line, msg);
}
