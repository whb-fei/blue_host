/*
 * blue_host.c
 *
 *  Created on: Apr 30, 2014
 *      Author: linux
 */

#include "blue_host.h"
#include "serial/serial.h"
#include <stdio.h>
#include <stdarg.h>  //va_start(), va_end()
#include <unistd.h> //pipe()
#include <string.h>
#include <stdlib.h> //atoi()

/*
*my_debug: 调试函数
*/
int debug_inf(const char *format, ...)
{
#ifdef DEBUG_INF
	va_list arglist;

	va_start(arglist, format);

	vprintf(format, arglist);

	va_end(arglist);
#endif
}

/*
*error_inf: 打印错误信息
*/
void error_inf(const char *str)
{
#ifdef ERROR_INF
	perror(str);
#endif
}

/*
*打印串口属性
*/
void print_serial_property(struct serial_property *sp)
{
	printf("portname : %s\n", sp->portname);
	printf("speed    : %d\n", sp->speed);
	printf("databits : %d\n", sp->databits);
	printf("stopbits : %d\n", sp->stopbits);
	printf("parity   : %d\n", sp->parity);
}

/*
*打印错误参数信息
*/
void parameter_err(char const *argv1, char const *argv2)
{
	printf("%s: invalid option -- '%s'\n", argv1, argv2);
	printf("Try '%s -?' or '%s --help' for more information.\n", argv1, argv1);
}

/*
*串口属性设置
*/
int serial_set(int fd, serial_property *sp)
{
	if(set_speed(fd, sp->speed) < 0)
	{
		debug_inf("set_speed error\n");
		return -1;
	}
	debug_inf("set_speed success\n");

	if(set_parity(fd, sp->databits, sp->stopbits, sp->parity) == -1)
	{
		debug_inf("set_parity error\n");
		return -1;
	}
	debug_inf("set_parity success\n");

	return 0;
}

/*
*创建无名管道
*/
int pipe_create(int pipefd[2])
{
	return pipe(pipefd);
}

/*
*任务选项
*/
void task_options(char *buf)
{
	char opt;

st:
	printf("Command action\n");
	printf("\ta\t蓝牙免提(HFP)\n");
	printf("\tb\t蓝牙立体声(A2DP)\n");
	printf("\tc\t蓝牙触摸控制(BTC)\n");
	printf("\td\t电话本和通话记录\n");
	printf("\te\t其他\n");
	printf("\tq\t退出\n");
	printf("\tm\t查看选项\n");
cont:
	printf("Command (m for help): ");
	if(scanf("%c", &opt) > 0)
	{
		while(getchar() != '\n');

		if(opt == 'm')
		{
			goto st;
		}
		else if(opt == 'a') //蓝牙免提(HFP)
		{
			hfp(buf);
		}
		else if(opt == 'b') //蓝牙立体声(A2DP)
		{
			a2dp(buf);
		}
		else if(opt == 'c') //蓝牙触摸控制(BTC)
		{
			btc(buf);
		}
		else if(opt == 'd') //电话本和通话记录
		{
			phone(buf);
		}
		else if(opt == 'e') //其他
		{
			other(buf);
		}
		else if(opt == 'q') //其他
		{
			return;
		}
		else if(opt == '\n') //其他
		{
			goto cont;
		}
		else
		{
			printf("Input error, please continue\n");
		}
	}
	else
	{
		perror("scanf error");
		return;
	}
}

int hfp(char *buf)
{
	printf("===> In HFP\n");
	int a;
	FILE *fd;
	int i = 0;
	char buff[128];
	cmd_inf p[CMD_NO];

	if((fd = fopen("cmd_inf.txt", "r")) == NULL)
	{
		perror("open error");
		return -1;
	}

	for(i = 0; i < CMD_NO; i++)
	{
		if(fgets(buff, 64, fd) > 0)
		{
			sscanf(buff, "%d%s%s", &p[i].id, p[i].cmd, p[i].cmd_inf);
			debug_inf("buff |%s\n", buff);
		}
	}

	fclose(fd);

	printf("Command action\n");
	printf("\tID\tCMD\t说明\n");
	printf("-----------------------------------------------------------\n");
	for(i = HFP_BASIC; i < HFP_BASIC + HFP_NO; i++)
	{
		printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
	}
	printf("\t0\t查看选项\n");

	while(1)
	{
		printf("Command ID(0 for help): ");
		scanf("%d", &a);
		while(getchar() != '\n');

		if(a == 0)
		{
			printf("Command action\n");
			printf("\tID\tCMD\t说明\n");
			printf("-----------------------------------------------------------\n");
			for(i = HFP_BASIC; i < HFP_BASIC + HFP_NO; i++)
			{
				printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
			}
			printf("\t0\t查看选项\n");
		}
		else
		{
			if(a < (HFP_BASIC + HFP_NO + 1) && a > HFP_BASIC)
			{
				memset(buf, 0, CMD_SIZE);
				strcpy(buf, p[a - 1].cmd);
				if(a == 12)
				{
					memset(buff, 0, 128);
					printf("please input number: ");
					scanf("%s", buff);
					while(getchar() != '\n');
					strncat(buf, buff, strlen(buff));
				}
				debug_inf("Leave HFP, In Home\n");
				break;
			}
			else
			{
				printf("Input error, please continue\n");
				continue;
			}

		}
	}

	return 0;
}

int a2dp(char *buf)
{
	printf("===> In A2DP\n");
	int a;
	FILE *fd;
	int i = 0;
	int n = 0;
	char buff[128];
	cmd_inf p[CMD_NO];

	if((fd = fopen("cmd_inf.txt", "r")) == NULL)
	{
		perror("open error");
		return -1;
	}

	for(i = 0; i < CMD_NO; i++)
	{
		if(fgets(buff, 64, fd) > 0)
		{
			sscanf(buff, "%d%s%s", &p[i].id, p[i].cmd, p[i].cmd_inf);
			//debug_inf("buff |%s\n", buff);
		}
	}

	fclose(fd);

	printf("Command action\n");
	printf("\tID\tCMD\t说明\n");
	printf("-----------------------------------------------------------\n");
	for(i = A2DP_BASIC; i < A2DP_BASIC + A2DP_NO; i++)
	{
		printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
	}
	printf("\t0\t查看选项\n");

	while(1)
	{
		printf("Command ID(0 for help): ");
		scanf("%d", &a);
		while(getchar() != '\n');

		if(a == 0)
		{
			printf("Command action\n");
			printf("\tID\tCMD\t说明\n");
			printf("-----------------------------------------------------------\n");
			for(i = A2DP_BASIC; i < A2DP_BASIC + A2DP_NO; i++)
			{
				printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
			}
			printf("\t0\t查看选项\n");
		}
		else
		{
			if(a < (A2DP_BASIC + A2DP_NO + 1) && a > A2DP_BASIC)
			{
				memset(buf, 0, CMD_SIZE);
				strcpy(buf, p[a - 1].cmd);
				debug_inf("Leave A2DP, In Home\n");
				break;
			}
			else
			{
				printf("Input error, please continue\n");
			}

		}
	}

	return 0;
}

int phone(char *buf)
{
	printf("===> In PHONE\n");
	int a;
	FILE *fd;
	int i = 0;
	int n = 0;
	char buff[128];
	cmd_inf p[CMD_NO];

	if((fd = fopen("cmd_inf.txt", "r")) == NULL)
	{
		perror("open error");
		return -1;
	}

	for(i = 0; i < CMD_NO; i++)
	{
		if(fgets(buff, 64, fd) > 0)
		{
			sscanf(buff, "%d%s%s", &p[i].id, p[i].cmd, p[i].cmd_inf);
			//debug_inf("buff |%s\n", buff);
		}
	}

	fclose(fd);

	printf("Command action\n");
	printf("\tID\tCMD\t说明\n");
	printf("-----------------------------------------------------------\n");
	for(i = PHONE_BASIC; i < PHONE_BASIC + PHONE_NO; i++)
	{
		printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
	}
	printf("\t0\t查看选项\n");

	while(1)
	{
		printf("Command ID(0 for help): ");
		scanf("%d", &a);
		while(getchar() != '\n');

		if(a == 0)
		{
			printf("Command action\n");
			printf("\tID\tCMD\t说明\n");
			printf("-----------------------------------------------------------\n");
			for(i = PHONE_BASIC; i < PHONE_BASIC + PHONE_NO; i++)
			{
				printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
			}
			printf("\t0\t查看选项\n");
		}
		else
		{
			if(a < (PHONE_BASIC + PHONE_NO + 1) && a > PHONE_BASIC)
			{
				memset(buf, 0, CMD_SIZE);
				strcpy(buf, p[a - 1].cmd);
				if(a == 46)
				{
				i_1:
					memset(buff, 0, 128);
					printf("please input target(m for help): ");
					scanf("%s", buff);
					while(getchar() != '\n');
					if(buff[0] == 'm')
					{
						printf("\t0\tSIM\n");
						printf("\t1\tPhone\n");
						printf("\t2\t呼出\n");
						printf("\t3\t未接\n");
						printf("\t4\t呼入\n");
						goto i_1;
					}
					else
					{
						strncat(buf, buff, 1);
						strncat(buf, ",", 1);

						memset(buff, 0, 128);
						printf("please input offset: ");
						scanf("%s", buff);
						while(getchar() != '\n');
						strncat(buf, buff, strlen(buff));
						strncat(buf, ",", 1);

						memset(buff, 0, 128);
						printf("please input count: ");
						scanf("%s", buff);
						while(getchar() != '\n');
						strncat(buf, buff, strlen(buff));
					}
					//strncat(buf, buff, strlen(buff));
				}
				debug_inf("Leave PHONE, In Home\n");
				break;
			}
			else
			{
				printf("Input error, please continue\n");
			}

		}
	}

	return 0;
}

int btc(char *buf)
{
	printf("暂未实现，请选择其他选项\n");
}

int other(char *buf)
{
	printf("===> In OTHER\n");
	int a;
	FILE *fd;
	cmd_inf p[CMD_NO];
	int i = 0;
	int n = 0;
	char s[10];
	char buff[128];

	if((fd = fopen("cmd_inf.txt", "r")) == NULL)
	{
		perror("open error");
		return -1;
	}

	for(i = 0; i < CMD_NO; i++)
	{
		if(fgets(buff, 64, fd) > 0)
		{
			sscanf(buff, "%d%s%s", &p[i].id, p[i].cmd, p[i].cmd_inf);
			//debug_inf("buff |%s\n", buff);
		}
	}

	fclose(fd);

	printf("Command action\n");
	printf("\tID\tCMD\t说明\n");
	printf("-----------------------------------------------------------\n");
	for(i = OTHER_BASIC; i < OTHER_BASIC + OTHER_NO; i++)
	{
		printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
	}
	printf("\t0\t查看选项\n");

	while(1)
	{
		printf("Command ID(0 for help): ");
		scanf("%d", &a);
		while(getchar() != '\n');

		if(a == 0)
		{
			printf("Command action\n");
			printf("\tID\tCMD\t说明\n");
			printf("-----------------------------------------------------------\n");
			for(i = OTHER_BASIC; i < OTHER_BASIC + OTHER_NO; i++)
			{
				printf("\t%d\t%s\t%s\n", p[i].id, p[i].cmd, p[i].cmd_inf);
			}
			printf("\t0\t查看选项\n");
		}
		else
		{
			if(a < (OTHER_BASIC + OTHER_NO + 1) && a > OTHER_BASIC)
			{
				memset(buf, 0, CMD_SIZE);
				strcpy(buf, p[a - 1].cmd);

				if(a == 61)
				{
					memset(buff, 0, 128);
					printf("please input Name(m = null): ");
					scanf("%s", buff);
					while(getchar() != '\n');
					if(buff[0] != 'm')
					{
						strncat(buf, buff, strlen(buff));
					}
				}
				else if(a == 62)
				{
					memset(buff, 0, 128);
					printf("please input PIN(m = null): ");
					scanf("%s", buff);
					while(getchar() != '\n');
					if(buff[0] != 'm')
					{
						strncat(buf, buff, strlen(buff));
					}
				}
				else if(a == 71)
				{
					memset(buff, 0, 128);
					printf("please input Vol(m = null): ");
					scanf("%s", buff);
					while(getchar() != '\n');
					if(buff[0] != 'm')
					{
						strncat(buf, buff, strlen(buff));
					}
				}
				debug_inf("Leave OTHER, In Home\n");
				break;
			}
			else
			{
				printf("Input error, please continue\n");
			}

		}
	}

	return 0;
}

/*
*电话呼入
*/
void phone_in(char *buf)
{
	printf("Call Me: number [%s]\n", buf + 2);
	printf("Command action\n");
	printf("\t3\t连接蓝牙免提\n"); //CC
	printf("\t4\t断开蓝牙免提\n"); //CD
	printf("\t5\t接听电话\n"); //CE
	printf("\t6\t拒绝接听\n"); //CF
	printf("\t7\t挂断电话\n"); //CG
	printf("\t10\t切换蓝牙声音\n"); //CO
}

/*
*电话呼出
*/
void phone_out(char *buf)
{
	printf("Call Out: number [%s]\n", buf + 2);

	printf("Command action\n");
	printf("\t3\t连接蓝牙免提\n"); //CC
	printf("\t4\t断开蓝牙免提\n"); //CD
	printf("\t7\t挂断电话\n"); //CG
	printf("\t10\t切换蓝牙声音\n"); //CO
}

/*
*当前通话
*/
void call(char *buf)
{
	printf("Call: number [%s]\n", buf + 2);

	printf("Command action\n");
	printf("\t3\t连接蓝牙免提\n"); //CC
	printf("\t4\t断开蓝牙免提\n"); //CD
	printf("\t7\t挂断电话\n"); //CG
	printf("\t10\t切换蓝牙声音\n"); //CO
}

/*
*同步联系人
*/
void contact(char *buf)
{
	char buff[32];
	int n = 0;
	int m = 0;

	if(0 == strncmp(buf+2, "0", 1))
		printf("SIM\t");
	else if(0 == strncmp(buf+2, "1", 1))
		printf("phone\t");
	else if(0 == strncmp(buf+2, "2", 1))
		printf("呼出\t");
	else if(0 == strncmp(buf+2, "3", 1))
		printf("未接\t");
	else if(0 == strncmp(buf+2, "4", 1))
		printf("呼入\t");

	memset(buff, 0, 32);
	strncpy(buff, buf + 3, 2);
	n = atoi(buff);
	debug_inf("n = %d\n", n);

	memset(buff, 0, 32);
	strncpy(buff, buf + 7, n);
	printf("%-16s\t", buff);

	memset(buff, 0, 32);
	strncpy(buff, buf + 5, 2);
	m = atoi(buff);
	debug_inf("m = %d\n", m);

	memset(buff, 0, 32);
	strncpy(buff, buf + 7 + n, m);
	printf("%s\n", buff);
}

