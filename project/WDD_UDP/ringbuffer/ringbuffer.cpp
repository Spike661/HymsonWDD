//#include "ringbuffer.h"
//
//#define smp_mb() _mm_mfence()  // 在 Visual Studio 中实现类似功能
//#define MIN(x, y) ((x)<(y) ? (x) : (y))
//
//Ringbuffer::Ringbuffer(int size):
//    m_buffer(nullptr),buffer_size(1),buffer_size_mask(0),
//    rpos(0),wpos(0),is_init(false)
//{
//    if(size<=0){
//        std::cout << "Input size error!" << std::endl;
//        return;
//    }
//    do{
//        size /= 2;
//        buffer_size *= 2;
//    }while(size);
//    std::cout << "buffer_size=" << buffer_size << std::endl;
//    buffer_size_mask = buffer_size-1;
//    m_buffer = new uint8_t[buffer_size];
//}
//
//Ringbuffer::~Ringbuffer()
//{
//    if(m_buffer) delete[] m_buffer;
//    m_buffer=nullptr;
//}
//
//
//
//int Ringbuffer::write(uint8_t* buffer, int len)
//{
//    register int available_len = ((rpos-wpos)+buffer_size)&buffer_size_mask;
//    if(available_len <= len){
//        if(!is_init){
//            is_init = true;
//            goto run;
//        }
//        return -1;
//    }
//
//run:
//
//    register int left_len = MIN(len, buffer_size-wpos);
//    ::memcpy(m_buffer+wpos, buffer, left_len);
//    ::memcpy(m_buffer,buffer+left_len, len-left_len);
//    
//    smp_mb();
//    wpos += len;
//    wpos &= buffer_size_mask;
//
//    return len;
//}
//
//int Ringbuffer::read(uint8_t* buffer, int len)
//{
//    register int available_len = ((wpos-rpos)+buffer_size)&buffer_size_mask;
//    if(available_len <= len) return -1;
//    
//    register int left_len=MIN(len, buffer_size-rpos);
//    ::memcpy(buffer, m_buffer+rpos, left_len);
//    ::memcpy(buffer+left_len, m_buffer, len-left_len);
//    
//    smp_mb();
//    rpos += len;
//    rpos &= buffer_size_mask;
//
//    return len;
//}
//
//#undef MIN
