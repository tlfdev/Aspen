#include "baseSocket.h"

#include "conf.h"
#include "mud.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

BaseSocket::BaseSocket() : _control(-1) {}
BaseSocket::BaseSocket(int desc) : _control(desc) {}
BaseSocket::~BaseSocket()
{
    if (_control != -1)
    {
        close(_control);
        _control = -1;
    }
}

int BaseSocket::GetControl() const
{
    return _control;
}

bool BaseSocket::Read()
{
    char temp[4096 + 2];
    int size = 0;
    int k = 0;
    std::string line;

    while (true)
    {
        size = recv(_control, temp, 4096, 0);
        if (size > 0)
        {
            temp[size] = '\0'; // sets the last byte we received to null.
            // iterate through the list and add that to the std::string
            for (k = 0; k < size; k++)
            {
                _inBuffer += temp[k];
            }
        }
        else if (size == 0)
        {
            return false;
        }
        else if (errno == EAGAIN)
        {
            break;
        }
        else
        {
            return false;
        }
    }

    return true;
}
void BaseSocket::Write(const std::string& txt)
{
    _outBuffer += txt;
}
size_t BaseSocket::Write(const unsigned char* data)
{
    Flush();
    size_t length = strlen((const char*)data);
    return Write((void*)const_cast<unsigned char*>(data), length);
}
size_t BaseSocket::Write(const void* buffer, size_t count)
{
    Flush();
    return write(_control, buffer, count);
}

bool BaseSocket::Flush()
{
    int b = 0;
    int w = 0;

    if (!_outBuffer.length())
    {
        return true;
    }

    while (_outBuffer.length() > 0)
    {
        b = (_outBuffer.length() < 4096) ? _outBuffer.length() : 4096;
        // any write failures ?
        if (_control != -1)
        {
            if ((w = send(_control, _outBuffer.c_str(), b, 0)) == -1)
            {
                return false;
            }
        }
        // move the buffer down
        _outBuffer.erase(0, w);
    }

    return true;
}

std::string BaseSocket::GetInBuffer()
{
    return _inBuffer;
}
void BaseSocket::ClrInBuffer()
{
    _inBuffer.erase();
}
bool BaseSocket::InputPending() const
{
    return (_inBuffer.empty() ? false : true);
}

sockaddr_in* BaseSocket::GetAddr()
{
    return &_addr;
}
void BaseSocket::SetAddr(sockaddr_in* addr)
{
    memcpy(&_addr, addr, sizeof(sockaddr_in));
}

bool BaseSocket::Close()
{
    if (_control != -1)
    {
        close(_control);
        _control = -1;
        return true;
    }

    return false;
}
bool BaseSocket::Connect(const char* address, unsigned short port)
{
    if (_control != -1)
    {
        return false;
    }

    memset(&_addr, 0, sizeof(sockaddr_in));
    if (!inet_aton(address, &_addr.sin_addr))
    {
        return false;
    }
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    _control = socket(AF_INET, SOCK_STREAM, 0);

    if (_control == -1)
    {
        return false;
    }
    if (connect(_control, (sockaddr*)&_addr, sizeof(sockaddr_in)) == -1)
    {
        return false;
    }

    return true;
}
