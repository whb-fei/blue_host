/*
 * serial.h
 *
 *  Created on: Apr 30, 2014
 *      Author: linux
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

/*
*open: 打开串口
*pathname  类型  char*   设备文件路径
*/
int serial_open(const char *pathname);

/*
*open: 关闭串口
*fd    类型  int  打开串口的文件描述符
*/
int serial_close(int fd);

/*
*set_spee: 设置串口通信速率
*fd     类型 int  打开串口的文件描述符
*speed  类型 int  串口速度(波特率)
*/
int set_speed(int fd, int speed);

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
int set_parity(int fd, int databits, int stopbits, int parity);


/*
*serial_read: 读串口
*fd     类型  int    打开串口的文件描述符
*buf    类型  void*  读数据保存缓冲区
*count  类型  int    需要读的字节数
*return  类型  int    实际读到的字节数
*/
int serial_read(int fd, void *buf, int count);

/*
*serial_read: 写串口
*fd     类型  int    打开串口的文件描述符
*buf    类型  void*  写数据缓冲区
*count  类型  int    需要写的字节数
*return 类型  int    实际写的字节数
*/
int serial_write(int fd, const void *buf, int count);

#endif
