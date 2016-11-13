/*
 * loopslow.c
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "maintask.h"
#include "loopslow.h"
#include "global.h"
#include "save_data.h"
#include "utilityfunctions.h"

#include "rocket.h"
#include "weather_station.h"
#include "udp.h"

sem_t sem_loopslow;

void loopslow(void)
{
	static char rocket_air_sounding_save_data[24]={0};
	static int rocket_air_sounding_len=0;
	static char air_weather_station_save_data[256]={0};//256随意设置的，大于需要保存的数据即可
	static int air_weather_station_len=0;

	printf("Enter the loopslow...\n");

	while(main_task.loopslow_permission)
	{

		main_task.loopslow_cnt++;

		sem_wait(&sem_loopslow);     /*等待信号量*/

		//通过udp把气象站和火箭的数据发送给主控
		char test[]={"wangbo"};
		send_udp_data((unsigned char *)test, sizeof(test));

		/*
		 * 需要在慢循环执行的程序，写在这里
		 */
		//printf("hello i am loopslow!!!\n");

		/*保存实时数据*/
		/*
		 * C语言各种数据类型的占位符：
		 * char -- %c或%hhd %c采用字符身份，%hhd采用数字身份；
		 * unsigned char -- %c或%hhu
		 * short -- %hd
		 * unsigned short -- %hu
		 * long -- %ld
		 * unsigned long -- %lu
		 * int -- %d
		 * unsigned int -- %u
		 * float -- %f或%g %f会保留小数点后面无效的0，%g则不会；
		 * double -- %lf或%lg
		 */

		/*保存火箭数据*/
		rocket_air_sounding_len=sprintf(rocket_air_sounding_save_data,"%hd,%hu,%hu,%hu,%hu,%d,%d,%d,%hhu\n",\
				                        rocket_air_sounding.temp,\
										rocket_air_sounding.humidity,\
										rocket_air_sounding.air_pressure,\
										rocket_air_sounding.wind_speed,\
										rocket_air_sounding.wind_dir,\
										rocket_air_sounding.longitude,\
										rocket_air_sounding.latitude,\
										rocket_air_sounding.height,\
										rocket_air_sounding.number);
		save_data_to_log(fd_rocket_air_sounding_log,rocket_air_sounding_save_data,rocket_air_sounding_len);

		/*保存气象站数据*/
		air_weather_station_len=sprintf(air_weather_station_save_data,\
                                        "%hu,%hhu,%hhu,"
                                        "%hhu,%hhu,%hhu,"
                                        "%c,%u,"
                                        "%c,%u,"
                                        "%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu,"
                                        "%u,%u,"
                                        "%hu,%hu,%hu,%hu,%hu\n",\
                                        read_aws.year,read_aws.month,read_aws.day,\
                                        read_aws.hour,read_aws.minute,read_aws.second,\
                                        read_aws.east_west,read_aws.longitude,\
                                        read_aws.north_south,read_aws.latitude,\
                                        read_aws.height,read_aws.temperature,read_aws.humidity,read_aws.wind_speed,read_aws.wind_dir,read_aws.air_press,read_aws.radiation1,read_aws.radiation2,\
                                        read_aws.salt_sea_temp,read_aws.conductivity,\
                                        read_aws.sea_temp1,read_aws.sea_temp2,read_aws.sea_temp3,read_aws.sea_temp4,read_aws.sea_temp5);
		save_data_to_log(fd_air_weather_station_log,air_weather_station_save_data,air_weather_station_len);

		if(main_task.loopslow_cnt>=MAX_LOOPSLOW_TICK)
		{
			main_task.loopslow_cnt=0;
		}
	}
}
