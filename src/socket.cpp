#include "socket.h"

#include "player.h"
#include "telnet.h"
#include "telnetParser.h"
#include "telnetParserEvents.hpp"
#include "utils.h"
#include "world.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

InputHandle::InputHandle()
{
    _sock = nullptr;
}
InputHandle::~InputHandle() {}
void InputHandle::Attach(Socket* sock)
{
    _sock = sock;
}
void InputHandle::Active(in_data* in) {}
void InputHandle::Input(void* arg, const std::string& input) {}

Socket::Socket(const int desc) : BaseSocket(desc)
{
    _termtype = "unknown";
    _mobile = nullptr;
    _Close = false;
    _lastInput = time(nullptr);
    _compressing = false;
    _port = -1;
    _totalReceived = 0;
    _totalSent = 0;
    _compressedSent = 0;
    _winsize.width = 80;
    _winsize.height = 23;
    cbuff = nullptr;

    // parser events.
    _parser.events.AddCallback("OnNegotiation", std::bind(&Socket::OnNegotiation, this, std::placeholders::_1));
    _parser.events.AddCallback("OnOption", std::bind(&Socket::OnOption, this, std::placeholders::_1));
}
Socket::~Socket()
{
    in_data* data = nullptr;
    if (_mobile)
    {
        delete _mobile;
        _mobile = nullptr;
    }

    while (!_input.empty())
    {
        data = _input.top();
        _input.pop();
        if (data->handle)
        {
            delete data->handle;
        }
        delete data;
    }
    if (cbuff)
    {
        delete[] cbuff;
    }
}

void Socket::Linkdeath()
{
    if (_mobile)
    {
        _mobile->SetSocket(nullptr);
        _mobile = nullptr;
        _Close = true;
    }
}

bool Socket::Read()
{
    char temp[4096 + 2];
    int size = 0;
    int k = 0;
    char* newbuff = nullptr;
    size_t nlpos = 0;
    std::string line;

    while (true)
    {
        size = read(_control, temp, 4096);
        if (size == -1) // error
        {
            // these are ok, we just continue on.
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return true;
            }
            /// other errors means we have a problem.
            else
            {
                return false;
            }
        }
        // linkdead
        if (size == 0)
        {
            return false;
        }
        // we have something to read
        _totalReceived += size; // add the number of bytes received to the counter.
        temp[size] = '\0';      // sets the last byte we received to null.
        _parser.Initialize(size);
        _parser.SetBuffer((unsigned char*)temp);
        _parser.Parse();
        newbuff = (char*)_parser.GetFinalBuffer();
        size = _parser.GetFinalSize();
        if (size == 0)
        {
            return true;
        }

        for (k = 0; k < size; k++)
        {
            _inBuffer += newbuff[k];
        } // end iteration of buffer
        // check for a newline character.
        nlpos = _inBuffer.find_first_of("\r\n");
        if (nlpos != std::string::npos)
        {
            line = _inBuffer.substr(0, nlpos);
            // we have a newline, push it to our command queue.
            if (line.length())
            {
                _cqueue.push(line);
            }
            _inBuffer.clear();
            break;
        }
    }
    _parser.Reset();
    return true;
}

// telnet handler functions
void Socket::OnOption(EventArgs* args)
{
    OptionEventArgs* eargs = (OptionEventArgs*)args;
    unsigned char option = eargs->GetOption();
    unsigned char command = eargs->GetCommand();
    // mccp2
    if (command == TELNET_WILL && option == TELNET_COMPRESS2)
    {
        Write(TELNET_COMPRESS2_STR);
        Flush();
        if (!InitCompression())
        {
            Kill();
        }
        return;
    }
    // termtype
    if (command == TELNET_WILL && option == TELNET_TERMTYPE)
    {
        Write(TELNET_REQUEST_TERMTYPE);
        return;
    }

    return;
}
void Socket::OnNegotiation(EventArgs* args)
{
    NegotiationEventArgs* eargs = (NegotiationEventArgs*)args;
    unsigned char option = eargs->GetOption();
    unsigned char* buff = eargs->GetData();
    unsigned int length = eargs->GetSize();

    // naws
    if (option == TELNET_NAWS)
    {
        unsigned char temp = 0;
        if (length != 4)
        {
            return;
        }
        temp = buff[0];
        buff[0] = buff[1];
        buff[1] = temp;
        temp = buff[2];
        buff[2] = buff[3];
        buff[3] = temp;

        _winsize.width = (*(unsigned short*)buff);
        _winsize.height = (*(unsigned short*)(buff + 2));
        return;
    }

    // termtype
    if (option == TELNET_TERMTYPE && length > 2 && buff[0] == TELNET_IS)
    {
        int i = 1;
        int curlength = length - 1;
        _termtype.clear();
        for (i = 1; i < curlength; ++i)
        {
            if (!isprint(buff[i]))
            {
                _termtype = "unknown";
                return;
            }
            _termtype += buff[i];
        }
        return;
    }

    return;
}

bool Socket::InitCompression()
{
    int ret = 0;
    cbuff = new unsigned char[MCCP_BUFFER_SIZE];
    zstream.zalloc = nullptr;
    zstream.zfree = nullptr;
    zstream.opaque = nullptr;
    zstream.next_in = nullptr;
    zstream.avail_in = 0;
    zstream.next_out = cbuff;
    zstream.avail_out = 4096;
    ret = deflateInit(&zstream, 9);
    if (ret != Z_OK)
    {
        return false;
    }

    _compressing = true;
    return true;
}

size_t Socket::Write(const unsigned char* data)
{
    size_t dlen = strlen((const char*)data);
    return Write((void*)const_cast<unsigned char*>(data), dlen);
}
size_t Socket::Write(const void* buffer, size_t count)
{
    int b = 0;
    int w = 0;
    int status = Z_OK;
    int length = 0;
    int i = 0;

    Flush();
    if (_compressing)
    {
        zstream.avail_in = count;
        zstream.next_in = (unsigned char*)const_cast<void*>(buffer);
        zstream.avail_out = 4096;
        zstream.next_out = cbuff;

        while (zstream.avail_in)
        {
            zstream.avail_out = 4096 - (zstream.next_out - cbuff);

            if (zstream.avail_out)
            {
                status = deflate(&zstream, Z_SYNC_FLUSH);
                if (status != Z_OK)
                {
                    return 0;
                }
            }
        }

        length = (zstream.next_out - cbuff);
        if (length > 0)
        {
            _compressedSent += length;
            b = 0;
            for (i = 0; i < length; i += b)
            {
                w = Min<int>(length - i, 4096);
                b = write(_control, (cbuff + i), w);
                if (b < 0)
                {
                    return 0;
                }
            }
        }
        return b;
    }
    else
    {
        return write(_control, buffer, count);
    }
}
void Socket::Write(const std::string& txt)
{
    BaseSocket::Write(txt);
}

bool Socket::Flush()
{
    int b = 0;
    int w = 0;
    int status = Z_OK;
    int length = 0;
    int i = 0;
    World* world = World::GetPtr();

    if (!_outBuffer.length())
    {
        return true;
    }

    _totalSent += _outBuffer.length();
    // prepend buffer to prompt
    if ((_mobile != nullptr) && (_con == CON_Game))
    {
        _outBuffer += "\r\n" + world->BuildPrompt(_mobile->GetPrompt(), _mobile) + (char)TELNET_IAC + (char)TELNET_GA;
    }

    if (!_compressing)
    {
        // we are not compressing outbound data.
        while (_outBuffer.length() > 0)
        {
            b = _outBuffer.length();
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
    } // end sending raw data
    else
    {
        // we are compressing, wheee!
        zstream.avail_in = _outBuffer.length();
        zstream.next_in = (unsigned char*)const_cast<char*>(_outBuffer.c_str());
        zstream.avail_out = 4096;
        zstream.next_out = cbuff;

        while (zstream.avail_in)
        {
            zstream.avail_out = 4096 - (zstream.next_out - cbuff);

            if (zstream.avail_out)
            {
                status = deflate(&zstream, Z_SYNC_FLUSH);
                if (status != Z_OK)
                {
                    return 0;
                }
            }
        }

        length = (zstream.next_out - cbuff);
        if (length > 0)
        {
            _compressedSent += length;
            for (i = 0; i < length; i += b)
            {
                w = Min<int>(length - i, 4096);
                b = write(_control, (cbuff + i), w);
                if (b < 0)
                {
                    return false;
                }
            }
        }
    }

    _outBuffer.clear();
    return true;
}
std::string Socket::GetInBuffer()
{
    // we need to clean up extra junk at the end
    if (_inBuffer == "")
    {
        return "";
    }
    return _inBuffer.substr(0, _inBuffer.find_first_of("\n\r"));
}

int Socket::GetConnectionType() const
{
    return _con;
}
void Socket::SetConnectionType(const int s)
{
    _con = s;
}

std::string Socket::GetHost() const
{
    return _host;
}
void Socket::SetHost(const std::string& host)
{
    _host = host;
}

void Socket::AllocatePlayer()
{
    if (!_mobile)
    {
        _mobile = new Player();
    }
}
Player* Socket::GetPlayer() const
{
    return _mobile;
}

void Socket::Kill()
{
    if (!_Close)
    {
        if (_mobile)
        {
            if (GetConnectionType() == CON_Game)
            {
                _mobile->LeaveGame();
            }
            delete _mobile;
            _mobile = nullptr;
        }
        _Close = true;
    }
}
void Socket::Close()
{
    _Close = true;
}

short Socket::GetPort() const
{
    return _port;
}
void Socket::SetPort(short port)
{
    _port = port;
}

unsigned int Socket::GetTotalReceived() const
{
    return _totalReceived;
}
unsigned int Socket::GetTotalSent() const
{
    return _totalSent;
}
unsigned int Socket::GetTotalCompressedSent() const
{
    return _compressedSent;
}
short Socket::GetWindowWidth() const
{
    return _winsize.width;
}
short Socket::GetWindowHeight() const
{
    return _winsize.height;
}
std::string Socket::GetTermtype() const
{
    return _termtype;
}
bool Socket::IsCompressing() const
{
    return _compressing;
}

bool Socket::ShouldClose()
{
    return (_Close ? true : false);
}

bool Socket::HasHandle() const
{
    return (_input.empty() == true ? false : true);
}
bool Socket::HandleInput()
{
    if (HasHandle())
    {
        in_data* data = _input.top();
        data->handle->Input(_input.top()->args, PopCommand());
        ClrInBuffer();
        return true;
    }

    return false;
}
void Socket::ClearInput()
{
    if (HasHandle())
    {
        in_data* in = _input.top();
        _input.pop();
        if (in)
        {
            if (in->handle)
            {
                delete in->handle;
            }
            delete in;
        }
        if (HasHandle())
        {
            in = _input.top();
            in->handle->Active(in);
        }
    }
}
bool Socket::SetInput(in_data* data)
{
    if (data->handle)
    {
        data->handle->Attach(this);
    }
    _input.push(data);
    data->handle->Active(data);
    ClrInBuffer();
    return true;
}

void Socket::UpdateLastCommand()
{
    _lastInput = time(nullptr);
}
time_t Socket::GetLastCommand()
{
    return _lastInput;
}
Player* Socket::GetMobile()
{
    return _mobile;
}
void Socket::ClearMobile()
{
    _mobile = nullptr;
}
bool Socket::CommandPending() const
{
    return (_cqueue.size() == 0 ? false : true);
}
std::string Socket::PopCommand()
{
    if (!CommandPending())
    {
        return "";
    }
    std::string ret = _cqueue.front();
    _cqueue.pop();

    return ret;
}
void Socket::AddCommand(const std::string& input)
{
    _cqueue.push(input);
}

bool Socket::HandleGameInput()
{
    World* world = World::GetPtr();
    Player* mob = GetPlayer();
    std::string input;

    // check to see if an input handler was associated with the socket before we pass to command parsing
    if (HasHandle())
    {
        HandleInput();
        return true;
    }

    // No handle was found, pass on to command parsing
    input = PopCommand();
    if (!input.length())
    {
        return true;
    }
    if (!world->DoCommand(mob, input))
    {
        mob->Message(MSG_ERROR, "I did not understand that.");
        return false;
    }

    return true;
}
bool Socket::HandleNameInput()
{
    Player* mob = nullptr;
    std::string input;
    World* world = World::GetPtr();
    Server* server = world->GetServer();
    input = PopCommand();

    // new player:
    if (input == "new")
    {
        Write("Welcome, what name would you like?\n");
        AllocatePlayer();
        mob = GetPlayer();
        SetConnectionType(CON_Newname);
        return true;
    }

    // checks to see if the username is valid
    if (!IsValidUserName(input))
    {
        Write("Invalid name, try again.\n");
        return true;
    }

    // check to see if the player exists
    if (!PlayerExists(input))
    {
        Write("That player doesn't exist.\nWhat is your name? Type new for a new character.\n");
        return true;
    }

    mob = server->GetLinkdeadUser(input);
    if (mob)
    {
        _mobile = mob;
        Write(TELNET_ECHO_OFF);
        Write("\n");
        Write("Password?\n");
        SetConnectionType(CON_Password);
        return true;
    }

    AllocatePlayer();
    mob = GetPlayer();
    // set the players name to the one specified and try to load the file.
    mob->SetName(input);
    mob->Load();

    Write(TELNET_ECHO_OFF);
    Write("\n");
    Write("Password?\n");
    SetConnectionType(CON_Password);
    return true;
}
bool Socket::HandlePasswordInput()
{
    World* world = World::GetPtr();
    PlayerManager& players = world->GetPlayerManager();
    Server* server = world->GetServer();
    Player* mobile = GetPlayer();
    Player* connected = nullptr;
    std::string input;

    input = PopCommand();
    mobile->SetTempPassword(input);

    if (!mobile->ComparePassword())
    {
        Write("That password isn't valid!\n");
        mobile->IncInvalidPassword();
        Close();
        return false;
    }

    Write(TELNET_ECHO_ON);
    SetConnectionType(CON_Game);

    // see if the player was already connected.
    connected = players.FindPlayer(mobile->GetName());
    if (connected)
    {
        connected->Message(MSG_INFO, "This player has been taken over from another connection...");
        connected->GetSocket()->Flush();
        connected->GetSocket()->SetConnectionType(CON_Disconnected);
        connected->GetSocket()->ClearMobile();
        connected->GetSocket()->Kill();
        connected->SetSocket(this);
        connected->Message(MSG_INFO, "Taking over connection.");
        delete _mobile;
        _mobile = connected;
        return true;
    }

    mobile->SetSocket(this);
    if (server->GetLinkdeadUser(mobile->GetName()))
    {
        server->RemoveLinkdeadUser(mobile->GetName());
        Write("Reconnected.\n");
        return true;
    }

    mobile->SetLastLogin((unsigned int)time(nullptr));
    mobile->EnterGame(false);
    return true;
}
bool Socket::HandleNewnameInput()
{
    std::string input;
    Player* mobile = GetPlayer();
    input = PopCommand();

    if (!IsValidUserName(input))
    {
        Write("That is not a valid username. Usernames must contain 4-12 characters.\nWhat name would you like?\n");
        return true;
    }
    if (PlayerExists(input))
    {
        Write("That player already exists, please try again.\nWhat name would you like?\n");
        return true;
    }

    Write("Please choose a password. Please make your password between 5 and 20 characters long.\nStrong passwords "
          "contain both lower and uppercase letters, numbers, letters and a dash ('-').\nEnter your new password?\n");
    Write(TELNET_ECHO_OFF);
    // name was valid, set it in the player class.
    mobile->SetName(input);
    SetConnectionType(CON_Newpass);
    return true;
}
bool Socket::HandleNewpassInput()
{
    std::string input;
    Player* mob = GetPlayer();

    input = PopCommand();
    if (!IsValidPassword(input))
    {
        Write("That password isn't valid, please try again.\n");
        return true;
    }

    // transfer control to password verification
    Write("Please re-enter your password for varification.\n");
    mob->SetPassword(input);
    SetConnectionType(CON_Verpass);

    return true;
}
bool Socket::HandleVerpassInput()
{
    Player* mob = GetPlayer();
    std::string input = PopCommand();

    if (!IsValidPassword(input))
    {
        Write("That password isn't valid, please try again.\n");
        return true;
    }

    mob->SetTempPassword(input);
    // passwords  did not match, transfer control back to new password.
    if (!mob->ComparePassword())
    {
        Write("That password isn't valid, please try again.\n");
        SetConnectionType(CON_Newpass);
        return true;
    }

    Write(TELNET_ECHO_ON);
    Write("What is your gender? Please enter male or female.\n");
    SetConnectionType(CON_Gender);

    return true;
}
bool Socket::HandleGenderInput()
{
    bool gf = false;
    Player* mob = GetPlayer();
    std::string input = PopCommand();
    Lower(input);

    if (input.length() >= 1)
    {
        if (input[0] == 'm')
        {
            mob->SetGender(Gender::Male);
            gf = true;
        }
        if (input[0] == 'f')
        {
            mob->SetGender(Gender::Female);
            gf = true;
        }
    }

    if (gf)
    {
        InitializeNewPlayer();
    }

    return true;
}

void Socket::InitializeNewPlayer()
{
    Player* mob = GetPlayer();
    World* world = World::GetPtr();

    mob->Initialize();
    // passwords matched, see if the player is the first user. If so, make it a god.
    if (IsFirstUser())
    {
        mob->SetRank(RANK_PLAYER | RANK_BUILDER | RANK_ADMIN | RANK_GOD);
        Write("You are the first player to create, rank set to God.\n");
    }

    mob->SetFirstLogin((unsigned int)time(nullptr));
    // Set the connection type to game and enter the player.
    SetConnectionType(CON_Game);
    mob->SetSocket(this);
    // call the Create event:
    mob->EnterGame(false);
    world->events.CallEvent("PlayerCreated", nullptr, (void*)mob);
}

bool Socket::HandleCommand()
{
    switch (GetConnectionType())
    {
            // handles all in-game connections that aren't handled with a function
        case CON_Disconnected:
        case CON_Game:
            return HandleGameInput();
        case CON_Name:
            return HandleNameInput();
            // login password prompt
        case CON_Password:
            return HandlePasswordInput();
            // login new username
        case CON_Newname:
            return HandleNewnameInput();
        // login new password
        case CON_Newpass:
            return HandleNewpassInput();
            // login verify password
        case CON_Verpass:
            return HandleVerpassInput();
        case CON_Gender:
            return HandleGenderInput();
        default:
            return false;
    }

    return true;
}
