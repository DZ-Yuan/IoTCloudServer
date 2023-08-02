#ifndef COMMON_H
#define COMMON_H

//
#define ushort unsigned short
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long long

// server define

#define SafeDelete(p) \
    if (p)            \
    {                 \
        delete p;   \
        p = nullptr;  \
    }

#endif