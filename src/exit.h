/**
 * Contains: exit
 *
 * Exits are a method by which rooms are connected.
 */
#ifndef EXIT_H
#define EXIT_H
#include "jsonSerializer.h"
#include "living.h"
#include "mud.h"

#include <string>

/**
 * Exits create connections between individual rooms.
 */
class Exit : public IJsonSerializable
{
  protected:
    VNUM _to; // the vnum that the exit points to.
    ExitDirection _direction;

  public:
    Exit(VNUM to);
    Exit();
    virtual ~Exit() = default;

    /**
     * Gets the location this exit points to.
     */
    virtual VNUM GetTo() const;
    /**
     * Sets the location the exit points to.
     *
     * @param [in] to the VNUM of the location the exit leads to.
     */
    virtual void SetTo(VNUM to);

    /**
     * Gets the direction associated with the exit.
     */
    virtual ExitDirection GetDirection() const;
    /**
     * Sets the direction associated with the exit.
     *
     * @param [in] dir the new direction for the exit.
     */
    virtual void SetDirection(ExitDirection dir);
    /**
     * Translates the direction to a human-readable name.
     *
     * @return The name of the direction for the exit.
     */
    std::string GetName() const;

    /**
     * Checks to see if the npc/player can enter through the exit.
     *
     * @param [in] the mobile to test.
     * @return true if the player/npc can enter, false otherwise.
     */
    virtual bool CanEnter(Living* mobile);

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }
};
#endif
