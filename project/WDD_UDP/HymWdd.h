// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef _HYMSON_WDD_H_
#define _HYMSON_WDD_H_

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t Mode;		//触发模式
	uint32_t Rate;		//采样率，Hz
	uint32_t SampTime;	//时间采样模式下，采样时间
	uint32_t SampNum;	//采样点数
	uint32_t Status;	//采样状态，0:未开启; 1:开启
}SampPara;
extern SampPara tSampPrm;

/********************************************************************************************************************************************************************
																		API
********************************************************************************************************************************************************************/
// LIB库版本号  [24082201]十六进制，年-月-日-版本
short _stdcall gmc_get_lib_version(uint32_t *version);
// FPGA库版本号  [24082201]十六进制，年-月-日-版本
short _stdcall gmc_get_fpga_version(uint32_t *fpga);

// 连接设备
short _stdcall ConnectedDevice();
// 关闭设备
short _stdcall CloseDevice();
// 配置设备，设置采样率，采样模式，采样时间
// mode[0:连续采样, 1:时间采样]，连续采样停止采集才停止，时间采样模式下，time才生效
// rate：采样率 单位Hz; time: ms
short _stdcall ConfigureADCParameters(int mode, float rate, float time);
// 设置DAC1, DAC2, DAC3, DAC4; 四个通道的DA值;0-4096对应0-2.48V
short _stdcall SetDACParameters(int DA[4]);
// 开始采集
short _stdcall StartADCCollection();
// 停止采集
short _stdcall StopADCCollection();
// 读取缓冲区数据
short _stdcall TryReadADCData(unsigned char *read_buffer, uint32_t read_size); //默认1024
// 设置IO输出, IO按位输出函数index[0,15] value[0关闭,1打开] 
short _stdcall SetIoOutput(int index, int value);
// 获取IO输入状态
short _stdcall GetIoInput(int index, uint32_t *value);
// 设置输入滤波时间,单位40ns
short _stdcall SetFilterTime(int time);
// 设置触发IO，index[0,7] enable[0关闭,1打开]
short _stdcall SetTriggerIo(int index, uint8_t enable);
// 设置采样电阻，Resistor[3],三路电阻值[1-8]
short _stdcall SetSampResistor(uint8_t Resistor[3]);

		
#ifdef __cplusplus
}
#endif

#endif //_HYMSON_WDD_H_
