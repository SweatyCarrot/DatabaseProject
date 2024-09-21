#ifndef FILE_H
#define FILE_H

struct dbheader_t;
struct employee_t;

int create_db_file(char *file_name);
int open_db_file(char *file_name);

void exit_program(int fd, struct dbheader_t *dbheader, struct employee_t *employees);

#endif

