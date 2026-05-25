#include "utils.h"

#include "editor.h"
#include "extensions/utils.h"
#include "uuid.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <list>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#include <ctype.h>
#include <dirent.h>
#include <strings.h>

#include <sys/stat.h>
#include <sys/time.h>

#include <openssl/sha.h>

int tonum(const char* str)
{
    const char* ptr = str;
    int ret = 0;

    while (*ptr)
    {
        if (!isdigit(*ptr))
        {
            throw(std::bad_cast());
        }
        ++ptr;
    }

    ret = atoi(str);
    return ret;
}

bool isnum(const char* str)
{
    try
    {
        tonum(str);
    }
    catch (std::bad_cast e)
    {
        return false;
    }
    return true;
}
bool FileExists(const std::string& name)
{
    struct stat info;
    int res = stat(name.c_str(), &info);
    if (res == 0)
    {
        return true;
    }
    return false;
}
bool DirectoryExists(const std::string& path)
{
    struct stat info;

    int res = stat(path.c_str(), &info);
    if (res == 0 && S_ISDIR(info.st_mode))
    {
        return true;
    }
    return false;
}

unsigned int GetFileSize(int fd)
{
    struct stat info;
    int res = fstat(fd, &info);
    if (res)
    {
        return -1;
    }
    else
    {
        return info.st_size;
    }
}
unsigned int GetFileSize(const char* path)
{
    struct stat info;
    int res = stat(path, &info);
    if (res)
    {
        return -1;
    }
    else
    {
        return info.st_size;
    }
}
unsigned int GetFileSize(const std::string& path)
{
    return GetFileSize(path.c_str());
}

void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& del)
{
    std::string::size_type lpos = str.find_first_not_of(del, 0);
    std::string::size_type pos = str.find_first_of(del, lpos);
    if (pos == std::string::npos)
    {
        tokens.push_back(str);
        return;
    }
    // our loop for tokenizing:
    while ((std::string::npos != pos) || (std::string::npos != lpos))
    {
        tokens.push_back(str.substr(lpos, (pos - lpos)));
        lpos = str.find_first_not_of(del, pos);
        pos = str.find_first_of(del, lpos);
    }
}

void SplitToVector(const std::string& line, std::vector<std::string>* output, int cols)
{
    if (line.length() <= (unsigned int)cols)
    {
        output->push_back(line);
        return;
    }

    size_t start, length, cur;
    length = line.length();
    cur = start = 0;

    while (1)
    {
        cur = start + cols;                // move the pointer to the maximum length from start.
        if ((int)(length - start) <= cols) // the rest of the line is less than cols, so we don't need to split.
        {
            output->push_back(line.substr(start, length));
            return;
        }

        for (;; --cur) // we decrement cur until we find a space:
        {
            if (line[cur] == ' ') // we found a space in our search backwards.
            {
                output->push_back(line.substr(start, cur - start)); // pushes the substr between start and cur.
                start = cur + 1;                                    // moves start to cur+1, which is after the space.
                break;
            }
        }
    }
}

std::string TimevalToString(struct timeval* tv)
{
    std::stringstream st;
    int sec = tv->tv_sec;
    int hour = 0, minute = 0, second = 0;
    bool found = false;
    float rem = 0.0F;

    if (sec >= 3600) // 3600 seconds in an hour
    {
        hour = sec / 3600;
        sec %= 3600;
    }
    if (sec > 60)
    {
        minute = sec / 60;
        sec %= 60;
    }
    second = sec;
    rem = (float)(tv->tv_usec / 1000) / 1000.0F;

    if (hour)
    {
        st << hour;
        found = true;
    }
    if (minute)
    {
        if (found)
            st << ":";
        st << minute;
        found = true;
    }
    if (second)
    {
        if (found)
            st << ":";
        st << second;
        found = true;
    }
    if (rem)
    {
        if (found)
            st << ":";
        st << rem;
    }

    return st.str();
}

bool IsValidUserName(const std::string& input)
{
    const char* ptr = input.c_str();
    if ((input.length() < 4) || (input.length() > 11))
    {
        return false;
    }
    while (*ptr)
    {
        if (!isalpha(*ptr))
        {
            return false;
        }
        ptr++;
    }
    return true;
}

bool IsValidPassword(const std::string& input)
{
    std::string::const_iterator it, itEnd;

    if ((input.length() < MIN_PASSWORD) || (input.length() > MAX_PASSWORD))
    {
        return false;
    }

    itEnd = input.end();
    for (it = input.begin(); it != itEnd; ++it)
    {
        if (!isgraph((*it)))
        {
            return false;
        }
    }

    return true;
}

bool PlayerExists(const std::string& name)
{
    return FileExists(PLAYER_DIR + name + ".json");
}

bool IsFirstUser()
{
    int count = 0;
    std::string path = (PLAYER_DIR);
    DIR* search = opendir(path.c_str());
    dirent* dir = nullptr;

    if (!search)
    {
        return false;
    }

    while (1)
    {
        dir = readdir(search);
        if (!dir)
        {
            break;
        }
        if (dir->d_name[0] == '.')
        {
            continue;
        }
        else
        {
            count = 1;
            break;
        }
    }

    closedir(search);
    return (count == 1 ? false : true);
}

void Lower(std::string& str)
{
    int i;
    int length = str.length();
    for (i = 0; i < length; i++)
        str[i] = tolower(str[i]);
}

std::string Capitalize(const std::string& str)
{
    std::string ret = str;
    if (str.length() >= 1)
    {
        ret[0] = toupper(str[0]);
    }
    return ret;
}

std::string Repeat(const std::string& filler, const int count)
{
    std::string ret;
    int i;
    for (i = 0; i < count; i++)
    {
        ret += filler;
    }
    return ret;
}
std::string Repeat(const char filler, const int count)
{
    std::string temp;
    temp += filler;
    return Repeat(temp, count);
}


std::string Center(const std::string& str, const int width)
{
    std::string temp;
    int left = 0;
    int right = 0;
    int length = (int)str.length();
    int result = 0;

    // the length was greater than the width, no centering possible:
    if (length >= width)
    {
        return str;
    }
    // calculate how many "filler"chars will be needed
    // If there is a remainder of one, it will add the extra one space to the left side
    result = (width - length) / 2;
    right = result;
    left = (result / 2 ? result + 1 : result);
    // remainder of 1.
    temp = Repeat(" ", left);
    temp += str;
    temp += Repeat(" ", right);

    return temp;
}

std::string CenterLines(const std::string& str, const int width)
{
    const char* blankspace = " ";
    std::string lines, temp, temp1;
    int leftfiller;
    int totalfiller;

    lines = str;
    if (str.empty())
    {
        return "";
    }

    if ((int)str.length() < (width - (int)str.length()))
    {
        return Center(str, width);
    }
    std::string::iterator it = lines.begin(), itEnd = lines.end();
    for (; it != itEnd; ++it)
    {
        temp1 += (*it);
        if (((int)temp1.length()) >= width - ((int)temp1.length()) && (*it) == *blankspace)
        {
            temp += Center(temp1, width);
            totalfiller = ((width - (int)temp1.length()) / 2);
            leftfiller = (totalfiller / 2 ? totalfiller - 1 : totalfiller);
            temp1.clear();

            continue;
        }
    }
    temp += Repeat(" ", leftfiller);
    temp += temp1;
    return temp;
}

std::string Explode(std::vector<std::string>& parts, const std::string& del)
{
    std::vector<std::string>::iterator it, itEnd;
    std::stringstream st;

    itEnd = parts.end();
    it = parts.begin();
    // we need to handle the first element before the loop so we don't have a space in the beginning.
    if (it != itEnd)
    {
        st << (*it);
        ++it;
    }
    for (; it != itEnd; ++it)
    {
        st << del << (*it);
    }

    return st.str();
}
std::string Explode(const std::vector<std::string>& parts, std::vector<std::string>::const_iterator it,
                    const std::string& del)
{
    std::vector<std::string>::const_iterator itEnd;
    std::stringstream st;

    itEnd = parts.end();
    if (it != itEnd)
    {
        st << (*it);
        ++it;
    }
    for (; it != itEnd; ++it)
    {
        st << del << (*it);
    }

    return st.str();
}

std::string StripWhitespace(const std::string& str)
{
    if (str.find_first_of(" \r\n") == std::string::npos)
    {
        return str;
    }
    else
    {
        return (str.substr(str.find_first_of(" \r\n")));
    }
}
void NumberToString(char* buffer, int number)
{
    sprintf(buffer, "%d", number);
}

std::string Columnize(std::vector<std::string>* data, int cols, std::vector<std::string>* headers, int width)
{
    int cwidth = (width / cols);
    int count = (int)data->size();
    std::vector<std::string>::iterator it, itEnd;
    int i = 0;
    int j = 0;
    std::stringstream st;

    if (!count)
    {
        return "";
    }
    if (headers && (int)headers->size() != cols)
    {
        return "";
    }
    if (headers)
    {
        itEnd = headers->end();
        for (it = headers->begin(); it != headers->end(); ++it)
        {
            int res = cwidth - (*it).length();
            st << (*it);
            for (j = 0; j < res; j++)
            {
                st << " ";
            }
        }
    }
    st << "\n" << Repeat("-", width) << "\n";

    itEnd = data->end();
    for (it = data->begin(), i = 1; it != itEnd; ++it, ++i)
    {
        if ((int)(*it).length() <= cwidth)
        {
            int res = (cwidth - (*it).length());
            st << (*it);
            for (j = 0; j < res; j++)
            {
                st << " ";
            }
        }
        if (i == cols)
        {
            st << "\n";
            i = 0;
        }
    }
    return (st.str());
}

int Percentage(int total, int count, int round)
{
    float temp = (float)total / (float)count;
    temp *= 100.0F;

    if (round == 1)
    {
        temp = ceil(temp);
    }
    else
    {
        temp = floor(temp);
    }

    return (int)temp;
}
FLAG BitSet(FLAG flag, int pos)
{
    return flag |= pos;
}
FLAG BitClear(FLAG flag, int pos)
{
    return flag &= ~pos;
}
bool BitIsSet(FLAG flag, int pos)
{
    return (flag & pos) == pos;
}

std::string GenerateUuid()
{
    Uuid id;
    id.Initialize();
    return id.ToString();
}

std::string EnglishList(std::vector<std::string>* in)
{
    std::vector<std::string>::iterator it;
    std::string str = "";

    if (!in->size())
    {
        return "";
    }
    if (in->size() == 1)
    {
        return (*in->begin());
    }

    for (it = in->begin(); it != in->end(); it++)
    {
        if ((*it) == in->back())
        {
            str += "and " + (*it);
            break;
        }
        str += (*it) + ", ";
    }

    return str;
}

int Roll(int n, int s)
{
    int result = 0;
    int i;

    if (n <= 0)
    {
        return 0;
    }
    if (s <= 0)
    {
        return 0;
    }

    for (i = 0; i < n; i++)
    {
        result += (rand() % (s + 1));
    }

    return result;
}
int Roll(const Dice& d)
{
    return Roll(d.num, d.sides);
}

std::string SwapExit(const std::string& exit)
{
    if ((exit == "n") || (exit == "north"))
    {
        return "south";
    }
    else if ((exit == "s") || (exit == "south"))
    {
        return "north";
    }
    else if ((exit == "e") || (exit == "east"))
    {
        return "west";
    }
    else if ((exit == "w") || (exit == "west"))
    {
        return "east";
    }
    else if ((exit == "ne") || (exit == "northeast"))
    {
        return "southwest";
    }
    else if ((exit == "nw") || (exit == "northwest"))
    {
        return "southeast";
    }
    else if ((exit == "se") || (exit == "southeast"))
    {
        return "northwest";
    }
    else if ((exit == "sw") || (exit == "southwest"))
    {
        return "northeast";
    }
    else if ((exit == "u") || (exit == "up"))
    {
        return "down";
    }
    else if ((exit == "d") || (exit == "down"))
    {
        return "up";
    }
    else
    {
        return "";
    }
}

bool IsValidExit(const std::string& name)
{
    if ((name == "north" || name == "n") || (name == "south" || name == "s") || (name == "east" || name == "e") ||
        (name == "west" || name == "w") || (name == "northeast" || name == "ne") ||
        (name == "northwest" || name == "nw") || (name == "southeast" || name == "se") ||
        (name == "southwest" || name == "sw"))
    {
        return true;
    }
    else
    {
        return false;
    }
}
ExitDirection GetDirectionByName(const std::string& name)
{
    if (name == "north" || name == "n")
        return north;
    else if (name == "south" || name == "s")
        return south;
    else if (name == "east" || name == "e")
        return east;
    else if (name == "west" || name == "w")
        return west;
    else if (name == "northeast" || name == "ne")
        return northeast;
    else if (name == "northwest" || name == "nw")
        return northwest;
    else if (name == "southwest" || name == "sw")
        return southwest;
    else if (name == "southeast" || name == "se")
        return southeast;
    else
        return nowhere;
}

void TimeInfo::Initialize()
{
    hour = 0;
    minute = 0;
    second = 0;
}

TimeInfo::TimeInfo(time_t data)
{
    Initialize();
    Calculate(data);
}
TimeInfo::TimeInfo(unsigned int data)
{
    Initialize();
    Calculate((time_t)data);
}
TimeInfo::TimeInfo()
{
    Initialize();
}

void TimeInfo::Calculate(time_t data)
{
    unsigned int count = data;
    hour = count / 3600;
    count %= 3600;
    minute = count / 60;
    count %= 60;
    second = count;
}

std::string TimeInfo::ToString()
{
    std::stringstream st;
    if (hour)
    {
        st << hour << ":";
    }
    if (minute)
    {
        st << minute << ":";
    }
    st << second;
    return st.str();
}

point::point()
{
    x = y = z = 0;
}
point::point(int X, int Y, int Z)
{
    x = X;
    y = Y;
    z = Z;
}

bool point::operator==(const point& p)
{
    return (p.x == x && p.y == y && p.z == z);
}

point& point::operator=(point& p)
{
    x = p.x;
    y = p.y;
    z = p.z;
    return *this;
}

// Static random number generator - initialized once for performance
static std::mt19937& GetRNG()
{
    static std::random_device rd;
    static std::mt19937 rgen(rd());
    return rgen;
}

int RandomPercent()
{
    std::uniform_int_distribution<int> dist(1, 100);
    return dist(GetRNG());
}
int RandomRange(int bottom, int top)
{
    std::uniform_int_distribution<int> dist(bottom, top);
    return dist(GetRNG());
}

bool iequals(const std::string& a, const std::string& b)
{
    return (strcasecmp(a.c_str(), b.c_str()) == 0 ? true : false);
}

std::string GetPositionString(unsigned int position)
{
    switch (position)
    {
        case POSITION_ANY:
            return "<any>";
        case POSITION_UNCONCIOUS:
            return "unconcious";
        case POSITION_SLEEPING:
            return "sleeping";
        case POSITION_SITTING:
            return "sitting";
        case POSITION_STANDING:
            return "standing";
        default:
            return GetExtendedPositionString(position);
    }
}

std::string GetCommandTypeName(CommandType type)
{
    switch (type)
    {
        case CommandType::Misc:
            return "miscellaneous";
        case CommandType::Builder:
            return "builder";
        case CommandType::Admin:
            return "admin";
        case CommandType::God:
            return "god";
        case CommandType::Information:
            return "information";
        case CommandType::Object:
            return "object";
        case CommandType::Movement:
            return "movement";
        case CommandType::Script:
            return "script";
        case CommandType::Social:
            return "social";
        case CommandType::Channel:
            return "channel";
        case CommandType::Communication:
            return "communication";
        default:
            return "unknown";
    }
}

std::string Sha256Hash(const std::string& str)
{
    int i = 0;
    std::stringstream st;
    unsigned char digest[SHA256_DIGEST_LENGTH + 1];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.length());
    SHA256_Final(digest, &sha256);
    for (i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        st << std::hex << (int)digest[i];
    }
    return st.str();
}
