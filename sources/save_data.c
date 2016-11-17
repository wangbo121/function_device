/*
 * save_data.c
 *
 *  Created on: Jul 1, 2016
 *      Author: wangbo
 */


#include <stdio.h>
#include <stdlib.h>/*exit(1)*/
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#include "save_data.h"

int create_log_file(char *log_name)
{
	int fd;
	char convert_to_string[200];

	time_t ptrtime;
	struct tm *time_of_file;

	time(&ptrtime);
	time_of_file=localtime(&ptrtime);

	sprintf(convert_to_string,"%d-%d-%d-%d-%d-%d",time_of_file->tm_year+1900,time_of_file->tm_mon+1,time_of_file->tm_mday,\
			                            time_of_file->tm_hour,time_of_file->tm_min,time_of_file->tm_sec);
	strcat(convert_to_string,log_name);

	if(-1==(fd=open(convert_to_string,O_RDWR |O_CREAT |O_APPEND)))
	{
		printf("Error:Can not open %s",convert_to_string);
		exit(1);
	}

	return fd;
}

int save_data_to_log(int fd_log,char *string,int len)
{
	int write_len;
	int i=0;

	for(i=0;string[i]!='\0';i++)
		;
	len=i;

	if(fd_log)
	{
		if(-1==(write_len=write(fd_log,string,len)))
		{
		    printf("write error!\n");
		}
	}

	return write_len;
}

int close_log_file(int fd_log)
{
	if(fd_log)
	{
		close(fd_log);
	}

	return 0;
}
