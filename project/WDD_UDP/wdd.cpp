// dllmain.cpp : 定义 DLL 应用程序的入口点。
//#include "pch.h"
#include "HymWdd.h"
#include "CmdDefine.h"
#include "VersionNo.h"
#include "inside.h"

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//#pragma data_seg("HYMMCShared")
int SktConnectNum[MAXCARDNUM] = { 0 };       //socket
int DllInited = 0;
//#pragma data_seg()
//#pragma comment(linker,"/SECTION:HYMMCShared,RWS")
///////////////////////////////////////////////////////////////////////////////
MSocket stMSkt[MAXCARDNUM] = { MSocket() };//soctet通信
HANDLE hmtx[MAXCARDNUM] = { NULL };
HANDLE DllHandMtx = NULL;
///////////////////////////////////////////////////////////////////////////////
HANDLE hRecvThread = NULL;  // 接收数据线程的句柄
DWORD WINAPI ReceiveDataThread(LPVOID lpParam);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HANDLE hProcessingThread = NULL;  // 接收数据线程的句柄
DWORD WINAPI ProcessingDataThread(LPVOID lpParam);
///////////////////////////////////////////////////////////////////////////////
SampPara tSampPrm = { 0 };
/********************************************************************************************************************************************************************
															 循环缓冲区
********************************************************************************************************************************************************************/
#define BUFFER_SIZE  (3*1024*1000)  // 定义缓冲区大小3M，根据实际需求调整

typedef struct {
	char data[BUFFER_SIZE];
	int head;
	int tail;
	int count;
	CRITICAL_SECTION cs;  // 用于同步的临界区
} CircularBuffer;

CircularBuffer circBuffer;  // 全局循环缓冲区-UDP接收到的原始数据
CircularBuffer dataBuffer;  // 全局循环缓冲区-校验处理后的数据

// 初始化缓冲区
void InitBuffer(CircularBuffer* cb) {
	cb->head = 0;
	cb->tail = 0;
	cb->count = 0;
	InitializeCriticalSection(&cb->cs);  // 初始化临界区
}

// 在开始采集前调用此函数
void InitializeSystem() {
	InitBuffer(&circBuffer);  // 初始化循环缓冲区
    InitBuffer(&dataBuffer);  // 初始化循环缓冲区
	// 其他初始化操作
}

// 将数据写入缓冲区
bool WriteToBuffer(CircularBuffer* cb, const unsigned char* data, int length) {
    // 输入参数检查
    if (cb == nullptr || data == nullptr || length < 0) {
        return false; // 参数无效
    }

    EnterCriticalSection(&cb->cs);  // 进入临界区

    for (int i = 0; i < length; i++) {
        // 写入数据并覆盖旧数据
        cb->data[cb->tail] = data[i];

        // 更新尾指针
        cb->tail = (cb->tail + 1) % BUFFER_SIZE;

        // 如果缓冲区已满，更新头指针，覆盖旧数据
        if (cb->count == BUFFER_SIZE) {
            cb->head = (cb->head + 1) % BUFFER_SIZE;  // 头指针向前移动，覆盖旧数据
        }
        else {
            cb->count++; // 增加计数，缓冲区未满时
        }
    }

    LeaveCriticalSection(&cb->cs);  // 离开临界区
    return true;
}


// 从缓冲区读取数据
bool ReadFromBuffer(CircularBuffer* cb, char* data, int length) {
	if (length > cb->count) {
		// 缓冲区数据不足
		return false;
	}

	EnterCriticalSection(&cb->cs);  // 进入临界区

	for (int i = 0; i < length; i++) {
		data[i] = cb->data[cb->head];
		cb->head = (cb->head + 1) % BUFFER_SIZE;
	}
	cb->count -= length;

	LeaveCriticalSection(&cb->cs);  // 离开临界区
	return true;
}

/********************************************************************************************************************************************************************
															  数据校验
********************************************************************************************************************************************************************/
#define DA_DATA_SIZE 8
#define CHECKSUM_SIZE 2
#define TOTAL_CHANNELS 4
#define SAMPLE_POINTS 128

// 校验函数，对数据做和校验
bool CheckSum(const unsigned char* data, size_t length, unsigned short expected_checksum) {
    unsigned int sum = 0; 
    // 每两个字节相加
    for (size_t i = 0; i < length; i += 2) {
        unsigned short pair_sum;

        // 检查是否还有一个字节可以配对
        if (i + 1 < length) {
            pair_sum = (data[i] | (data[i + 1] << 8)); // 直接构建 pair_sum
        }
        else {
            pair_sum = data[i]; // 处理单个字节的情况
        }
        sum += pair_sum; // 累加
    }
    // 取低16位
    sum = (sum & 0xFFFF) + (sum >> 16); // 处理可能的溢出
    return (sum == expected_checksum);
}

// 处理接收到的数据包
void ProcessDataPacket(const unsigned char* buffer, unsigned char* revbuffer, size_t length) {
    const size_t expected_length = 1280;
    if (length != expected_length) {
        DEBUG("Invalid data length %zu, expected 1280 bytes.\n", length);
        return;
    }
    unsigned char last_data[DA_DATA_SIZE] = { 0 };
	// 每次处理 8 字节 DA 数据 + 2 字节校验位
    for (int i = 0; i < SAMPLE_POINTS; ++i) {
        size_t data_start_index = i * (DA_DATA_SIZE + CHECKSUM_SIZE);
        size_t data_rev_index = i * DA_DATA_SIZE;

        // 提取 8 字节的 DA 数据
        unsigned char da_data[DA_DATA_SIZE];
        memcpy(da_data, &buffer[data_start_index], DA_DATA_SIZE);

        // 提取 2 字节的校验位
        unsigned short received_checksum = buffer[data_start_index + DA_DATA_SIZE] |
            (buffer[data_start_index + DA_DATA_SIZE + 1] << 8);

        // 校验
        if (CheckSum(da_data, DA_DATA_SIZE, received_checksum)) {
            // 校验通过，将数据返回
            memcpy(last_data, da_data, DA_DATA_SIZE); // 记录上一次数据
            memcpy(&revbuffer[data_rev_index], da_data, DA_DATA_SIZE);
        }
        else {
            DEBUG("Checksum error, data discarded.\n");
            memcpy(&revbuffer[data_rev_index], last_data, DA_DATA_SIZE); // 使用上一次的有效数据
        }
    }
}
/********************************************************************************************************************************************************************
															  用时检测
********************************************************************************************************************************************************************/
LARGE_INTEGER litmp;
LONGLONG QPart1 = 0, QFreq = 0;
void start_click_counter()
{
	QueryPerformanceFrequency(&litmp);
	QFreq = litmp.QuadPart;  //获得计数器的时钟频率
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart; //获得初始值
}
double get_click_counter()
{
	QueryPerformanceCounter(&litmp);
	return (double)(litmp.QuadPart - QPart1) * 1000 / QFreq;//单位为ms
}

/********************************************************************************************************************************************************************
															  内部指令
********************************************************************************************************************************************************************/
// 命令收发
short SetCommand(MSocket& mSkt, T_COMMAND& tCmd, void* sbuf, void* rbuf)
{
	short rtn = 0;
	T_COMMAND rCmd = { 0 };
	if (!mSkt.LinkStus) {
		return err_link_fail;
	}
	if (tCmd.length > 1500) {
		return err_data_length;
	}
	if (sbuf != NULL) {
		memcpy(&tCmd.data[0], sbuf, tCmd.length);
	}
	short ssz = tCmd.length + CMD_HEAD;
	if (mSkt.SendData((char*)&tCmd, ssz) <= 0) {
		return err_udp_send;
	}

	short rsz = mSkt.RecvData((char*)&rCmd);

	if (rsz <= 0) {
		return err_udp_timeout;
	}
	if (rsz - rCmd.length != CMD_HEAD) {
		return err_udp_recv;
	}
	rtn = rCmd.state;				//返回状态码
	if (rbuf != NULL && rCmd.length > 0) {
		memcpy(rbuf, &rCmd.data[0], rCmd.length);
		tCmd.length = rCmd.length;	//返回数据长度；
	}
	return rtn;
}
// 命令发送
short SendCommand(MSocket& mSkt, T_COMMAND& tCmd, void* sbuf, void* rbuf)
{
    short rtn = 0;
    T_COMMAND rCmd = { 0 };
    if (!mSkt.LinkStus) {
        return err_link_fail;
    }
    if (tCmd.length > 1500) {
        return err_data_length;
    }
    if (sbuf != NULL) {
        memcpy(&tCmd.data[0], sbuf, tCmd.length);
    }
    short ssz = tCmd.length + CMD_HEAD;
    if (mSkt.SendData((char*)&tCmd, ssz) <= 0) {
        return err_udp_send;
    }
    return rtn;
}
// 接收码流数据
short ReceiveData(MSocket& mSkt, T_COMMAND& tCmd, void* rbuf)
{
    if (!mSkt.LinkStus) {
        return err_link_fail;
    }

    uint16_t length = 0;
    T_COMMAND rCmd = { 0 };
    short rtn = mSkt.RecvData((char*)&rCmd);
    length = rtn - CMD_HEAD;

	if (rCmd.state == 0xFF)
	{
		double ms = 0;
		start_click_counter();
		// 调用处理函数处理接收到的数据包
        ProcessDataPacket(&rCmd.data[0], static_cast<unsigned char*>(rbuf), length);
    	ms = get_click_counter();
        //DEBUG("校验后:");
        //for (int i = 0; i < 1024; ++i) {
        //    DEBUG("%02x ", static_cast<unsigned char>(revbuff[i]));
        //}
        //DEBUG("\n");
        DEBUG("ProcessDataPacket: %fms\n", ms);   
	}

    tCmd.head = rCmd.head;      //头
    tCmd.state = rCmd.state;    //状态码
    tCmd.cmdno = rCmd.cmdno;    //命令号
    tCmd.subcmd = rCmd.subcmd;  //子命令号
    tCmd.length = rCmd.length;  //返回数据长度；

    return rtn; // 返回接收的字节数
}
// 通信测试
short _stdcall gmc_cmd_test(int idx, uint32_t* data, int count)
{
	int rtn = 0;
	MSocket& pSkt = stMSkt[0];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tCmd.cmdno = 11;
	tCmd.subcmd = idx;
	tCmd.length = (uint16_t)(sizeof(uint32_t) * count);
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, data, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}

short _stdcall gmc_connect_cmd()
{
	// 发送连接命令
	short rtn = 0;
	int cNo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tCmd.head = head;
	tCmd.cmdno = cmd_connect_device;
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, NULL, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}
// 设置采样率
short _stdcall gmc_set_sampling_rate(int rate) 
{
	short rtn = 0;
	int cNo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tCmd.head = head;
	tCmd.cmdno = cmd_set_sampling_rate;
	tCmd.length = 0x04;
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, &rate, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}
// 连续采样模式
short _stdcall gmc_sampling_mode_continuous()
{
	short rtn = 0;
	int cNo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tCmd.head = head;
	tCmd.cmdno = cmd_set_sampling_mode;
	tCmd.subcmd = 0x01;
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, NULL, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}
// 时间采样模式
short _stdcall gmc_sampling_mode_time(float time)
{
	short rtn = 0;
	int cNo = 0;
	int rtime = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tSampPrm.SampNum = time * tSampPrm.Rate/1000;
	rtime = (int)(time * tSampPrm.Rate / 1000 * 4 * 2);
	tCmd.head = head;
	tCmd.cmdno = cmd_set_sampling_mode;
	tCmd.subcmd = 0x02;
	tCmd.length = 0x04;
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, &rtime, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}


// 接收数据线程函数
DWORD WINAPI ReceiveDataThread(LPVOID lpParam) {
    MSocket* pSkt = (MSocket*)lpParam;
    uint16_t PackageCount = 0;

    T_COMMAND tCmd = { 0 };
    unsigned char buff[1500] = { 0 };

    while (tSampPrm.Status) {
        short rtn = ReceiveData(*pSkt, tCmd, buff);
        if (rtn > 0) {
            if (tCmd.state == 0xFF) {
                if (tCmd.subcmd != PackageCount + 1) {
                    printf("error count: %d\n", tCmd.subcmd);
                }
                //printf("Packcount:%d\n", tCmd.subcmd);           
                PackageCount = tCmd.subcmd;     // 更新包计数
            }

            if (!WriteToBuffer(&circBuffer, buff, tCmd.length)) {               // 将接收到的数据存储到循环缓冲区
                DEBUG("Warning: Circular buffer overflow, data discarded.\n");  // 缓冲区写入失败，可能是因为缓冲区满了，丢弃数据
                return err_write_data; // 返回错误代码
            }
        }
    }
    return 0;
}

// 处理数据线程函数
DWORD WINAPI ProcessingDataThread(LPVOID lpParam) {
    MSocket* pSkt = (MSocket*)lpParam;
    uint16_t PackageCount = 0;

    while (tSampPrm.Status) {
        T_COMMAND tCmd = { 0 };
        unsigned char buff[1500] = { 0 };
        short rtn = ReceiveData(*pSkt, tCmd, buff);
        //DEBUG("%d\n", rtn);
        //DEBUG("%d\n", tCmd.length);
        if (tCmd.subcmd != PackageCount + 1)
        {
            printf("error count:%d\n", tCmd.subcmd);
        }
        PackageCount = tCmd.subcmd;
        if (rtn > 0) {
            // 处理接收到的数据,将接收到的数据存储到循环缓冲区
            if (!WriteToBuffer(&circBuffer, buff, tCmd.length)) {
                // 缓冲区写入失败，可能是因为缓冲区满了，丢弃数据
                DEBUG("Warning: Circular buffer overflow, data discarded.\n");
                return err_write_data;
            }
        }
        tCmd.length = 0;
    }
    return 0;
}
/********************************************************************************************************************************************************************
                                                              API
********************************************************************************************************************************************************************/
// LIB库版本号  [24082201]十六进制，年-月-日-版本
short _stdcall gmc_get_lib_version(uint32_t *version)
{
    *version = LIB_VERSION;
    return 0;
}
// FPGA库版本号  [24082201]十六进制，年-月-日-版本
short _stdcall gmc_get_fpga_version(uint32_t *fpgaversion)
{
    short rtn = 0;
    MSocket& pSkt = stMSkt[0];
    if (!pSkt.LinkStus) { return err_link_fail; }
    T_COMMAND tCmd = { 0 };
    tCmd.head = head;
    tCmd.cmdno = cmd_get_version;
    uint32_t rData = 0;
    WaitForSingleObject(hmtx[0], INFINITE);
    rtn = SetCommand(pSkt, tCmd, NULL, &rData);
    if (rtn == 0) { *fpgaversion = rData; }
    ReleaseMutex(hmtx[0]);
    return rtn;

}
// 连接设备
short _stdcall ConnectedDevice() 
{
	short rtn = 0;
	int cNo = 0;
	char Ip[] = "192.168.137.2";			//目标IP地址
    //char Ip[] = "127.0.0.1";			    //目标IP地址
	short port = 6002;					    //目标端口
	char *IpAddr = Ip;
	MSocket& pSkt = stMSkt[cNo];
	// UDP通信连接
	if (pSkt.CsConnect(IpAddr, port)) {
		return err_link_fail;
	}
	else {
		// 创建互斥量
		hmtx[cNo] = CreateMutex(NULL, false, _T("Mutex_0"));
		if (ERROR_ALREADY_EXISTS == GetLastError()) {
			hmtx[cNo] = OpenMutex(MUTEX_ALL_ACCESS, false, _T("Mutex_0"));
			if (NULL == hmtx[cNo]) { return err_sem_mutex; }
		}
		else if (NULL == hmtx[cNo])
		{
			return err_sem_mutex;
		}
		rtn = gmc_connect_cmd();

	}
	return rtn;

}

// 关闭设备
short _stdcall CloseDevice() 
{
    short rtn = 0;
    int cNo = 0;
    MSocket& pSkt = stMSkt[cNo];
    pSkt.Close();
    return rtn;
}

// 配置设备，设置采样率，采样模式，采样时间
// mode[0:连续采样, 1:时间采样]，连续采样停止采集才停止，时间采样模式下，time才生效；time：ms单位
// rate：采样率 单位Hz
short _stdcall ConfigureADCParameters(int mode, float rate, float time)
{
	if (mode < 0 || mode > 1) { return err_parameter; }				//参数保护
	if (rate < 1 || rate > 1000000) { return err_parameter; }
	short rtn = 0;
	float rate_send = 1000000 / rate;
	tSampPrm.Mode = mode;											//记录下发数据
	tSampPrm.Rate = rate;
	tSampPrm.SampTime = time;
	rtn = gmc_set_sampling_rate(rate_send);
	if (mode == 0) {
		rtn = gmc_sampling_mode_continuous();
	}
	else{
		rtn = gmc_sampling_mode_time(time);
	}
	return rtn;
}

// 设置DAC1, DAC2, DAC3, DAC4
short _stdcall SetDACParameters(int DA[4])
{
    short rtn = 0;
    int cNo = 0;
    MSocket& pSkt = stMSkt[cNo];
    if (!pSkt.LinkStus) { return err_link_fail; }
    T_COMMAND tCmd = { 0 };
    tCmd.head = head;
    tCmd.cmdno = cmd_set_dac;
    tCmd.length = sizeof(int) * 4;
    WaitForSingleObject(hmtx[0], INFINITE);
    rtn = SetCommand(pSkt, tCmd, &DA[0], NULL);
    ReleaseMutex(hmtx[0]);
    return rtn;
}

// 开始采集
short _stdcall StartADCCollection()
{
	short rtn = 0;
	int cNo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	InitializeSystem();												// 初始化循环缓冲区
	char buff[1500] = { 0 };
	T_COMMAND tCmd = { 0 };
	tSampPrm.Status = 1;
	tCmd.head = head; 
	tCmd.cmdno = cmd_start_sampling;
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SendCommand(pSkt, tCmd, NULL, NULL);
	ReleaseMutex(hmtx[0]);
	// 创建接收数据线程
    if (hRecvThread != NULL) {
        CloseHandle(hRecvThread);
        hRecvThread = NULL;
    }
	hRecvThread = CreateThread(NULL, 0, ReceiveDataThread, (LPVOID)&pSkt, 0, NULL);
	if (hRecvThread == NULL) {
		// 线程创建失败，处理错误
		rtn = err_create_thread;									// 返回错误码
	}
    // 创建处理数据线程
    //if (hProcessingThread != NULL) {
    //    CloseHandle(hProcessingThread);
    //    hProcessingThread = NULL;
    //}
    //hProcessingThread = CreateThread(NULL, 0, ProcessingDataThread, (LPVOID)&pSkt, 0, NULL);
    //if (hProcessingThread == NULL) {
    //    // 线程创建失败，处理错误
    //    rtn = err_create_thread;									// 返回错误码
    //}
	return rtn;
}

// 停止采集
short _stdcall StopADCCollection()
{
	short rtn = 0;
	int cNo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tSampPrm.Status = 0;											//采集标志位置0
	tCmd.head = head;
	tCmd.cmdno = cmd_stop_sampling;
	// 等待接收数据线程结束

	if (hRecvThread != NULL) {
		//WaitForSingleObject(hRecvThread, INFINITE);
		CloseHandle(hRecvThread);
		hRecvThread = NULL;
	}
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SendCommand(pSkt, tCmd, NULL, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}
// 读取缓冲区数据；
short _stdcall TryReadADCData(unsigned char* readbuffer, uint32_t readsize) // 默认1024
{
    short rtn = 0;  // 返回值，0表示成功，非0表示失败
    char data[1024];  // 存储读取的数据
    int length = readsize;  // 假设每次处理1024字节

    if (ReadFromBuffer(&circBuffer, data, length)) {
        // 成功读取到数据，将数据拷贝到调用者提供的缓冲区
        memcpy(readbuffer, data, length);
        // 打印读取到的数据
        DEBUG("buffer data:");
        for (int i = 0; i < length; ++i) {
            DEBUG("%02x ", static_cast<unsigned char>(readbuffer[i]));
        }
        DEBUG("\n");

        rtn = err_none;  // 成功读取到数据
    }
    else {
        // 缓冲区中没有足够数据，等待
        DEBUG("Buffer underflow, waiting for data...\n");
        Sleep(10);					 // 暂停一会，避免忙等待
        rtn = err_data_read;		 // 表示没有读取到数据
    }

    return rtn;
}
// 设置IO输出；IO按位输出函数index[0,15] value[0关闭,1打开] 
short _stdcall SetIoOutput(int index, int value) 
{
	if (index < 0 || index > 31) { return err_parameter; }			//参数保护
	if (value < 0 || value > 1) { return err_parameter; }
	short rtn = 0;
	int cNo = 0;
	uint32_t IoDo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tCmd.head = head;
	tCmd.cmdno = cmd_set_output;
    tCmd.length = sizeof(int);
	if (value) {
		BIT_SET(IoDo, index);
	}
	else {
		BIT_CLR(IoDo, index);
	}
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, &IoDo, NULL);
	ReleaseMutex(hmtx[0]);
	return rtn;
}
// 获取IO输入状态
short _stdcall GetIoInput(int index, uint32_t *value)
{
	if (index < 0 || index > 31) { return err_parameter; }			//参数保护
	short rtn = 0;
	int cNo = 0;
	uint32_t IoDo = 0;
	MSocket& pSkt = stMSkt[cNo];
	if (!pSkt.LinkStus) { return err_link_fail; }
	T_COMMAND tCmd = { 0 };
	tCmd.head = head;
	tCmd.cmdno = cmd_get_input;
	WaitForSingleObject(hmtx[0], INFINITE);
	rtn = SetCommand(pSkt, tCmd, NULL, &IoDo);
	*value = BIT_VAL(IoDo, index);
	ReleaseMutex(hmtx[0]);
	return rtn;
}
//设置输入滤波时间
short _stdcall SetFilterTime(int time)
{

    short rtn = 0;
    int cNo = 0;
    MSocket& pSkt = stMSkt[cNo];
    if (!pSkt.LinkStus) { return err_link_fail; }
    T_COMMAND tCmd = { 0 };
    tCmd.head = head;
    tCmd.cmdno = cmd_set_filtering_time;
    tCmd.length = sizeof(int);
    WaitForSingleObject(hmtx[0], INFINITE);
    rtn = SetCommand(pSkt, tCmd, &time, NULL);
    ReleaseMutex(hmtx[0]);
    return rtn;
}
//设置触发IO，（0-7位设置输入引脚，24-31位为1开启外部触发，为0关闭外部触发）
short _stdcall SetTriggerIo(int index, uint8_t enable)
{
    if (index < 0 || index > 7) { return err_parameter; }
    short rtn = 0;
    int cNo = 0;
    uint32_t cmd = 0;
    MSocket& pSkt = stMSkt[cNo];
    if (!pSkt.LinkStus) { return err_link_fail; }
    T_COMMAND tCmd = { 0 };
    tCmd.head = head;
    tCmd.cmdno = cmd_set_trigger_io;
    tCmd.length = sizeof(uint32_t);
    cmd = index | (enable << 24);
    WaitForSingleObject(hmtx[0], INFINITE);
    rtn = SetCommand(pSkt, tCmd, &cmd, NULL);
    ReleaseMutex(hmtx[0]);
    return rtn;
}
//设置采样电阻，Resistor[3],三路电阻值0-7
short _stdcall SetSampResistor(uint8_t resistor[3])
{
    if (resistor[0] < 0 || resistor[0] > 7) { return err_parameter; }
    if (resistor[1] < 0 || resistor[1] > 7) { return err_parameter; }
    if (resistor[2] < 0 || resistor[2] > 7) { return err_parameter; }
    short rtn = 0;
    int cNo = 0;
    uint32_t cmd = 0;
    MSocket& pSkt = stMSkt[cNo];
    if (!pSkt.LinkStus) { return err_link_fail; }
    T_COMMAND tCmd = { 0 };
    tCmd.head = head;
    tCmd.cmdno = cmd_set_sampling_resistor;
    tCmd.length = sizeof(uint32_t);
    cmd = resistor[0] | (resistor[1] << 8) | (resistor[2] << 16);
    WaitForSingleObject(hmtx[0], INFINITE);
    rtn = SetCommand(pSkt, tCmd, &cmd, NULL);
    ReleaseMutex(hmtx[0]);
    return rtn;
}