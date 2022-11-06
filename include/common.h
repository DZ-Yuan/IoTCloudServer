#ifndef COMMON_H
#define COMMON_H

//
#define ushort unsigned short

// server define

#define SafeDelete(p) \
    if (p)            \
    {                 \
        delete p;     \
        p = nullptr;  \
    }

#endif