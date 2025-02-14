//#pragma once
//
//#include <iostream>
//#include <stdint.h>
//#include <string.h>
//#include <emmintrin.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//
//class Ringbuffer
//{
//public:
//    Ringbuffer(const Ringbuffer&)=delete;
//    Ringbuffer& operator=(const Ringbuffer&)=delete;
//
//    explicit Ringbuffer(int size);
//    ~Ringbuffer();
//    int read(uint8_t* buffer, int len);
//    int write(uint8_t* buffer, int len);
//
//private:
//    uint8_t *m_buffer;
//    int buffer_size;
//    int buffer_size_mask;
//    volatile int rpos;
//    volatile int wpos;
//    bool is_init;
//};
//
//#ifdef __cplusplus
//}
//#endif