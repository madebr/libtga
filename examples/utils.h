#ifndef UTILS_H
#define UTILS_H

void print_message(const char *msg, const char *file, unsigned line);

#define TGA_EXAMPLE_ERROR(msg) print_message((msg), __FILE__, __LINE__)

#endif /* UTILS_H */
