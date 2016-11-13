/*
 * loopfast.c
 *
 *  Created on: 2016年5月9日
 *      Author: wangbo
 */

#include <stdio.h>
#include <semaphore.h>

#include "maintask.h"
#include "loopfast.h"
#include "rocket.h"
#include "global.h"

#include "modbus_power_supply.h"
#include "modbus_toggle_discharge.h"

sem_t sem_loopfast;

void loopfast(void)
{
	printf("Enter the loopfast...\n");

	while(main_task.loopfast_permission)
	{
		sem_wait(&sem_loopfast);     /*等待信号量*/

		/*
		 * 需要在快循环执行的程序，写在这里
		 */

        /*切换器开始切换电池放电*/
        toggle_discharge_loop(&query_toggle,&set_toggle,&read_toggle);

        /*
         * 放电周期+从130放到100伏特时间=2*(充电周期)+从100充电到130伏时间+从10伏充到12伏特以上的时间
         * 充电时间一般来说是固定的，因为充电的时候一定要保证不再放电没有负荷
         * 而放电速度就不一定了，如果我们把期望速度调低点，放电就慢了
         */

        if((main_task.loopfast_cnt)%5==0)
        {
            //power_charge_loop(&query_power,&set_power,&read_power);
        }

		if(global_bool.bool_rocket_request_attitude)
		{
		    //write_rocket_attitude();
		}

		main_task.loopfast_cnt++;

		if(main_task.loopfast_cnt>=MAX_LOOPFAST_TICK)
		{
			main_task.loopfast_cnt=0;
		}
	}
}
