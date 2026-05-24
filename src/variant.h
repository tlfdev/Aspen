/*
 *This class is used to hold variables.
 *It allows the user to specify the type for the variable.
 *Also allows for serialization and deserialization, so they can be stored.
 */
#ifndef VARIANT_H
#define VARIANT_H
#include "conf.h"
#include "exception.h"
#include "mud.h"

#include <json/json.h>

// exceptions
/*
 *Used for operations on an empty variable.
 */
class VariableEmptyException : public Exception
{
  public:
    VariableEmptyException(const std::string& msg) : Exception(msg) {}
};
/*
 *Used when operations are performed on an invalid variable type.
 *For example: var++, when var is a string.
 */
class InvalidVariableTypeException : public Exception
{
  public:
    InvalidVariableTypeException(const std::string& msg) : Exception(msg) {}
};

// variable types
enum VARIABLE_TYPE
{
    VAR_EMPTY,
    VAR_BYTE,
    VAR_INT,
    VAR_DOUBLE,
    VAR_STR
};

class Variant
{
    union
    {
        int i32;
        char byte;
        double d;
    };
    std::string str;
    VARIABLE_TYPE type;

  public:
    Variant();
    Variant(const Variant& var);
    Variant(int s);
    Variant(char s);
    Variant(const std::string& s);
    Variant(const char* s);
    Variant(double s);

    /*
     *Returns the type of the variable.
     */
    VARIABLE_TYPE Typeof() const;

    // getters
    int GetInt() const;
    char GetByte() const;
    std::string GetStr() const;
    double GetDouble() const;
    // setters
    void SetInt(int s);
    void SetByte(char s);
    void SetDouble(double s);
    void SetStr(const std::string& s);
    void SetStr(const char* s);
    // type checks
    bool IsEmpty() const;
    bool IsInt() const;
    bool IsByte() const;
    bool IsDouble() const;
    bool IsNumber() const;
    bool IsChar() const;
    bool IsString() const;

    template <class T>
    T Get() const
    {
        switch (type)
        {
            case VAR_INT:
                return i32;
                break;
            case VAR_BYTE:
                return byte;
                break;
            case VAR_STR:
                return str;
                break;
            case VAR_DOUBLE:
                return d;
                break;
            case VAR_EMPTY:
                throw(Exception("Tried to get value of empty variable."));
                break;
            default:
                throw(Exception("Unknown variable type."));
                break;
        }
    }

    bool Compare(const Variant& var) const;

    // operator overloads
    bool operator==(const Variant& var);
    bool operator!=(const Variant& var);
    // assignment overloads
    Variant& operator=(int s);
    Variant& operator=(char s);
    Variant& operator=(double s);
    Variant& operator=(std::string s);

    template <class T>
    T operator()() const
    {
        switch (type)
        {
            case VAR_INT:
                return (GetInt());
                break;
            case VAR_BYTE:
                return (GetByte());
                break;
            case VAR_STR:
                return (GetStr());
                break;
            case VAR_DOUBLE:
                return (GetDouble());
                break;
            case VAR_EMPTY:
            default:
                throw(Exception("Tried to get invalid variable type."));
                break;
        }
    }
    void SetType(VARIABLE_TYPE t);

    // serialization
    //  JSON serialization
    void ToJson(Json::Value& json) const;
    void FromJson(const Json::Value& json, int version);

    // math operator overloads:
    // prefix ++ and --
    Variant& operator++();
    Variant& operator--();
    Variant operator+(Variant var);
    Variant& operator+=(Variant var);
    Variant operator-(Variant var);
    Variant& operator-=(Variant var);
    Variant operator*(Variant var);
    Variant& operator*=(Variant var);
    Variant operator/(Variant var);
    Variant& operator/=(Variant var);
    Variant operator%(Variant var);
    Variant& operator%=(Variant var);
    bool operator<(Variant var);
    bool operator<=(Variant var);
    bool operator>(Variant var);
    bool operator>=(Variant var);
};
#endif
