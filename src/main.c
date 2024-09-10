#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[])
{
	printf("Usage: %s [-n] new file? [-f <file_path>] (required)\n", argv[0]);
	return;
}

int main(int argc, char *argv[])
{
	int dbfd = -1;
	struct dbheader_t *dbheader = NULL;
	struct employee_t *employees = NULL;
	
	int opt = 0;

	bool stat = false;
	bool add = false;
	bool new_file = false;
	bool list = false;

	char *add_string = NULL;
	char *file_path = NULL;

	while ((opt = getopt(argc, argv, "nsla:f:")) != -1) {
		switch (opt) {
			case 'n':
				new_file = true;
				break;
			case 's':
				stat = true;
				break;
			case 'a':
				add = true;
				add_string = optarg;
				break;
			case 'l':
				list = true;
				break;
			case 'f':
				file_path = optarg;
				break;
			default:
				return STATUS_ERROR;
		}
	}

	//FLAG: FILE PATH
	if (file_path == NULL) {
		print_usage(argv);
		return STATUS_ERROR;
	}

	//FLAG: NEW FILE
	if (new_file) {
		dbfd = create_db_file(file_path);
		if (dbfd == -1) {
			printf("Error creating database file\n");
			return STATUS_ERROR;
		}
		
		if (create_dbheader(&dbheader) == STATUS_ERROR) {
			printf("Failed to create database header\n");
			return STATUS_ERROR;
		}
	} else {
		dbfd = open_db_file(file_path);
		if (dbfd == -1) {
			printf("Error opening database file. If this is your first time running the program, add -n.\n");
			return STATUS_ERROR;
		}
		if (validate_dbheader(dbfd, &dbheader) == STATUS_ERROR) {
			printf("Error validating dbheader\n");
			return STATUS_ERROR;
		}
	}

	if(read_employees(dbfd, dbheader, &employees) != STATUS_SUCCESS) {
		printf("Failed to read employees!\n");
		return STATUS_ERROR;
	}
	
	//FLAG: ADD
	if (add) {
		dbheader->count++;
		employees = realloc(employees, dbheader->count*sizeof(struct employee_t));
		if (employees == NULL) {
			printf("realloc failed\n");
			perror("realloc");
			return STATUS_ERROR;
		}
		if (add_employee(add_string, dbheader, employees) != STATUS_SUCCESS) {
			printf("Failed to add employee.\n");
			return STATUS_ERROR;
		}
	}

	if (list) {
		if (list_employees(dbheader, employees) != STATUS_SUCCESS) {
			printf("Failed to list employees.\n");
			return STATUS_ERROR;
		}
	}

	//FLAG: STAT
	if (stat) {
		printf("dbfd = %d\n", dbfd);
		printf("newFile: %d\n", new_file);
		printf("filePath: %s\n", file_path);
	}

	//EXIT
	output_file(dbfd, dbheader, employees);
	close(dbfd);
	free(dbheader);
	free(employees);
	return STATUS_SUCCESS;
}