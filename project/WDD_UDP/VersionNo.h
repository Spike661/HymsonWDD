#ifndef	_VERSION_NO_H__
#define	_VERSION_NO_H__

#define LIB_VERSION     0x25032401
#define NO_PRODUCTVER   25,03,24,01
#define STR_PRODUCTVER  "25.03.24.01"
#define NO_FILEVER       1,0,0,1
#define STR_FILEVER     "1.0.0.1"
#define STR_PRODUCTNAME "Hymson WDD"

#ifdef _DEBUG  
#ifndef _WIN64 
#define STR_FILEDES    "GMC-32D"
#else  
#define STR_FILEDES    "GMC-64D"
#endif  
#else  
#ifndef _WIN64
#define STR_FILEDES    "GMC-32R"
#else  
#define STR_FILEDES    "GMC-64R"
#endif  
#endif
///////////////////////////////////////////////////////////////////////////////
#endif
