/*
 * serial.c
 *
 *  Created on: Apr 30, 2014
 *      Author: linux
 */

#include "serial.h"

#include <stdio.h>
#include <sys/types.h> //open()
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h> //tcgetattr()...
#include <unistd.h>

typedef enum {false = 0, true = 1}bool;

int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};

/*
*open: 打开串口
*/
int serial_open(const char *pathname)
{
	int fd = -1;

	if((fd = open(pathname, O_RDWR | O_NOCTTY)) < 0) //O_RDWR | O_NOCTTY | O_NDELAY
	{
		error_inf("open error");
		return -1;
	}

	return fd;
}

/*
*open: 关闭串口
*/
int serial_close(int fd)
{
	if(close(fd) < 0)
	{
		error_inf("close error");
		return -1;
	}
	return 0;
}

/*
*set_spee: 设置串口通信速率
*fd     类型 int  打开串口的文件描述符
*speed  类型 int  串口速度(波特率)
*/
int set_speed(int fd, int speed)
{
	int i = 0;
	bool flag = false;
	struct termios opt;

	if(tcgetattr(fd, &opt) < 0)
	{
		error_inf("tcgetattr error");
		return -1;
	}

	for(i = 0; i < sizeof(speed_arr) / sizeof(int); i++)
	{
		flag = false;

		if(speed == name_arr[i])
		{
			flag = true;

			if(tcflush(fd, TCIOFLUSH) < 0)
			{
				error_inf("tcflush failure");
				return -2;
			}
			if(cfsetispeed(&opt, speed_arr[i]) < 0)
			{
				error_inf("cfsetispeed failure");
				return -2;
			}
			if(cfsetospeed(&opt, speed_arr[i]) < 0)
			{
				error_inf("cfsetospeed failure");
				return -2;
			}
			if(tcsetattr(fd, TCSANOW, &opt) != 0)
			{
				error_inf("tcsetattr failure");
				return -2;
			}

			break;
		}
	}
	if(flag == false)
	{
		debug_inf("no speed");
		return -3;
	}

	if(tcflush(fd, TCIOFLUSH) < 0)
	{
		error_inf("tcflush failure");
		return -2;
	}

	return 0;
}

/*
*set_parity: 设置串口数据位，停止位和效验位
*fd       类型  int  打开串口的文件描述符
*databits 类型  int  数据位   取值为 7 或者 8
*stopbits 类型  int  停止位   取值为 1 或者 2
*parity   类型  int  效验类型 取值为N,E,O,S
                                N: 无效验
                                E: 偶效验
                                O: 奇效验
                                S: Space效验
*/
int set_parity(int fd, int databits, int stopbits, int parity)
{
	struct termios opt;

	if(tcgetattr(fd, &opt) < 0)
	{
		error_inf("tcgetattr error");
		return -1;
	}

	opt.c_cflag &= ~CSIZE;

	/*设置数据位数*/
	switch(databits)
	{
		case 7:
			opt.c_cflag |= CS7;
			break;
		case 8:
			opt.c_cflag |= CS8;
			break;
		default:
			error_inf("no databits");
			return -1;
	}

	/*设置效验类型*/
	switch(parity)
	{
		case 'n':
		case 'N':
			opt.c_cflag &= ~PARENB;
			opt.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			opt.c_cflag |= (PARODD | PARENB);
			opt.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			opt.c_cflag |= PARENB;
			opt.c_cflag &= ~PARODD;
			opt.c_iflag |= INPCK;
			break;
		case 's':
		case 'S':
			opt.c_cflag &= ~PARENB;
			opt.c_cflag &= ~CSTOPB;
			break;
		default:
			error_inf("no parity");
			return -1;
	}

	/* 设置停止位*/
	switch(stopbits)
	{
		case 1:
			opt.c_cflag &= ~CSTOPB;
			break;
		case 2:
			opt.c_cflag |= CSTOPB;
			break;
		default:
			error_inf("no stopbits");
			return -1;
	}

	if(parity != 'n' || parity != 'N')
	{
		opt.c_iflag |= INPCK;
		opt.c_cc[VTIME] = 1;
		opt.c_cc[VMIN] = 0;
	}

	if(tcflush(fd, TCIFLUSH) < 0)
	{
		error_inf("tcflush failure");
		return -1;
	}

	/* 激活新配置 */
	if(tcsetattr(fd, TCSANOW, &opt) != 0)
	{
		error_inf("tcsetattr failure");
		return -1;
	}

	return 0;
}

/*
*serial_read: 读串口
*fd     类型  int    打开串口的文件描述符
*buf    类型  void*  读数据保存缓冲区
*count  类型  int    需要读的字节数
*return  类型  int    实际读到的字节数
*/
int serial_read(int fd, void *buf, int count)
{
	int n = -1;

	n = read(fd, buf, count);
	
	return n;
}

/*
*serial_read: 写串口
*fd     类型  int    打开串口的文件描述符
*buf    类型  void*  写数据缓冲区
*count  类型  int    需要写的字节数
*return 类型  int    实际写的字节数
*/
int serial_write(int fd, const void *buf, int count)
{
	int n = -1;

	n = write(fd, buf, count);

	return n;
}
