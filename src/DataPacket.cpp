#include "pch.h"

DataPacket::DataPacket()
{
    int len = PACKET_SIZE;
    this->mem_ptr_ = (char *)malloc(len);
    memset(this->mem_ptr_, 0, len);
    this->offset_ = this->mem_ptr_;
    this->end_ptr_ = this->mem_ptr_ + len;
}

DataPacket::DataPacket(char *mem_ptr, int size)
{
    mem_ptr_ = mem_ptr;
    end_ptr_ = mem_ptr_ + size;
    offset_ = mem_ptr_;
}

DataPacket::~DataPacket()
{
   ClearData();
}

int DataPacket::GetNetPacket(char *pack_ptr)
{
    int net_size = offset_ - mem_ptr_ + sizeof(int);
    char *pack = (char *)malloc(net_size);
    memset(pack, 0, net_size);

    pack_ptr = pack;

    int pack_size = offset_ - mem_ptr_;
    memcpy(pack, &pack_size, sizeof(pack_size));

    // cp data
    memcpy(pack + sizeof(pack_size), mem_ptr_, pack_size);

    return pack_size;
}

void DataPacket::GetNetPacket(Packet &p)
{
    p.len = offset_ - mem_ptr_;
    p.data = mem_ptr_;
}

// template<typename T>
// void DataPacket::WriteData(T &data){
//     int size = sizeof(T);
//     memcpy(this->offset_, &data, size);

//     offset_ += size;
// }

// template<typename T>
// T DataPacket::ReadOnce(){
//     if (this->offset_ <= this->mem_ptr_)
//     {
//         return -1;
//     }

//     T val;
//     offset_ -= sizeof(T);
//     val = *(T*)offset_;

//     return val;
// }
