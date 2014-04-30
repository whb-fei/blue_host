/*
 * main.c
 *
 *  Created on: Apr 30, 2014
 *      Author: linux
 */

#include "blue_host.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

void mx(const char *buf);

int main(int argc, char const *argv[])
{
	int fd = -1;
	FILE *fs;
	cmd_inf p[CMD_NO];
	char rbuf[SIZE] = {'\0'}; //read buf
	char wbuf[SIZE] = {'\0'}; //write buf
	char buf[SIZE] = {'\0'}; //write buf
	char cmdbuf[CMD_SIZE] = {"\0"};
	int cmdlean = 1;
	int rcount = -1;
	int pipe_fd[2];
	pid_t pid;
	int n;
	int cmd_no = -1;
	struct serial_property *sp;
	fd_set fds;
	struct timeval tv = {0, 100};

	sp = malloc(sizeof(struct serial_property));

	if(argc == 1)
	{
		strcpy(sp->portname, "/dev/ttyUSB0");
		sp->speed    = 115200;
		sp->databits = 8;
		sp->stopbits = 1;
		sp->parity   = 'n';
	}
	else if(argc == 2)
	{
		if((strcmp(argv[1], "-v")) == 0 || \
			(strcmp(argv[1], "--version")) == 0)
		{

			printf("version: 1.0.0\n");
			return 0;
		}
		else if((strcmp(argv[1], "-?")) == 0 || \
			(strcmp(argv[1], "--help")) == 0)
		{
			if((fd = open("blue_host_help.txt", O_RDONLY)) < 0)
			{
				perror("open error");
				return -1;
			}

			while(read(fd, rbuf, SIZE) > 0)
			{
				printf("%s", rbuf);
				memset(rbuf, 0, SIZE);
			}
			printf("\n");

			close(fd);

			return 0;
		}
		else
		{
			parameter_err(argv[0], argv[1]);

			return 0;
		}
	}
	else if(argc == 7)
	{
		if((strcmp(argv[1], "-l")) == 0 || \
			(strcmp(argv[1], "--list")) == 0)
		{
			sprintf(sp->portname, "/dev/tty%s", argv[2]);
			sp->speed    = atoi(argv[3]);
			sp->databits = atoi(argv[4]);
			sp->stopbits = atoi(argv[5]);
			sp->parity   = *argv[6];
		}
		else
		{
			parameter_err(argv[0], argv[1]);

			return 0;
		}
	}
	else
	{
		printf("%s: invalid option !\n", argv[0]);
		printf("Try '%s -?' or '%s --help' for more information.\n", argv[0], argv[0]);

		return 0;
	}
	print_serial_property(sp);


	if((fd = serial_open(sp->portname)) == -1)
	{
		debug_inf("serial_open error\n");
		return -1;
	}
	debug_inf("serial_open success\n");

	//串口属性设置
	if(serial_set(fd, sp) < 0)
	{
		debug_inf("serial_set error");
		goto err;
	}
	debug_inf("serial_set success\n");

	printf("Connecting to %s, speed %d\n\n", sp->portname, sp->speed);
	printf("Bluetooth Host controller\n\n");

	//创建无名管道
	if(pipe_create(pipe_fd) < 0)
	{
		perror("pipe error");
		return -1;
	}

	if((pid = fork()) < 0)
	{
		perror("fork error");
		goto err;
	}
	else if(pid == 0) //child process
	{
		close(pipe_fd[1]); // close write pipe

		if((fs = fopen("cmd_inf.txt", "r")) == NULL)
		{
			perror("open error");
			return -1;
		}

		for(n = 0; n < CMD_NO; n++)
		{
			memset(buf, 0 , SIZE);
			if(fgets(buf, 64, fs) > 0)
			{
				sscanf(buf, "%d%s%s", &p[n].id, p[n].cmd, p[n].cmd_inf);
				//debug_inf("buf |%s\n", buf);
			}
		}

		fclose(fs);

		printf("提示: 输入[0]进入键盘模式\n");

		while(1)
		{

			memset(wbuf, 0, SIZE);
			wbuf[0] = 'A';
			wbuf[1] = 'T';
			wbuf[2] = '#';

			memset(cmdbuf, 0, CMD_SIZE);
			memset(buf, 0, CMD_SIZE);

			FD_ZERO(&fds);
			FD_SET(pipe_fd[0], &fds);
			FD_SET(0, &fds);

			if((n = select(pipe_fd[0] + 1, &fds, NULL, NULL, &tv)) < 0)
			{
				perror("select error");
				return -1;
			}
			else if(n > 0)
			{
				cmd_no = -1;
				if(FD_ISSET(pipe_fd[0], &fds))
				{
					memset(buf, 0, SIZE);
					if(read(pipe_fd[0], buf, SIZE) < 0)
					{
						perror("read error");
						return -1;
					}

					if(0 == strncmp(buf, "MX", 2))
					{
						debug_inf("提示: MX测试\n");
						mx(buf);
					}

					if(0 == strncmp(buf, "IC", 2))
					{
						debug_inf("提示: 有电话呼出\n");
						phone_out(buf);
					}

					if(0 == strncmp(buf, "ID", 2))
					{
						debug_inf("提示: 有电话呼入\n");
						phone_in(buf);
					}

					if(0 == strncmp(buf, "IR", 2))
					{
						debug_inf("提示: 当前通话\n");
						call(buf);
					}

					if(0 == strncmp(buf, "PB", 2))
					{
						debug_inf("提示: 当前通话\n");
						contact(buf);
					}

					if(0 == strncmp(buf, "PC", 2))
					{
						printf("提示: 同步联系人完成\n");
					}
				}
				else if(FD_ISSET(0, &fds))
				{
					if(scanf("%d", &cmd_no) < 0)
					{
						perror("scanf error");
						return -1;
					}

					if(0 == cmd_no)
					{
						printf("========= 进入键盘模式 =========\n");
						while(getchar() != '\n');
						task_options(cmdbuf);
						debug_inf("|| cmdbuf: %s\n", cmdbuf);
					}
					else if(cmd_no < CMD_NO + 1 && cmd_no > 0)
					{
						strncpy(cmdbuf, p[cmd_no - 1].cmd, 2);
						if(cmd_no == 12)
						{
							printf("please input number: ");
							memset(buf, 0, SIZE);
							scanf("%s", buf);
							while(getchar() != '\n');
							strncat(cmdbuf, buf, strlen(buf));
						}
						else if(cmd_no == 46)
						{
							memset(buf, 0, SIZE);
							printf("please input target: ");
							scanf("%s", buf);
							while(getchar() != '\n');

							strncat(cmdbuf, buf, 1);
							strncat(cmdbuf, ",", 1);

							memset(buf, 0, SIZE);
							printf("please input offset: ");
							scanf("%s", buf);
							while(getchar() != '\n');
							strncat(cmdbuf, buf, strlen(buf));
							strncat(cmdbuf, ",", 1);

							memset(buf, 0, SIZE);
							printf("please input count: ");
							scanf("%s", buf);
							while(getchar() != '\n');
							strncat(cmdbuf, buf, strlen(buf));
						}
						if(cmd_no == 61)
						{
							memset(buf, 0, 128);
							printf("please input Name(m = null): ");
							scanf("%s", buf);
							while(getchar() != '\n');
							if(buf[0] != 'm')
							{
								strncat(cmdbuf, buf, strlen(buf));
							}
						}
						else if(cmd_no == 62)
						{
							memset(buf, 0, 128);
							printf("please input PIN(m = null): ");
							scanf("%s", buf);
							while(getchar() != '\n');
							if(buf[0] != 'm')
							{
								strncat(cmdbuf, buf, strlen(buf));
							}
						}
						else if(cmd_no == 71)
						{
							memset(buf, 0, 128);
							printf("please input Vol(m = null): ");
							scanf("%s", buf);
							while(getchar() != '\n');
							if(buf[0] != 'm')
							{
								strncat(cmdbuf, buf, strlen(buf));
							}
						}
					}
					else
					{
						printf("Input error, please continue\n");
						continue;
					}
				}
			}

			if(strlen(cmdbuf) == 0)
			{
				continue;
			}
			strcat(wbuf, cmdbuf);
			cmdlean = strlen(wbuf);
			wbuf[cmdlean] = '\r';
			wbuf[cmdlean + 1] = '\n';
			//printf("CMD: %s\n", wbuf);

			if(serial_write(fd, wbuf, cmdlean + 2) < 0)
			{
				perror("serial_write error");
				return -1;
			}
			debug_inf("serial_write OK\n");
		}
	}
	else //parent process
	{
		close(pipe_fd[0]); // close read pipe

		while(1)
		{

			memset(rbuf, 0, SIZE);
			if((rcount = serial_read(fd, rbuf, SIZE)) < 0)
			{
				perror("serial_read error");
				return -1;
			}
			else if(rcount == 1 && rbuf[0] == '\n') //rcount = 1: rbuf = '\n'
			{
				continue;
			}

			rbuf[rcount -1] = '\0'; //去掉换行符
			debug_inf("===>rcount = %d\n", rcount);
			debug_inf("===>%s\n", rbuf);

			if(strncmp(rbuf, "MX", 2) == 0) //测试
			{
				if(write(pipe_fd[1], rbuf, strlen(rbuf)) < 0)
				{
					perror("write error");
					return -1;
				}
				usleep(200);
			}

			if(strncmp(rbuf, "IA", 2) == 0) //蓝牙免提断开
			{
				printf("蓝牙免提断开\n");
			}

			if(strncmp(rbuf, "IB", 2) == 0) //蓝牙免提接入成功
			{
				printf("蓝牙免提接入成功\n");
			}

			if(strncmp(rbuf, "IC", 2) == 0) //有电话呼出
			{
				if(write(pipe_fd[1], rbuf, strlen(rbuf)) < 0)
				{
					perror("write error");
					return -1;
				}
				usleep(200);
			}

			if(strncmp(rbuf, "ID", 2) == 0) //有电话呼入
			{
				if(write(pipe_fd[1], rbuf, strlen(rbuf)) < 0)
				{
					perror("write error");
					return -1;
				}
				usleep(200);
			}

			if(strncmp(rbuf, "IR", 2) == 0) //当前通话
			{
				if(write(pipe_fd[1], rbuf, strlen(rbuf)) < 0)
				{
					perror("write error");
					return -1;
				}
				usleep(200);
			}

			if(strncmp(rbuf, "PB", 2) == 0) //同步联系人
			{
				if(write(pipe_fd[1], rbuf, strlen(rbuf)) < 0)
				{
					perror("write error");
					return -1;
				}
				usleep(200);
			}
			if(strncmp(rbuf, "PC", 2) == 0) //同步联系人完成
			{
				if(write(pipe_fd[1], rbuf, strlen(rbuf)) < 0)
				{
					perror("write error");
					return -1;
				}
				usleep(200);
			}
		}
	}

err:
	if(serial_close(fd) < 0)
	{
		debug_inf("serial_close error\n");
		return -1;
	}
	debug_inf("serial_close success\n");

//#endif

	return 0;
}

void mx(const char *buf)
{
	char cmd[3] = {'\0'};

	debug_inf("=== Come From MX\n");

	strncpy(cmd, buf, 2);

	printf("CMD = %s || INF = %s\n", cmd, buf + 2);
}
