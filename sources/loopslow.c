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

#include "utilityfunctions.h"

#include "save_data.h"
#include "rocket.h"

sem_t sem_loopslow;

void loopslow(void)
{
	static char rocket_air_sounding_save_data[24]={0};
	static int rocket_air_sounding_len=0;
	printf("Enter the loopslow...\n");

	while(main_task.loopslow_permission)
	{

		main_task.loopslow_cnt++;

		sem_wait(&sem_loopslow);     /*等待信号量*/

		/*
		 * 需要在慢循环执行的程序，写在这里
		 */
		printf("hello i am loopslow!!!\n");

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
		//memcpy(realtime_save_data,&ap2gcs_real,sizeof(struct AP2GCS_REAL));
		//memcpy(realtime_save_data,&ap2gcs_real,sizeof(struct AP2GCS_REAL));
		//save_data_to_log(fd_realtime_log,realtime_save_data,sizeof(struct AP2GCS_REAL));
#if 0
		realtime_len=sprintf(realtime_save_data,"%-f,%-f,%-f,%u,%u,%u,%hu,%hd,%hd,%hd,%hd,%hd,%hd,%hhu,%hhu,%hhu,%hhu,%hhu\n",\
							auto_navigation.command_heading,ctrlinput.track_heading,\
							read_encoder.postion,\
							ap2gcs_real.lng,ap2gcs_real.lat,\
							ap2gcs_real.spd,\
							ap2gcs_real.dir_gps,ap2gcs_real.dir_heading,ap2gcs_real.dir_target,ap2gcs_real.dir_nav,\
							ap2gcs_real.pitch,ap2gcs_real.roll,ap2gcs_real.yaw,\
							ap2gcs_real.moo_pwm,ap2gcs_real.mbf_pwm,ap2gcs_real.rud_pwm,\
							ap2gcs_real.rud_p,ap2gcs_real.rud_i);

		save_data_to_log(fd_realtime_log,realtime_save_data,realtime_len);
#endif
		//为什么%h这个参数编译的时候会有警告呢
		//没有%h，只有%hu，一开是写错了

#if 1
		rocket_air_sounding_len=sprintf(rocket_air_sounding_save_data,"%hd,%hu,%hu,%hu,%hu,%d,%d,%d,%hhu\n",\
				                        air_sounding_rocket.temp,\
										air_sounding_rocket.humidity,\
										air_sounding_rocket.air_pressure,\
										air_sounding_rocket.wind_speed,\
										air_sounding_rocket.wind_dir,\
										air_sounding_rocket.longitude,\
										air_sounding_rocket.latitude,\
										air_sounding_rocket.height,\
										air_sounding_rocket.number);
#endif
		save_data_to_log(fd_rocket_air_sounding_log,rocket_air_sounding_save_data,rocket_air_sounding_len);





		if(main_task.loopslow_cnt>=MAX_LOOPSLOW_TICK)
		{
			main_task.loopslow_cnt=0;
		}
	}
}
