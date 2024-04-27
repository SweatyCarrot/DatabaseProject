#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "parse.h"

int create_dbheader(struct dbheader_t **dbheaderOut)
{
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed to allocate for dbheader_t.\n");
		perror("malloc");
		return STATUS_ERROR;
	}

	header->sig = FILE_SIG;
	header->version = 0x1;
	header->count = 0;
	header->filesize = sizeof(struct dbheader_t);

	*dbheaderOut = header;

	return STATUS_SUCCESS;
}

int validate_dbheader(int fd, struct dbheader_t **dbheaderOut)
{
	if (fd < 0) {
		printf("create_dbheader() got a bad file descriptor.\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed to allocate for dbheader_t.\n");
		perror("malloc");
		return STATUS_ERROR;
	}

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		printf("Bad result when reading dbheader.\n");
		free(header);
		return STATUS_ERROR;
	}

	header->sig = ntohl(header->sig);
	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->filesize = ntohl(header->filesize);

	if (header->sig != FILE_SIG) {
		printf("Improper file signature\n");
		free(header);
		return STATUS_ERROR;
	}

	// TODO: Compare with a DB_VERSION macro?
	if (header->version != 0x1) {
		printf("Improper DB file version. DB file version: %u.\n", header->version);
		free(header);
		return STATUS_ERROR;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if (header->filesize != dbstat.st_size) {
		printf("File size difference. DB is likely corrupted.\n");
		free(header);
		return STATUS_ERROR;
	}

	*dbheaderOut = header;

	return STATUS_SUCCESS;
}

void output_file(int fd, struct dbheader_t *header)
{
	if (fd < 0) {
		printf("output_file() got a bad file descriptor.\n");
		return;
	}

	header->sig = htonl(header->sig);
	header->version = htons(header->version);
	header->count = htons(header->count);
	header->filesize = htonl(header->filesize);

	if (lseek(fd, 0, SEEK_SET) != 0) {
		perror("lseek");
		return;
	}

	if (write(fd, header, sizeof(struct dbheader_t)) == -1) {
		perror("write");
		return;
	}

	return;
}