#pragma once

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;


short _stdcall gmc_connect_cmd();						//发送连接命令
short _stdcall gmc_set_sampling_rate(int rate);			//设置采样率
short _stdcall gmc_sampling_mode_continuous();			//连续采样模式
short _stdcall gmc_sampling_mode_time(float time);		//时间采样模式