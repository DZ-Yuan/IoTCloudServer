#ifndef DATAPACKET
#define DATAPACKET

#define PACKET_SIZE 128

struct Packet
{
    unsigned short len;
    char *data;

    Packet()
    {
        memset(this, 0, sizeof(*this));
    }
};

class DataPacket
{
public:
    DataPacket();
    DataPacket(char *mem_ptr, int size);
    ~DataPacket();

public:
    char *GetMemPtr()
    {
        return mem_ptr_;
    }

    bool IsEmpty()
    {
        return !mem_ptr_;
    }

    int GetNetPacket(char *pack_ptr);
    void GetNetPacket(Packet &p);

    void ClearData()
    {
        if (mem_ptr_)
        {
            delete mem_ptr_;
            mem_ptr_ = nullptr;
            end_ptr_ = nullptr;
            offset_ = nullptr;
        }
    }

    // IO
    template <typename T>
    void WriteData(T &data);

    template <typename T>
    void WriteData(T &&data);

    template <typename T>
    T ReadOnce();

    template <typename T>
    T rReadOnce();

private:
    char *mem_ptr_;
    char *end_ptr_;
    char *offset_;
};

template <typename T>
void DataPacket::WriteData(T &data)
{
    int size = sizeof(T);
    memcpy(this->offset_, &data, size);

    offset_ += size;
}

template <typename T>
void DataPacket::WriteData(T &&data)
{
    int s = sizeof(T);
    memcpy(this->offset_, &data, s);

    offset_ += s;
}

template <typename T>
T DataPacket::ReadOnce()
{
    if (offset_ >= end_ptr_)
    {
        return -1;
    }

    T val;
    val = *(T *)offset_;
    offset_ += sizeof(T);

    return val;
}

template <typename T>
T DataPacket::rReadOnce()
{
    if (this->offset_ <= this->mem_ptr_)
    {
        return -1;
    }

    T val;
    offset_ -= sizeof(T);
    val = *(T *)offset_;

    return val;
}

// ----------------------------

// ----------------------------
class DataPacketReader
{
public:
private:
    char *mem_ptr_;
    char *end_ptr_;
    char *offset_;
};

#endif