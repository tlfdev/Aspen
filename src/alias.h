/**
 * Contains: Alias
 *
 * The Alias class is responsible for managing aliases.
 * In their simple form, aliases are commands which expand to multiple commands.
 * For example, you might want an alias to echo a message before you perform a common task.
 * This elimenates the need to type both commands, as you can have an alias that calls both commands.
 */
#pragma once
#include "conf.h"
#include "jsonSerializer.h"
#include "mud.h"
#include "player.h"

#include <string>
#include <vector>

/**
 * Manages individual aliases.
 */
class Alias : public IJsonSerializable
{
    std::vector<std::string> _aliases;
    std::string _name;

  public:
    Alias() = default;
    /**
     * Creates a named alias.
     *
     * @param [in] name The name of the alias.
     */
    Alias(const std::string& name);
    ~Alias() = default;
    /**
     * Returns the name of the alias.
     */
    std::string GetName() const;
    /**
     * Sets the name of the alias.
     *
     * @param [in] name The name of the alias.
     */
    void SetName(const std::string& name);
    /**
     * Processes the commands in the alias.
     *
     * Currently, we only pass the commands on to the command interpreter.
     * @todo Limit alias length.
     * @todo check for recursion of aliases.
     * @todo allow for delays.
     * @param [in] mobile The player calling the commands.
     */
    void ProcessCommands(Player* mobile);
    /**
     * Adds a command to the list of commands that this alias executes.
     *
     * @param [in] command The command to add to the alias.
     */
    void AddCommand(const std::string& command);
    /**
     * Removes all commands belonging to this alias.
     */
    void ClearCommands();
    /**
     * Gets a list of the commands in this alias.
     * @param [out] commands A vector the commands can be added to.
     */
    void ListCommands(std::vector<std::string>& commands);

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }
};
