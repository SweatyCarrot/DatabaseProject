#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "file.h"
#include "common.h"

int create_db_file(char *file_name)
{
	int fd = open(file_name, O_RDONLY);
	if (fd != -1) {
		close(fd);
		printf("DB file already exists. Did you accidentally specify new_file?\n");
		return STATUS_ERROR;
	}

	fd = open(file_name, O_RDWR | O_CREAT, 0664);
	if (fd == -1) {
		perror("open");
		return STATUS_ERROR;
	}

	return fd;
}

int open_db_file(char *file_name)
{
	int fd = open(file_name, O_RDWR);
	if (fd == -1) {
		perror("open");
		return STATUS_ERROR;
	}

	return fd;
}