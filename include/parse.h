#ifndef PARSE_H
#define PARSE_H

#define FILE_SIG 0x53434442

struct dbheader_t {
	unsigned int sig;
	unsigned short version;
	unsigned short count;
	unsigned int filesize;
};

struct employee_t {
	char name[MAX_STR_LEN];
	char address[MAX_STR_LEN];
	unsigned int hours;
};

int create_dbheader(struct dbheader_t **dbheaderOut);
int validate_dbheader(int fd, struct dbheader_t **dbheaderOut);
int read_employees(int fd, struct dbheader_t *dbheader, struct employee_t **employeesOut);
int add_employee(char *add_string, struct dbheader_t *dbheader, struct employee_t *employees);
int list_employees(struct dbheader_t *dbheader, struct employee_t *employees);
void output_file(int fd, struct dbheader_t *dbheader, struct employee_t *employees);

#endif