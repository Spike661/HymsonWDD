#pragma once

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;


short _stdcall gmc_connect_cmd();						//������������
short _stdcall gmc_set_sampling_rate(int rate);			//���ò�����
short _stdcall gmc_sampling_mode_continuous();			//��������ģʽ
short _stdcall gmc_sampling_mode_time(float time);		//ʱ�����ģʽ