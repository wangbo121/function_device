/*
 * save_data.h
 *
 *  Created on: Jul 1, 2016
 *      Author: wangbo
 */

#ifndef HEADERS_SAVE_DATA_H_
#define HEADERS_SAVE_DATA_H_

int create_log_file(char *log_name);

/*
 * only for string, the end char must be \0' ,(len=0 or any one) is ok
 * 只能存储字符串也就是最后一定要有'\0'
 */
int save_data_to_log(int fd_log,char *string,int len);
int close_log_file(int fd_log);

#endif /* HEADERS_SAVE_DATA_H_ */
