/*
 *A basic server class.
 *Manages all game connections and socket operations for the game.
 */
#include "server.h"

#include "banList.h"
#include "callout.h"
#include "calloutManager.h"
#include "log.h"
#include "player.h"
#include "socket.h"
#include "telnet.h"
#include "utils.h"
#include "world.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <list>

#include <netdb.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

Server::Server()
{
    CalloutManager* manager = CalloutManager::GetInstance();
    World* world = World::GetPtr();
    control = -1; // listening socket
    // clear descriptor sets:
    FD_ZERO(&fSet);
    FD_ZERO(&rSet);
    // clear the address buffer
    memset(&my_addr, 0, sizeof(my_addr));
    // initialize the ban list.
    blist = new BanList();
    world->AddState("banlist", blist);
    // initialize lastSleep
    gettimeofday(&lastSleep, nullptr);
    manager->RegisterCallout(
        30, 0,
        [this](Callout* cb)
        {
            CheckLinkdeaths(cb);
        },
        false);
}
Server::~Server()
{
    // free variables and close sockets.
    if (control != -1)
    {
        close(control);
    }
}

void Server::CheckLinkdeaths(Callout* cb)
{
    time_t tm = time(nullptr);
    std::list<LinkdeathNode*>::iterator it, itEnd;
    LinkdeathNode* node = nullptr;

    itEnd = _linkdead.end();
    for (it = _linkdead.begin(); it != itEnd; ++it)
    {
        node = (*it);
        if (tm - (node->added) >= LINKDEATH_TIME)
        {
            node->mobile->LeaveGame();
            delete node->mobile;
            delete node;
            it = _linkdead.erase(it);
        }
    }
}

bool Server::Listen(const int port)
{
    int reuse = 1;
    // try to create a communications endpoint
    control = socket(PF_INET, SOCK_STREAM, 0);
    if (control == -1)
    {
        WriteLog(SeverityLevel::Fatal, "could not create socket.");
        return false;
    }
    // set options for this control socket
    if (setsockopt(control, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) == -1)
    {
        WriteLog(SeverityLevel::Fatal, "Setting socket options failed.");
        close(control);
        return false;
    }
    // attach the control socket to it's own filedescriptor set
    FD_SET(control, &fSet);
    // settings for this socket, (and set the listning port)
    my_addr.sin_family = PF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);
    // try to bind the mud port
    if (bind(control, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        WriteLog(SeverityLevel::Fatal, "Could not bind socket.");
        close(control);
        return false;
    }
    // start listening
    if (listen(control, LISTEN_BACKLOG) == -1)
    {
        WriteLog(SeverityLevel::Fatal, "Could not set parent socket to listening state.");
        close(control);
        return false;
    }

    return true;
}

int Server::GetListener() const
{
    return control;
}

bool Server::PollSockets()
{
    static struct timeval tv;
    std::list<Socket*>::iterator iSocket, iSocketEnd;
    Socket* sock = nullptr;
    std::string input;

    iSocketEnd = socketList.end();
    iSocket = socketList.begin();
    for (; iSocket != iSocketEnd;)
    {
        sock = *iSocket;
        // read and update
        if (sock->ShouldClose())
        {
            iSocket = CloseSocket(iSocket);
            sock = *iSocket;
            continue;
        }
        // attempt to read from this socket if pending incoming data
        if (FD_ISSET(sock->GetControl(), &rSet))
        {
            // if read fails, close the connection
            if (sock->Read() == false)
            {
                if (sock->GetMobile() && sock->GetConnectionType() == CON_Game)
                {
                    LinkdeathNode* ld = new LinkdeathNode();
                    ld->added = time(nullptr);
                    ld->mobile = sock->GetMobile();
                    _linkdead.push_back(ld);
                    sock->Linkdeath();
                    iSocket = CloseSocket(iSocket);
                }
                else
                {
                    sock->Kill();
                }
                continue;
            }
            // update their last input since we received data
            sock->UpdateLastCommand();
        }

        if (sock->CommandPending())
        {
            sock->HandleCommand();
        }

        ++iSocket;
    }

    // attempt to establish new connections
    memcpy(&rSet, &fSet, sizeof(fd_set));
    // poll the descriptor set
    if (select(FD_SETSIZE, &rSet, nullptr, nullptr, &tv) < 0)
    {
        return false;
    }
    Accept();
    return true;
}

void Server::FlushSockets()
{
    std::list<Socket*>::iterator iSocket, iSocketEnd;
    iSocketEnd = socketList.end();
    Socket* pSocket = nullptr;

    // iterate through all sockets and flush outgoing data
    for (iSocket = socketList.begin(); iSocket != iSocketEnd; ++iSocket)
    {
        pSocket = (*iSocket);
        // Attempt to flush this socket, close socket if failure
        if (pSocket->Flush() == false)
        {
            iSocket = CloseSocket(iSocket);
        }
    }
}

// Sleep() should be called with an argument that can be divided by 1000,
// like 4, 5, 8 or 10. This is the amount of "commands" that will
// be processed each second, and it is recommended to have a
// constant defined for this purpose.
int Server::Sleep(int pps)
{
    struct timeval newTime;
    unsigned long long int spent = 0;     // time spent updating (not sleeping)
    unsigned long long int remaining = 0; // time remaining to sleep for.

    if (pps <= 0)
    {
        return 0;
    }

    gettimeofday(&newTime, nullptr);
    spent = (newTime.tv_sec - lastSleep.tv_sec) * 1000000;
    spent += (newTime.tv_usec - lastSleep.tv_usec);
    remaining = (MS_PER_PULSE * 1000);
    if (spent >= remaining)
    {
        gettimeofday(&lastSleep, nullptr);
        WriteLog(SeverityLevel::Warning, "Exceeded frame time.");
        return 0;
    }
    remaining -= spent;

    newTime.tv_sec = 0;
    newTime.tv_usec = remaining;
    select(0, nullptr, nullptr, nullptr, &newTime);
    gettimeofday(&lastSleep, nullptr);
    return remaining;
}

std::list<Socket*>::iterator Server::CloseSocket(std::list<Socket*>::iterator& it)
{
    Socket* tmp = (*it);
    tmp->Flush();
    // clear the sockets descriptor from the listening set
    FD_CLR(tmp->GetControl(), &rSet);
    FD_CLR(tmp->GetControl(), &fSet);
    // and finally delete the socket
    delete tmp;
    // remove the socket.
    return socketList.erase(it);
}

void Server::Accept()
{
    Socket* pSocket = nullptr;
    sockaddr_in addr;
    unsigned int len = sizeof(addr);
    int desc = 0;
    int argp = 1;

    // any new connections pending ?
    if (!FD_ISSET(control, &rSet))
    {
        return;
    }

    // clear the address buffer:
    memset(&addr, 0, len);

    // try to accept new connection
    desc = accept(control, (struct sockaddr*)&addr, &len);
    if (desc == -1)
    {
        return;
    }

    // allocate a new socket
    pSocket = new Socket(desc);
    pSocket->SetPort(ntohs(addr.sin_port));
    // set non-blocking I/O
    ioctl(desc, FIONBIO, &argp);
    argp = 1;
    setsockopt(desc, SOL_SOCKET, SO_KEEPALIVE, (void*)&argp, sizeof(int));

    if (blist->AddressExists(addr.sin_addr.s_addr))
    {
        pSocket->Write("This IP has been banned.");
        pSocket->Kill();
        delete pSocket;
        return;
    }

    World* world = World::GetPtr();
    // negotiation stuff
    pSocket->Write(TELNET_DO_TERMTYPE);
    pSocket->Write(TELNET_DO_COMPRESS2);
    pSocket->Write(TELNET_DO_NAWS);

    // greet users:
    pSocket->Write("\n");
    pSocket->Write(world->GetBanner());
    pSocket->Write("What is your name? Type new for a new character.\n");

    // Set the connection state
    pSocket->SetConnectionType(CON_Name);

    // attach to socket list
    socketList.push_back(pSocket);
    // attach to file descriptor set
    FD_SET(desc, &fSet);
    // copy the address to memory:
    memcpy(pSocket->GetAddr(), &addr, sizeof(sockaddr_in));
    // perform lookup:
    struct hostent* host = nullptr;
    host = gethostbyaddr((&addr.sin_addr), sizeof(in_addr), AF_INET);
    if ((!host) || (!host->h_name))
    {
        pSocket->SetHost(inet_ntoa(pSocket->GetAddr()->sin_addr));
    }
    else
    {
        pSocket->SetHost(std::string(host->h_name));
    }

    WriteLog("New connection from " + pSocket->GetHost() + ".");
}

std::list<Socket*> Server::GetSocketList()
{
    return socketList;
}

void Server::Recover(int listener)
{
    control = listener;
    FD_SET(control, &fSet);
}

void Server::AddSock(Socket* sock)
{
    socketList.push_back(sock);
    // attach to file descriptor set
    FD_SET(sock->GetControl(), &fSet);
}

BanList* Server::GetBanList() const
{
    return blist;
}

Player* Server::GetLinkdeadUser(const std::string& name) const
{
    for (auto it : _linkdead)
    {
        if (it->mobile->GetName() == name)
        {
            return it->mobile;
        }
    }

    return nullptr;
}
void Server::RemoveLinkdeadUser(const std::string& name)
{
    std::list<LinkdeathNode*>::iterator it, itEnd;
    LinkdeathNode* node = nullptr;

    itEnd = _linkdead.end();
    for (it = _linkdead.begin(); it != itEnd; ++it)
    {
        node = (*it);
        if (node->mobile->GetName() == name)
        {
            it = _linkdead.erase(it);
            delete node;
            break;
        }
    }
}
