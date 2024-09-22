//TODO: Add --help flag. Improve --stat flag. 

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
	printf("Usage %s:\n", argv[0]);
	printf("\t[-f (--file) <file_path>] | Required. Specify path of database file\n");
	printf("\t[-n (--new-file)] | Create a new database file at path specified in --file\n");
	printf("\t[-l (--list)] | List employees in database\n");
	printf("\t[-a (--add) <\"{name},{address},{hours}\">] | Add a new employee\n");
	printf("\t[-u (--update) <\"{employee_num},{new_hours}\">] | Update the specified employee's hours\n");
	printf("\t[-d (--delete) <\"{employee_num}\">] | Delete the specified employee\n");
	printf("\t[-s (--stat)] | Print information about database\n");
	printf("\t[-h (--help)] | Display this message\n");
	return;
}

int main(int argc, char *argv[])
{

	//STRUCTS AND FILE DESCRIPTOR
	int dbfd = -1;
	struct dbheader_t *dbheader = NULL;
	struct employee_t *employees = NULL;
	
	//FLAGS AND COMMAND LINE ARGS
	int opt = 0;
	int opt_index = 0;

	static struct option long_options[] =
	{
		{"new-file", no_argument, 0, 'n'},
		{"file", required_argument, 0, 'f'},
		{"list", no_argument, 0, 'l'},
		{"add", required_argument, 0, 'a'},
		{"update", required_argument, 0, 'u'},
		{"delete", required_argument, 0, 'd'},
		{"stat", no_argument, 0, 's'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	
	bool new_file = false;
	bool list = false;
	bool add = false;
	bool update = false;
	bool delete = false;
	bool stat = false;

	char *file_path = NULL;
	char *add_string = NULL;
	char *update_string = NULL;
	char *delete_string = NULL;

	while ((opt = getopt_long(argc, argv, "nshld:u:a:f:", long_options, &opt_index)) != -1) {
		switch (opt) {
			case 'n':
				new_file = true;
				break;
			case 'f':
				file_path = optarg;
				break;
			case 'l':
				list = true;
				break;
			case 'a':
				add = true;
				add_string = optarg;
				break;
			case 'u':
				update = true;
				update_string = optarg;
				break;
			case 'd':
				delete = true;
				delete_string = optarg;
				break;
			case 's':
				stat = true;
				break;
			case 'h':
				printf("Help requested. Printing usage and exiting...\n");
				print_usage(argv);
				return STATUS_SUCCESS;
			default:
				print_usage(argv);
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
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
		
		if (create_dbheader(&dbheader) == STATUS_ERROR) {
			printf("Failed to create database header\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
	} else {
		dbfd = open_db_file(file_path);
		if (dbfd == -1) {
			printf("Error opening database file. If this is your first time running the program, add -n.\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
		if (validate_dbheader(dbfd, &dbheader) == STATUS_ERROR) {
			printf("Error validating dbheader\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
	}

	//Read employees from disk into struct array
	if(read_employees(dbfd, dbheader, &employees) != STATUS_SUCCESS) {
		printf("Failed to read employees!\n");
		exit_program(dbfd, dbheader, employees);
		return STATUS_ERROR;
	}
	
	//FLAG: DELETE
	if (delete) {
		dbheader->count--;
		if (delete_employee(delete_string, dbheader, employees) != STATUS_SUCCESS) {
			printf("Failed to delete employee.\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
		employees = realloc(employees, dbheader->count*sizeof(struct employee_t));
	}
	
	//FLAG: ADD
	if (add) {
		dbheader->count++;
		employees = realloc(employees, dbheader->count*sizeof(struct employee_t));
		if (employees == NULL) {
			printf("realloc failed\n");
			perror("realloc");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
		if (add_employee(add_string, dbheader, employees) != STATUS_SUCCESS) {
			printf("Failed to add employee.\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
	}

	//FLAG: UPDATE
	if (update) {
		if(update_hours(update_string, dbheader, employees) != STATUS_SUCCESS) {
			printf("Update hours failed\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
	}

	//FLAG: LIST
	if (list) {
		if (list_employees(dbheader, employees) != STATUS_SUCCESS) {
			printf("Failed to list employees.\n");
			exit_program(dbfd, dbheader, employees);
			return STATUS_ERROR;
		}
	}

	//FLAG: STAT
	if (stat) {
		//Stats for <file>
		//DB Version
		//DB Size
		//Count of employees
		printf("dbfd = %d\n", dbfd);
		printf("newFile: %d\n", new_file);
		printf("filePath: %s\n", file_path);
	}

	//EXIT
	output_file(dbfd, dbheader, employees);
	exit_program(dbfd, dbheader, employees);
	return STATUS_SUCCESS;
}