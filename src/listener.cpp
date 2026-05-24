#include "listener.h"

#include "baseSocket.h"
#include "event.h"
#include "mud.h"
#include "world.h"

#include <cstdlib>
#include <cstring>
#include <list>
#include <string>

#include <unistd.h>

std::list<BaseSocket*>::iterator Listener::CloseSocket(std::list<BaseSocket*>::iterator& it)
{
    BaseSocket* tmp = (*it);
    tmp->Flush();
    tmp->Close();
    // clear the sockets descriptor from the listening set
    FD_CLR(tmp->GetControl(), &fset);
    // and finally delete the socket
    delete tmp;
    // remove the socket.
    return _sockets.erase(it);
}

Listener::Listener() : _control(-1)
{
    FD_ZERO(&fset);
    FD_ZERO(&rset);
}

void Listener::Accept()
{
    sockaddr_in addr;
    int fd = 0;
    int arg = 0;

    memset(&addr, 0, sizeof(addr));
    if (!FD_ISSET(_control, &rset))
    {
        return;
    }

    arg = sizeof(addr);
    fd = accept(_control, (sockaddr*)&addr, (socklen_t*)&arg);
    if (fd == -1)
    {
        return;
    }

    arg = 1;
    BaseSocket* sock = new BaseSocket(fd);
    ioctl(fd, FIONBIO, &arg);
    _sockets.push_back(sock);
    FD_SET(fd, &fset);
    memcpy(sock->GetAddr(), &addr, sizeof(sockaddr_in));
}

bool Listener::Listen(int port)
{
    int unused = 1;

    _control = socket(PF_INET, SOCK_STREAM, 0);
    if (_control == -1)
    {
        return false;
    }

    if (setsockopt(_control, SOL_SOCKET, SO_REUSEADDR, (const char*)&unused, sizeof(int)) == -1)
    {
        close(_control);
        return false;
    }

    FD_SET(_control, &fset);
    _addr.sin_family = PF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(port);

    if (bind(_control, (struct sockaddr*)&_addr, sizeof(_addr)) == -1)
    {
        close(_control);
        return false;
    }
    if (listen(_control, LISTEN_BACKLOG) == -1)
    {
        close(_control);
        return false;
    }

    return true;
}

void Listener::Poll()
{
    std::list<BaseSocket*>::iterator it, itEnd;
    BaseSocket* sock = nullptr;

    memcpy(&rset, &fset, sizeof(fd_set));
    if (select(FD_SETSIZE, &rset, nullptr, nullptr, nullptr) < 0)
    {
        return;
    }
    Accept();

    itEnd = _sockets.end();
    for (it = _sockets.begin(); it != itEnd; ++it)
    {
        sock = (*it);
        if (FD_ISSET(sock->GetControl(), &rset))
        {
            // if read fails, close the connection
            if (sock->Read() == false)
            {
                it = CloseSocket(it);
                continue;
            }
        }
    }
}
