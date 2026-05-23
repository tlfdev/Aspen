#include "variant.h"

#include "exception.h"
#include "world.h"

#include <tinyxml2.h>

Variant::Variant()
{
    type = VAR_EMPTY;
    i32 = 0;
}
Variant::Variant(const Variant& var)
{
    type = var.Typeof();
    switch (type)
    {
        case VAR_BYTE:
            byte = var.GetByte();
            break;
        case VAR_INT:
            i32 = var.GetInt();
            break;
        case VAR_DOUBLE:
            d = var.GetDouble();
            break;
        case VAR_STR:
            str = var.GetStr();
            ;
            break;
        case VAR_EMPTY:
        default:
            break;
    }
}
Variant::Variant(int s)
{
    i32 = s;
    type = VAR_INT;
}
Variant::Variant(char s)
{
    byte = s;
    type = VAR_BYTE;
}
Variant::Variant(const std::string& s)
{
    str = s;
    type = VAR_STR;
}
Variant::Variant(const char* s)
{
    str = s;
    type = VAR_STR;
}
Variant::Variant(double s)
{
    d = s;
    type = VAR_DOUBLE;
}

VARIABLE_TYPE Variant::Typeof() const
{
    return type;
}

int Variant::GetInt() const
{
    return i32;
}
char Variant::GetByte() const
{
    return byte;
}
std::string Variant::GetStr() const
{
    return str;
}
double Variant::GetDouble() const
{
    return d;
}

void Variant::SetInt(int s)
{
    i32 = s;
    type = VAR_INT;
}
void Variant::SetByte(char s)
{
    byte = s;
    type = VAR_BYTE;
}
void Variant::SetDouble(double s)
{
    d = s;
    type = VAR_DOUBLE;
}
void Variant::SetStr(const std::string& s)
{
    str = s;
    type = VAR_STR;
}
void Variant::SetStr(const char* s)
{
    str = s;
    type = VAR_STR;
}

bool Variant::IsEmpty() const
{
    return (type == VAR_EMPTY);
}
bool Variant::IsInt() const
{
    return (type == VAR_INT);
}
bool Variant::IsByte() const
{
    return (type == VAR_BYTE);
}
bool Variant::IsDouble() const
{
    return (type == VAR_DOUBLE);
}
bool Variant::IsNumber() const
{
    switch (type)
    {
        case VAR_INT:
        case VAR_DOUBLE:
            return true;
        default:
            return false;
    }
}
bool Variant::IsChar() const
{
    return (type == VAR_BYTE ? true : false);
}
bool Variant::IsString() const
{
    return (type == VAR_STR);
}

bool Variant::Compare(const Variant& var) const
{
    VARIABLE_TYPE vtype = var.Typeof();
    switch (type)
    {
        case VAR_STR:
            switch (vtype)
            {
                case VAR_STR:
                    return (str == var.GetStr() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare a string with an illegal value."));
            }
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return (i32 == var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (i32 == (int)var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (i32 == var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare integer with an illegal type."));
            }
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_INT:
                    return ((int)d == var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return d == var.GetDouble();
                case VAR_BYTE:
                    return ((char)d == var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare double with an illegal type."));
            }
        case VAR_BYTE:
            switch (vtype)
            {
                case VAR_INT:
                    return (byte == var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (byte == (char)var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (byte == var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare byte with an illegal type."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to compare with an illegal type."));
    }

    return false;
}
bool Variant::operator==(const Variant& var)
{
    return Compare(var);
}
bool Variant::operator!=(const Variant& var)
{
    return (Compare(var) == 1 ? 0 : 1);
}

Variant& Variant::operator=(int s)
{
    type = VAR_INT;
    i32 = s;
    return (*this);
}
Variant& Variant::operator=(char s)
{
    type = VAR_BYTE;
    byte = s;
    return (*this);
}
Variant& Variant::operator=(double s)
{
    type = VAR_DOUBLE;
    d = s;
    return (*this);
}
Variant& Variant::operator=(std::string s)
{
    type = VAR_STR;
    str = s;
    return (*this);
}

void Variant::SetType(VARIABLE_TYPE t)
{
    type = t;
}

// JSON serialization
void Variant::ToJson(Json::Value& json) const
{
    json["type"] = static_cast<int>(type);

    switch (type)
    {
        case VAR_INT:
            json["value"] = i32;
            break;
        case VAR_BYTE:
            json["value"] = static_cast<int>(byte);
            break;
        case VAR_STR:
            json["value"] = str;
            break;
        case VAR_DOUBLE:
            json["value"] = d;
            break;
        case VAR_EMPTY:
            json["value"] = Json::Value::null;
            break;
        default:
            break;
    }
}

void Variant::FromJson(const Json::Value& json, int version)
{
    if (json.isMember("type"))
    {
        type = static_cast<VARIABLE_TYPE>(json["type"].asInt());
    }
    else
    {
        type = VAR_EMPTY;
    }

    if (!json.isMember("value"))
    {
        return;
    }

    const Json::Value& value = json["value"];

    switch (type)
    {
        case VAR_INT:
            i32 = value.isInt() ? value.asInt() : 0;
            break;
        case VAR_BYTE:
            byte = value.isInt() ? static_cast<char>(value.asInt()) : 0;
            break;
        case VAR_STR:
            str = value.isString() ? value.asString() : "";
            break;
        case VAR_DOUBLE:
            d = value.isDouble() ? value.asDouble() : 0.0;
            break;
        case VAR_EMPTY:
        default:
            break;
    }
}

// math operator overloads

Variant& Variant::operator++()
{
    switch (type)
    {
        default:
            throw(InvalidVariableTypeException("Tried to use \'++\' on a variable that isn't a number."));
            break;
        case VAR_INT:
            i32++;
            break;
    }
    return (*this);
}

Variant& Variant::operator--()
{
    switch (type)
    {
        default:
            throw(InvalidVariableTypeException("Tried to use \'--\' on a variable that isn't a number."));
            break;
        case VAR_INT:
            i32--;
            break;
    }
    return (*this);
}

Variant Variant::operator+(Variant var)
{
    VARIABLE_TYPE vtype = var.Typeof();

    switch (type)
    {
            // string
        case VAR_STR:
            switch (vtype)
            {
                case VAR_STR:
                    return Variant(str + var.GetStr());
                case VAR_BYTE:
                    return Variant(str + var.GetByte());
                case VAR_INT:
                    return Variant(str + std::to_string(var.GetInt()));
                case VAR_DOUBLE:
                    return Variant(str + std::to_string(var.GetDouble()));
                default:
                    throw(VariableEmptyException("Tried to add an empty variant to another variant."));
            }
            // integer
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(i32 + var.GetInt());
                case VAR_DOUBLE:
                    return Variant(i32 + var.GetDouble());
                case VAR_BYTE:
                    return Variant(i32 + var.GetByte());
                case VAR_STR:
                    throw(InvalidVariableTypeException("Tried to add a string to an integer."));
                default:
                    throw(VariableEmptyException("Tried to add an empty variant to another variant."));
            }
            // double
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_DOUBLE:
                    return Variant(d + var.GetDouble());
                case VAR_INT:
                    return Variant(d + var.GetInt());
                case VAR_BYTE:
                    throw(InvalidVariableTypeException("Tried to add byte to double."));
                case VAR_STR:
                    throw(InvalidVariableTypeException("Tried to add a string to a double."));
                default:
                    throw(VariableEmptyException("Tried to add an empty variant to another variant."));
            }
            // byte
        case VAR_BYTE:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(byte + var.GetInt());
                case VAR_BYTE:
                    return Variant(byte + var.GetByte());
                default:
                    throw(InvalidVariableTypeException("Tried to add byte to an illegal value."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to add empty variable."));
    }
}
Variant& Variant::operator+=(Variant var)
{
    *this = (*this) + var;
    return (*this);
}

Variant Variant::operator-(Variant var)
{
    VARIABLE_TYPE vtype = var.Typeof();

    switch (type)
    {
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(i32 - var.GetInt());
                case VAR_DOUBLE:
                    return Variant(i32 - var.GetDouble());
                default:
                    throw(InvalidVariableTypeException("Tried to subtract illegal value."));
            }
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(d - var.GetInt());
                case VAR_DOUBLE:
                    return Variant(d - var.GetDouble());
                default:
                    throw(InvalidVariableTypeException("Tried to subtract illegal value."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to subtract illegal value."));
    }
}
Variant& Variant::operator-=(Variant var)
{
    *this = (*this) - var;
    return *this;
}

Variant Variant::operator*(Variant var)
{
    VARIABLE_TYPE vtype = var.Typeof();

    switch (type)
    {
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(i32 * var.GetInt());
                case VAR_DOUBLE:
                    return Variant(i32 * var.GetDouble());
                default:
                    throw(InvalidVariableTypeException("Tried to multiply illegal value."));
            }
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(d * var.GetInt());
                case VAR_DOUBLE:
                    return Variant(d * var.GetDouble());
                default:
                    throw(InvalidVariableTypeException("Tried to multiply illegal value."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to multiply illegal value."));
    }
}
Variant& Variant::operator*=(Variant var)
{
    (*this) = (*this) * var;
    return (*this);
}

Variant Variant::operator/(Variant var)
{
    VARIABLE_TYPE vtype = var.Typeof();

    switch (type)
    {
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(i32 / var.GetInt());
                case VAR_DOUBLE:
                    return Variant(i32 / var.GetDouble());
                default:
                    throw(InvalidVariableTypeException("Tried to divide illegal value."));
            }
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_INT:
                    return Variant(d / var.GetInt());
                case VAR_DOUBLE:
                    return Variant(d / var.GetDouble());
                default:
                    throw(InvalidVariableTypeException("Tried to divide illegal value."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to divide illegal value."));
    }
}
Variant& Variant::operator/=(Variant var)
{
    *this = (*this) / var;
    return *this;
}

Variant Variant::operator%(Variant var)
{
    if ((var.Typeof() == VAR_INT) && (type == VAR_INT))
    {
        return Variant(i32 % var.GetInt());
    }
    else
    {
        throw(InvalidVariableTypeException("Tried to get modulous of illegal value."));
    }
}
Variant& Variant::operator%=(Variant var)
{
    *this = (*this) % var;
    return *this;
}

bool Variant::operator<(Variant var)
{
    VARIABLE_TYPE vtype = var.Typeof();
    switch (type)
    {
        case VAR_STR:
            switch (vtype)
            {
                case VAR_STR:
                    return (str < var.GetStr() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare a string with an illegal value."));
            }
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return (i32 < var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (i32 < var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (i32 < var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare integer with an illegal type."));
            }
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_INT:
                    return (d < var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (d < var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (d < var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare double with an illegal type."));
            }
        case VAR_BYTE:
            switch (vtype)
            {
                case VAR_INT:
                    return (byte < var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (byte < var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (byte < var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare byte with an illegal type."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to compare with an illegal type."));
    }
}
bool Variant::operator<=(Variant var)
{
    return (((*this) == var) || ((*this) < var)) ? true : false;
}

bool Variant::operator>(Variant var)
{
    VARIABLE_TYPE vtype = var.Typeof();
    switch (type)
    {
        case VAR_STR:
            switch (vtype)
            {
                case VAR_STR:
                    return (str > var.GetStr() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare a string with an illegal value."));
            }
        case VAR_INT:
            switch (vtype)
            {
                case VAR_INT:
                    return (i32 > var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (i32 > var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (i32 > var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare integer with an illegal type."));
            }
        case VAR_DOUBLE:
            switch (vtype)
            {
                case VAR_INT:
                    return (d > var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (d > var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (d > var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare double with an illegal type."));
            }
        case VAR_BYTE:
            switch (vtype)
            {
                case VAR_INT:
                    return (byte > var.GetInt() ? 1 : 0);
                case VAR_DOUBLE:
                    return (byte > var.GetDouble() ? 1 : 0);
                case VAR_BYTE:
                    return (byte > var.GetByte() ? 1 : 0);
                default:
                    throw(InvalidVariableTypeException("Tried to compare byte with an illegal type."));
            }
        default:
            throw(InvalidVariableTypeException("Tried to compare with an illegal type."));
    }
}

bool Variant::operator>=(Variant var)
{
    return (((*this) == var) || ((*this) > var)) ? true : false;
}
