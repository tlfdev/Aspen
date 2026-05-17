#include <string>
#include "mud.h"
#include "conf.h"
#include "telnetParser.h"
#include "telnetParserEvents.hpp"
#include "telnet.h"
#include "eventManager.h"
#include "event.h"

TelnetParser::TelnetParser()
{
    _buff = nullptr;
    _newbuff = nullptr;
    Reset();
    events.RegisterEvent("OnNegotiation");
    events.RegisterEvent("OnOption");
}
TelnetParser::~TelnetParser()
{
    if (_newbuff != nullptr)
        {
            delete []_newbuff;
        }
}

void TelnetParser::SetBuffer(unsigned char* buff)
{
    _buff = buff;
}
unsigned char* TelnetParser::GetFinalBuffer() const
{
    return _newbuff;
}
unsigned int TelnetParser::GetFinalSize() const
{
    return _newsize;
}
void TelnetParser::Parse()
{
    if (_buff == nullptr)
        {
            return;
        }

    unsigned char* r = _buff;
    unsigned char* w = _newbuff;
    unsigned char cur;
    unsigned char option = 0;
    unsigned char* start = nullptr;
    unsigned char* end = nullptr;

    for (unsigned int counter = 0; counter < _size; ++counter)
        {
            cur = _buff[counter];
//possible: iac iac or iac option.
            if (cur == TELNET_IAC)
                {
//beginning of sequence or end of negotiation
                    if (_state == 0)
                        {
                            _state = TELNET_IAC;
                            continue;
                        }
                    if (_state == TELNET_IAC)
                        {
                            *w = TELNET_IAC;
                            ++w;
                            _state = 0;
                            continue;
                        }
                }
//option negotiation
            else if (_state == TELNET_IAC)
                {
                    if (cur == TELNET_DO || cur == TELNET_DONT || cur == TELNET_WILL || cur == TELNET_WONT)
                        {
                            _state = cur;
                            continue;
                        }
//sequence begin
                    else if(cur == TELNET_SB && _state == TELNET_IAC)
                        {
                            _state = TELNET_SB;
                            continue;
                        }
//sequence end
                    else if(cur == TELNET_SE && start != nullptr && option)
                        {
                            NegotiationEventArgs args(option, start, (unsigned int)(end-start)+1);
                            events.CallEvent("OnNegotiation", &args, static_cast<void*>(this));
                            _state = 0;
                            option = 0;
                            start = end = nullptr;
                            continue;
                        }
                }
//option
//we've already seen will|wont|do|dont.
            else if (_state == TELNET_WILL || _state == TELNET_WONT || _state == TELNET_DO || _state == TELNET_DONT)
                {
                    OptionEventArgs args(cur, _state);
                    events.CallEvent("OnOption", &args, static_cast<void*>(this));
                    _state = 0;
                    continue;
                }
            else if (_state == TELNET_SB && !option)
                {
                    option = cur;
                    start = r+counter;
                    end = start;
                    for (; counter < _size; ++ counter, ++end)
                        {
                            cur = _buff[counter];
                            if (cur == TELNET_IAC)
                                {
                                    _state = TELNET_IAC;
                                    break;
                                }
                        }
                    start++; //move it past option.
                    end--; //move it to the last bite of the sequence.
                    continue;
                }
            else
                {
                    *w = cur;
                    w++;
                }
        }
    _newsize = (unsigned int)(w-_newbuff);
}

void TelnetParser::Reset()
{
    _state = 0;
    _size = 0;
    _newsize = 0;
    _buff = nullptr;
    if (_newbuff)
        {
            delete []_newbuff;
        }
    _newbuff = nullptr;
}
void TelnetParser::Initialize(unsigned int size)
{
    if (_newbuff)
        {
            delete []_newbuff;
        }
    _newbuff = new unsigned char[size];
    _size = size;
}
