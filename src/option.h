/*
 *A setup for holding options for players.
 *We register each option here, then if the player has something different set we check that.
 *This both saves us time and adds for more flexability.
 */
#pragma once
#ifndef OPTION_H
    #define OPTION_H
    #include "conf.h"
    #include "mud.h"
    #include "optionMeta.h"
    #include "variant.h"

    #include <string>

class Option
{
    OptionMeta* _meta;
    Variant _value;

  public:
    Option(OptionMeta* meta, const Variant& value);
    ~Option();
    OptionMeta* GetMeta() const;
    void SetMeta(OptionMeta* meta);
    Variant& GetValue();
    const Variant& GetValue() const;
    void SetValue(const Variant& value);
};

// option helper functions
/*
 *Gets the value of the option either from the global option, or from the player's option. Checks the player first.
 *Param: [in] the name of the option.
 *Param: [in] a pointer to the player object.
 *Return: A variant holding the value.
 */
class Player;
Variant GetOptionValue(const std::string& name, const Player* mobile);
#endif
