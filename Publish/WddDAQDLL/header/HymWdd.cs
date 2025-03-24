using System;
using System.Runtime.InteropServices;

namespace Hymmc.WDD
{
    public class HymWdd
    {
        // LIB库版本号  [24082201]十六进制，年-月-日-版本
        [DllImport("wdd.dll", EntryPoint = "gmc_get_lib_version", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short gmc_get_lib_version(ref uint version);

        // 连接设备
        [DllImport("wdd.dll", EntryPoint = "ConnectedDevice", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short ConnectedDevice();

        // 关闭设备
        [DllImport("wdd.dll", EntryPoint = "CloseDevice", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short CloseDevice();

        // 配置设备，设置采样率，采样模式，采样时间
        // mode[0:连续采样, 1:时间采样]，连续采样停止采集才停止，时间采样模式下，time才生效
        // rate：采样率 单位Hz
        [DllImport("wdd.dll", EntryPoint = "ConfigureADCParameters", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short ConfigureADCParameters(int mode, float rate, float time);

        // 设置DAC1, DAC2, DAC3, DAC4; 四个通道的DA值
        [DllImport("wdd.dll", EntryPoint = "SetDACParameters", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short SetDACParameters(uint16_t DA[4]);

        // 开始采集
        [DllImport("wdd.dll", EntryPoint = "StartADCCollection", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short StartADCCollection();

        // 停止采集
        [DllImport("wdd.dll", EntryPoint = "StopADCCollection", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short StopADCCollection();

        // 读取缓冲区数据；
        // 默认1024
        [DllImport("wdd.dll", EntryPoint = "TryReadADCData", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short TryReadADCData(string read_buffer, uint read_size);

        // 设置IO输出；IO按位输出函数index[0,15] value[0关闭,1打开] 
        [DllImport("wdd.dll", EntryPoint = "SetIoOutput", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short SetIoOutput(int index, int value);
         
        // 获取IO输入状态；
        [DllImport("wdd.dll", EntryPoint = "GetIoInput", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short GetIoInput(int index, ref int value);

        // 设置输入滤波时间,单位40ns
        [DllImport("wdd.dll", EntryPoint = "SetFilterTime", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short SetFilterTime(int time);

        // 设置触发IO，index[0,7] enable[0关闭,1打开]
        [DllImport("wdd.dll", EntryPoint = "SetTriggerIo", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short SetTriggerIo(int index, uint enable);

        // 设置采样电阻，Resistor[3],三路电阻值[1-8]
        [DllImport("wdd.dll", EntryPoint = "SetSampResistor", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        public static extern short SetSampResistor(uint Resistor[3]);



    }
}

