#include "banList.h"

#include "conf.h"
#include "jsonSerializer.h"
#include "mud.h"

#include <string>

#include <arpa/inet.h>

bool BanList::AddAddress(const std::string& address)
{
    in_addr addr;

    if (!inet_aton(address.c_str(), &addr))
    {
        return false;
    }
    if (AddressExists(addr.s_addr))
    {
        return false;
    }

    _addresses.push_back(addr.s_addr);
    return true;
}
bool BanList::RemoveAddress(const std::string& address)
{
    in_addr addr;

    if (inet_aton(address.c_str(), &addr))
    {
        return false;
    }
    auto end = _addresses.end();
    for (auto it = _addresses.begin(); it != end; ++it)
    {
        if ((*it) == addr.s_addr)
        {
            _addresses.erase(it);
            return true;
        }
    }

    return false;
}
bool BanList::AddressExists(const std::string& address) const
{
    in_addr addr;

    if (inet_aton(address.c_str(), &addr))
    {
        return false;
    }

    for (auto it : _addresses)
    {
        if (it == addr.s_addr)
        {
            return true;
        }
    }

    return false;
}
bool BanList::AddressExists(unsigned long address) const
{
    for (auto it : _addresses)
    {
        if (it == address)
        {
            return true;
        }
    }

    return false;
}
void BanList::ListAddresses(std::vector<std::string>& addresses)
{
    in_addr addr;
    std::string val;

    for (auto it : _addresses)
    {
        addr.s_addr = it;
        val = inet_ntoa(addr);
        addresses.push_back(val);
    }
}

void BanList::ToJson(Json::Value& json) const
{
    Json::Value addressesArray(Json::arrayValue);
    in_addr addr;

    for (auto it : _addresses)
    {
        addr.s_addr = it;
        std::string printable = inet_ntoa(addr);
        addressesArray.append(printable);
    }

    json["addresses"] = addressesArray;
}

void BanList::FromJson(const Json::Value& json, int version)
{
    _addresses.clear();

    if (json.isMember("addresses") && json["addresses"].isArray())
    {
        const Json::Value& addressesArray = json["addresses"];
        for (const auto& addrJson : addressesArray)
        {
            if (addrJson.isString())
            {
                AddAddress(addrJson.asString());
            }
        }
    }
}
