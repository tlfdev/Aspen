#pragma once
#ifndef WORLD_H
    #define WORLD_H
    #include "ComponentFactory.h"
    #include "channel.h"
    #include "command.h"
    #include "component.h"
    #include "componentMeta.hpp"
    #include "conf.h"
    #include "eventManager.h"
    #include "mud.h"
    #include "objectManager.h"
    #include "olcManager.h"
    #include "optionManager.h"
    #include "player.h"
    #include "playerManager.h"
    #include "serializer.h"
    #include "server.h"
    #include "zone.h"

    #include <functional>
    #include <map>
    #include <string>
    #include <unordered_map>
    #include <vector>

typedef std::function<std::string(Player*)> PROMPTCB;
/**
This class is responsible for managing collections of objects (users, zones etc) as well as holding references to
manager objects. All of the core functions will be kept within this class, and tasks delegated to other classes as
needed for organizational purposes.
*/
class World
{
    static World* _ptr;
    Server* _server; // our main server object.
    ComponentFactory _cfactory;
    std::unordered_map<char, PROMPTCB> _prompts;
    std::map<int, Channel*> _channels;
    std::map<std::string, void*> _properties;
    std::map<std::string, ISerializable*> _state;
    std::vector<Zone*> _zones;
    OlcManager _olcs;
    ObjectManager _objectManager;
    PlayerManager _pmanager;
    int _chanid;
    unsigned long long int _updates;
    unsigned long long int _totalUpdateTime;
    unsigned long long int _totalSleepTime;
    unsigned long long int _commands;
    unsigned long long int _commandElapsed;
    time_t _ruptime; // real uptime.
    time_t _cuptime; // uptime since last copyover.
    bool _running;
    // mud text buffers
    char* _motd;
    char* _banner;
    OptionManager _options;

    /**
     * Todo: update zones to use a manager.
     */
    void UpdateZones();

  public:
    static World* GetPtr();
    // these are objects we need to store on each entity.
    EventManager events;
    Commandable commands;

    World();
    ~World();

    /**
    Initializes the server object.
    */
    void InitializeServer();
    /**
    Shuts down the mud: saves open objects, sends messages to players, etc.
    */
    void Shutdown();
    /**
    Performs a copyover.
    \param: [in] the player who initiated the copyover.
    */
    void Copyover(Player* mobile);
    /**
    Returns a pointer to the main server object.
    */
    Server* GetServer() const;
    /**
    Returns a pointer to the OLC manager.
    */
    OlcManager* GetOlcManager();
    /**
    Returns a pointer to the component factory object.
    */
    ComponentFactory* GetComponentFactory();
    PlayerManager& GetPlayerManager();
    /*
     *Adds a channel to the worlds channel lookup table.
     *Param: [in] A pointer to the object to add.
     */
    bool AddChannel(Channel* chan, bool command = true);
    /*
     *Will locate a channel object based on it's ID.
     *Param: [in] the id of the channel.
     *Return: a pointer to the channel object if it was found, NULLotherwise.
     */
    Channel* FindChannel(int id);
    /*
     *Will locate a channel object based on it's name.
     *Param: [in] The name of the channel to locate.
     *Return: a pointer to the channel object if it was found, NULLotherwise.
     */
    Channel* FindChannel(const std::string& name);
    /*
     *Returns a list of all channel names, used to show channels and to allow for easily getting access to their
     *pointers. Param: [out] A pointer to a list of strings that will receive the chanel names.
     */
    void GetChannelNames(std::list<std::string>* out);
    /*
     *Checks to see if the specified channel exists.
     *Param: [in] a pointer to the channel.
     *Return: True if the channel exists, false otherwise.
     */
    bool ChannelExists(Channel* chan);
    /*
     *Registers the specified component
     *Params: [in] The name of the component.
     *[in] The component registration callback.
     *[in] the component init callback.
     *Return: True if the component was registered, false otherwise.
     */
    bool RegisterComponent(IComponentMeta* meta);
    /*
     *Creates the specified component
     *Parameters: [in] the name of the component to create.
     *Return: The object created.
     */
    Component* CreateComponent(const std::string& name);
    /**
     *Gets the real uptime.
     *Return: the time the mud was last rebooted.
     */
    time_t GetRealUptime() const;
    /*
     *Sets the real uptime
     *Param: [in] the time of the last hard reboot.
     */
    void SetRealUptime(time_t tm);
    /*
     *gets the time of the last copyover.
     *Return: time of last copyover.
     */
    time_t GetCopyoverUptime() const;
    /*
     *Sets the time of the last copyover.
     *Param: [in] the time of last copyover.
     */
    void SetCopyoverUptime(time_t tm);
    /*
     *Adds a property to the world's property table.
     *Param: [in] the name of the property to add.
     *[in] a pointer to the object.
     *Return: true on success, false if the property couldn't be added (null pointer provided, or the property already
     *exists).
     */
    bool AddProperty(const std::string& name, void* ptr);
    /*
     *Retrieves the property from the world's property table.
     *Param: [in] the name of the property.
     *Return: a pointer to the object, NULL if the property wasn't found.
     */
    void* GetProperty(const std::string& name);
    /*
     *Removes the property from the property table.
     *Param: [in] the name of the property to remove.
     *Return: true on success, false otherwise.
     *Note: this will not call delete on the object associated with that property.
     */
    bool RemoveProperty(const std::string& name);
    /*
     *Loads files into memory such as the MOTD so we won't have to read them every time we need them.
     */
    bool InitializeFiles();
    /*
     *The following functions return pointers to memory where the loaded files can be found.
     */
    const char* GetBanner() const;
    const char* GetMotd() const;
    /*
     *Updates the world.
     *Polls sockets, updates events, and keeps the world clean with tasks that should be ran every game loop.
     *Keep this function as minimal as possible; everything that occurs here runs every game loop iteration.
     */
    void Update();
    /*
     *Will perform a command for the specified player.
     *Params: [in] a pointer to the player object.
     *[in] A string with the arguments passed to the command.
     *Returns: true on success, false on failure.
     */
    void ParseArguments(const std::string& args, int start, std::vector<std::string>& params);
    bool DoCommand(Player* mobile, const std::string args);

    /*
     *Matching methods.
     */

    Entity* MatchKeyword(const std::string& name, Player* caller);
    /*
     *Finds an object from the input.
     *Param: [in] the name of the object.
     *Return: A pointer to the object, NULL if it wasn't found.
     */
    Entity* MatchObject(const std::string& name, Player* caller);
    /*
     *Matches an object in a list.
     *Param: [in] the name of the object to find.
     *param: [in] a pointer to the list of objects to match against.
     *Return: An std::pair containing the return code and a pointer to the object.
     */
    Entity* MatchObjectInList(const std::string& name, std::list<Entity*>* olist);
    /*
     *Matches an object in a vector.
     *Param: [in] the name of the object to find.
     *param: [in] a pointer to the vector of objects to match against.
     *Return: An std::pair containing the return code and a pointer to the object.
     */
    Entity* MatchObjectInVector(const std::string& name, std::vector<Entity*>* olist);
    /*
     *Adds a zone to the list of zones.
     *Param: [in] a pointer to the zone to add.
     *Return: True on success, false on failure.
     */
    bool AddZone(Zone* zone);
    /*
     *Removes the zone from the list.
     *Param: [in] a pointer to the object to remove.
     *Return: True on success, false on failure.
     */
    bool RemoveZone(Zone* zone);
    /*
     *Retrieves a pointer to the specified zone.
     *Param: [in] The name of the zone object.
     *Return: A pointer to the object, NULLif it wasn't found.
     */
    Zone* GetZone(const std::string& name);
    /*
     *Populates a vector with pointers to the currently loaded zones.
     *Param: [in] a pointer to the vector to populate.
     *Return: True on success, false on failure.
     */
    bool GetZones(std::vector<Zone*>* zones);
    OptionManager* GetOptionManager();
    /*
     *Checks to see if the game engine is still running.
     *Return: True if the game loop should continue, false otherwise.
     */
    bool IsRunning() const;
    /*
     *Sets whether the game loop should continue.
     *This will be caught each iteration.
     *Param: [in] A boolian value that controls the game loop.
     */
    void SetRunning(bool running);
    /*
     *Checks to see if the specified prompt handler exists.
     *Param: [in] the letter of the handler to check for.
     *return: True if the prompt exists, false otherwise.
     */
    bool PromptExists(char prompt);
    /*
     *Registers the prompt variable and handler.
     *Param: [in] the letter to register for the prompt.
     *Param: [in] a std::function that points to the callback to retrieve information.
     *Return: True if the prompt could be registered, false otherwise.
     */
    bool RegisterPrompt(char prompt, PROMPTCB callback);
    /*
     *Format's a player's prompt.
     *Param: [in] the string with formatting characters.
     *Param: [in] a pointer to the player object.
     *Return: the new prompt with formatting characters replaced.
     */
    std::string BuildPrompt(const std::string& prompt, Player* mobile);
    /*
     *Adds the state to the list of pre-existing states to be serialized.
     *Param: [in] the name of the state to add.
     *Param: [in] a pointer to an object which inherits ISerializable.
     *Return: True if the state could be added, false otherwise.
     */
    bool AddState(const std::string& name, ISerializable* s);
    /*
     *Removes the specified state by name.
     *Param: [in] the name of the state to remove.
     *Return: True if the state can be removed, false otherwise.
     */
    bool RemoveState(const std::string& name);
    /*
     *Checks to see if the specified state exists.
     *Param: [in] the name of the state.
     *Return: True if the state exists, false otherwise.
     */
    bool StateExists(const std::string& name);
    /*
     *Saves and loads the world state, respectively.
     *Return: True on success, false on failure.
     */
    bool SaveState();
    bool LoadState();
    unsigned long long int GetUpdates() const;
    unsigned long long int GetUpdateTime() const;
    unsigned long long int GetSleepTime() const;
    unsigned long long int GetCommands() const;
    unsigned long long int GetCommandTime() const;
    ObjectManager* GetObjectManager();
};
#endif
