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
	
	int opt = 0;
	bool new_file = false;
	char *file_path = NULL;
	int dbfd = -1;
	struct dbheader_t *dbheader = NULL;

	while ((opt = getopt(argc, argv, "nf:")) != -1) {
		switch (opt) {
			case 'n':
				new_file = true;
				break;
			case 'f':
				file_path = optarg;
				break;
			default:
				return STATUS_ERROR;
		}
	}

	if (file_path == NULL) {
		print_usage(argv);
		return STATUS_ERROR;
	}

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
		output_file(dbfd, dbheader);

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
	
	// Placeholder functionality
	printf("dbfd = %d\n", dbfd);
	close(dbfd);
	free(dbheader);
	printf("newFile: %d\n", new_file);
	printf("filePath: %s\n", file_path);

	return 0;
}