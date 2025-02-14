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
 

#define MAXCARDNUM  (4)		//���֧�ֿ���Դ��
#define BIT_CPL(value,bit)  (value ^= 1<<(bit))    //ȡ��ָ��λ
#define BIT_SET(value,bit)  (value |= 1<<(bit))    //��λָ��λ
#define BIT_CLR(value,bit)  (value &= ~(1<<(bit))) //����ָ��λ
#define BIT_GET(value,bit)  ((value) & (1<<(bit)))   //��ȡָ��λ
#define BIT_VAL(value,bit)  ((value)>>(bit) & 0x01)  //��ȡָ��λ��ֵ��0||1

typedef struct {
	uint32_t head;			//ͷ
	uint16_t state;			//״̬��
	uint16_t cmdno;			//�����
	uint16_t subcmd;		//�������
	uint16_t length;		//���ݳ���
	uint8_t data[1400];		//����
}T_COMMAND;
#define CMD_HEAD  (12)		 //ͷ��Ϣ4byte����
#define DATA_LENGTH  (1024)		 //ͷ��Ϣ4byte����

enum CmdType
{
	head = 0xAAFF0055,					//����ͷ
	cmd_null = 0x0000,					//��Чָ��
	cmd_connect_device = 0xFFFF,		//�����豸  
	cmd_set_sampling_rate = 0x0001,		//���ò����ʣ�
	cmd_set_sampling_mode = 0x0002,		//���ò���ģʽ��
	cmd_start_sampling = 0x0003,		//��ʼ������
	cmd_stop_sampling = 0x0004,			//����������
	cmd_set_output = 0x0005,			//����IO�����
    cmd_get_input = 0x0006,			    //��ȡIO����״̬��
    cmd_get_version = 0x0007,			//��ȡFPGA�汾��
    cmd_set_dac = 0x0008,		        //����DAC��
    cmd_set_ip = 0x0009,                //����IP
    cmd_set_filtering_time = 0x000a,    //���������˲�ʱ��
    cmd_set_trigger_io = 0x000b,        //���ô���IO
    cmd_set_sampling_resistor = 0x000c, //���ò�������

	cmd_end
};

enum ErrType
{
	err_none = 0x0000,			//�ɹ�
	err_parameter = 0x0001,		//��������
	err_sem_mutex = 0x0002,		//����������ʧ��
	err_link_fail = 0x0003,		//�������Ӵ���

	err_file_fail = 0x0009,		//�ļ���д����
	err_status_thread = 0x000a, //״̬�̴߳���ʧ��
	err_multi_init = 0x000b,	//�����DLL��ص���
	err_udp_send = 0x000c,		//socket���ʹ���
	err_data_length = 0x000d,	//socket�������ݳ������
	err_udp_recv = 0x000e,		//socket���մ���
	err_udp_timeout = 0x000f,	//ͨ����Ӧ��ʱ

	err_cmd_timeout = 0x0010,	//ָ����Ӧ��ʱ
	err_data_read = 0x0011,		//û�ж�ȡ������������
	err_create_thread = 0x0012,	//���������̴߳�������
    err_write_data = 0x0013,	//д��ѭ��������ʧ��

	err_cmd_null = 0x0101,		//��Чָ��
	err_parameter_cmd = 0x0102, //�����쳣
	err_para_protect = 0x0103,	//�����������޸�
	err_data_size = 0x0104,		//ͨ���������쳣

};

//�����շ�
short SetCommand(MSocket& mSkt, T_COMMAND& tCmd, void* sbuf, void* rbuf);
//�����
short SendCommand(MSocket& mSkt, T_COMMAND& tCmd, void* sbuf, void* rbuf);
//������������
short ReceiveData(MSocket& mSkt, T_COMMAND& tCmd, void* rbuf);