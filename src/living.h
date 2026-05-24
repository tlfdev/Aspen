/**
 * Contains: Living
 */
#pragma once
#ifndef LIVING_H
    #define LIVING_H
    #include "affect.h"
    #include "attribute.h"
    #include "entity.h"
    #include "event.h"
    #include "mud.h"

    #include <queue>
    #include <vector>

/**
 * Affect comparison.
 *
 * This functor is used to compare affects in the priority queue.
 */
struct _aff_comp
{
    bool operator()(Affect* a, Affect* b)
    {
        return ((a->GetDuration() > b->GetDuration()) ? true : false);
    }
};

/**
 * Gender of the object.
 */
enum class Gender
{
    Male = 1,
    Female,
    Neuter
};

/**
 * Objects which should be sentient or alive (players, NPCS) should inherit this.
 */
class Living : public Entity
{
    unsigned int _position;
    Gender _gender;
    std::vector<Attribute*> _attributes;
    std::priority_queue<Affect*, std::vector<Affect*>, _aff_comp> _taffects;
    std::vector<Affect*> _paffects;
    Living* _following;
    std::list<Living*> _followers;

  public:
    Living();
    virtual ~Living() = default;

    /**
     * This is called when an object enters or leaves the game environment
     */
    virtual void EnterGame();
    /**
     * This is called when the object leaves the game environment.
     */
    virtual void LeaveGame();

    /**
     * This is called whenever the living is pulsed.
     */
    virtual void Update();
    /**
     * Overwritten to note that this is a living object.
     */
    bool IsLiving() const;

    Gender GetGender() const;
    void SetGender(Gender gender);
    /**
     * Retrieves the position of the living.
     */
    unsigned int GetPosition() const;
    /**
     * sets the position for this object.
     * \param [in] the position.
     */
    void SetPosition(unsigned int pos);
    bool AddAttribute(Attribute* attr);
    void FindAttribute(int apply, int id, std::vector<Attribute*>& results);
    void FindAttribute(int type, std::vector<Attribute*>& results);
    /***
    This is called to determine if follower can follow.
    Return true on success, false otherwise.
    */
    bool CanFollow(Living* follower);

    // JSON serialization
    virtual void ToJson(Json::Value& json) const override;
    virtual void FromJson(const Json::Value& json, int version) override;
    virtual int GetSerializationVersion() const override
    {
        return 1;
    }
};

#endif // LIVING_H
