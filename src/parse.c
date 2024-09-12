#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int read_employees(int fd, struct dbheader_t *dbheader, struct employee_t **employeesOut)
{
	if (fd < 0) {
		printf("output_file() got a bad file descriptor.\n");
		return STATUS_ERROR;
	}

	int count = dbheader->count;

	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if (employees == NULL) {
		printf("Malloc failed to allocate for employees.\n");
		perror("malloc");
		return STATUS_ERROR;
	}

	read(fd, employees, count*sizeof(struct employee_t));
	int i;
	for (i = 0; i < count; ++i) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;
}

int add_employee(char *add_string, struct dbheader_t *dbheader, struct employee_t *employees)
{
	
	char *name = strtok(add_string, ",");
	char *address = strtok(NULL, ",");
	char *hours = strtok(NULL, ",");
	strncpy(employees[dbheader->count-1].name, name, sizeof(employees[dbheader->count-1].name));
	strncpy(employees[dbheader->count-1].address, address, sizeof(employees[dbheader->count-1].address));
	//TODO: Error handle atoi()?
	employees[dbheader->count-1].hours = atoi(hours);
	
	printf("Added employee\n");
	return STATUS_SUCCESS;
}


int list_employees(struct dbheader_t *dbheader, struct employee_t *employees)
{
	if (dbheader->count == 0) {
		printf("No employees to list!\n");
		return STATUS_ERROR;
	}

	int i;
	for (i = 0; i < dbheader->count; i++) {
		printf("Employee %d\n", i+1);
		printf("\tName: %s\n",employees[i].name);
		printf("\tAddress: %s\n", employees[i].address);
		printf("\tHours: %d\n", employees[i].hours);

		// Legacy display: printf("Employee %d - Name: %s\t| Address: %s\t| Hours: %d\n", i+1, employees[i].name, employees[i].address, employees[i].hours);
	}
	return STATUS_SUCCESS;
}

int update_hours(char *update_string, struct employee_t *employees)
{
	//TODO: Handle errors from atoi()?
	int empnum = atoi(strtok(update_string, ","));
	int hours = atoi(strtok(NULL, ","));
	
	printf("Employee: %d | Hours: %d\n", empnum, hours);
	employees[empnum-1].hours = hours;

	return STATUS_SUCCESS;
}

void output_file(int fd, struct dbheader_t *header, struct employee_t *employees)
{
	if (fd < 0) {
		printf("output_file() got a bad file descriptor.\n");
		return;
	}

	int real_count = header->count;

	header->sig = htonl(header->sig);
	header->version = htons(header->version);
	header->count = htons(header->count);
	header->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * real_count));


	if (lseek(fd, 0, SEEK_SET) != 0) {
		perror("lseek");
		return;
	}

	//WRITE HEADER
	if (write(fd, header, sizeof(struct dbheader_t)) == -1) {
		perror("write");
		return;
	}

	//WRITE EMPLOYEES
	if (employees == NULL) {
		printf("No employees read. Exiting.\n");
		return;
	}

	int i;
	for(i = 0; i < real_count; ++i) {
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

	return;
}