#ifndef _STRSTR_IO_H
#define _STRSTR_IO_H

#include <cstdint>

namespace StructStream {

struct IOIntf {
public:
    virtual ~IOIntf() {};
    virtual intptr_t read(void *buf, const intptr_t len) = 0;
    virtual intptr_t write(const void *buf, const intptr_t len) = 0;
};

struct MemoryIO: public IOIntf {
public:
    MemoryIO(const void *srcbuf, const intptr_t len);
    MemoryIO(const MemoryIO &ref);
    virtual ~MemoryIO();
private:
    void* _buf;
    intptr_t _len;
    intptr_t _offs;
public:
    MemoryIO& operator=(const MemoryIO &ref);
    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);
};

void sread(IOIntf *io, void *buf, const intptr_t len);
void swrite(IOIntf *io, const void *buf, const intptr_t len);

}

#endif
