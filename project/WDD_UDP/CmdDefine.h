#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tchar.h>
#include <string.h> 
#include <process.h>
#include <windows.h>
#include ".\Socket\MSocket.h"
#include ".\ringbuffer\ringbuffer.h"
 

#define MAXCARDNUM  (4)		//最大支持卡资源数
#define BIT_CPL(value,bit)  (value ^= 1<<(bit))    //取反指定位
#define BIT_SET(value,bit)  (value |= 1<<(bit))    //置位指定位
#define BIT_CLR(value,bit)  (value &= ~(1<<(bit))) //清零指定位
#define BIT_GET(value,bit)  ((value) & (1<<(bit)))   //读取指定位
#define BIT_VAL(value,bit)  ((value)>>(bit) & 0x01)  //读取指定位的值，0||1

typedef struct {
	uint32_t head;			//头
	uint16_t state;			//状态码
	uint16_t cmdno;			//命令号
	uint16_t subcmd;		//子命令号
	uint16_t length;		//数据长度
	uint8_t data[1400];		//数据
}T_COMMAND;
#define CMD_HEAD  (12)		 //头信息4byte对齐
#define DATA_LENGTH  (1024)		 //头信息4byte对齐

enum CmdType
{
	head = 0xAAFF0055,					//数据头
	cmd_null = 0x0000,					//无效指令
	cmd_connect_device = 0xFFFF,		//连接设备  
	cmd_set_sampling_rate = 0x0001,		//设置采样率；
	cmd_set_sampling_mode = 0x0002,		//设置采样模式；
	cmd_start_sampling = 0x0003,		//开始采样；
	cmd_stop_sampling = 0x0004,			//结束采样；
	cmd_set_output = 0x0005,			//设置IO输出；
    cmd_get_input = 0x0006,			    //获取IO输入状态；
    cmd_get_version = 0x0007,			//获取FPGA版本；
    cmd_set_dac = 0x0008,		        //设置DAC；
    cmd_set_ip = 0x0009,                //设置IP
    cmd_set_filtering_time = 0x000a,    //设置输入滤波时间
    cmd_set_trigger_io = 0x000b,        //设置触发IO
    cmd_set_sampling_resistor = 0x000c, //设置采样电阻

	cmd_end
};

enum ErrType
{
	err_none = 0x0000,			//成功
	err_parameter = 0x0001,		//参数错误
	err_sem_mutex = 0x0002,		//互斥量保护失败
	err_link_fail = 0x0003,		//网络连接错误

	err_file_fail = 0x0009,		//文件读写错误
	err_status_thread = 0x000a, //状态线程创建失败
	err_multi_init = 0x000b,	//多进程DLL异地调用
	err_udp_send = 0x000c,		//socket发送错误
	err_data_length = 0x000d,	//socket发送数据长度溢出
	err_udp_recv = 0x000e,		//socket接收错误
	err_udp_timeout = 0x000f,	//通信响应超时

	err_cmd_timeout = 0x0010,	//指令响应超时
	err_data_read = 0x0011,		//没有读取到缓冲区数据
	err_create_thread = 0x0012,	//码流接收线程创建错误
    err_write_data = 0x0013,	//写入循环缓冲区失败

	err_cmd_null = 0x0101,		//无效指令
	err_parameter_cmd = 0x0102, //参数异常
	err_para_protect = 0x0103,	//参数不允许修改
	err_data_size = 0x0104,		//通信数据量异常

};

//命令收发
short SetCommand(MSocket& mSkt, T_COMMAND& tCmd, void* sbuf, void* rbuf);
//命令发送
short SendCommand(MSocket& mSkt, T_COMMAND& tCmd, void* sbuf, void* rbuf);
//接收码流数据
short ReceiveData(MSocket& mSkt, T_COMMAND& tCmd, void* rbuf);