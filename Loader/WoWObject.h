#ifndef WoWObjet_H
#define WoWObjet_H
#include <vector>
#include <string>

enum ObjectType {
    None,
    Item,
    Container,
    Unit,
    Player,
    GameObject,
    DynamicObject,
    Corpse
};

enum ClickType {
    FaceTarget = 0x1,
    Face = 0x2,
    Stop_ThrowsException = 0x3,
    Move = 0x4,
    NpcInteract = 0x5,
    Loot = 0x6,
    ObjInteract = 0x7,
    FaceOther = 0x8,
    Skin = 0x9,
    AttackPosition = 0xA,
    AttackGuid = 0xB,
    ConstantFace = 0xC,
    Nothing = 0xD,

    Attack = 0x10,
    Idle = 0x13,
};

enum UnitReaction {
    Hated,
    Hostile,
    Unfriendly,
    Neutral,
    Friendly,
    Honored,
    Revered,
    Exalted
};

enum CreatureType {
    Null,
    Beast,
    Dragonkin,
    Demon,
    Elemental,
    Giant,
    Undead,
    Humanoid,
    Critter,
    Mechanical,
    NotSpecified,
    Totem
};

enum UnitFlags : unsigned int {
    UNIT_FLAG_UNK_0 = 1 << 0, // Movement checks disabled, likely paired with loss of client control packet. We use it to add custom cliffwalking to GM mode until actual usecases will be known.
    UNIT_FLAG_NON_ATTACKABLE = 1 << 1, // not attackable
    UNIT_FLAG_CLIENT_CONTROL_LOST = 1 << 2, // Generic unspecified loss of control initiated by server script, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_PLAYER_CONTROLLED = 1 << 3, // players, pets, totems, guardians, companions, charms, any units associated with players
    UNIT_FLAG_RENAME = 1 << 4, // ??
    UNIT_FLAG_PREPARATION = 1 << 5, // don't take reagents for spells with SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_UNK_6 = 1 << 6, // ??
    UNIT_FLAG_NOT_ATTACKABLE_1 = 1 << 7, // ?? (UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_IMMUNE_TO_PLAYER = 1 << 8, // Target is immune to players
    UNIT_FLAG_IMMUNE_TO_NPC = 1 << 9, // Target is immune to Non-Player Characters
    UNIT_FLAG_LOOTING = 1 << 10, // loot animation
    UNIT_FLAG_PET_IN_COMBAT = 1 << 11, // in combat?, 2.0.8
    UNIT_FLAG_PVP = 1 << 12, // is flagged for pvp
    UNIT_FLAG_SILENCED = 1 << 13, // silenced, 2.1.1
    UNIT_FLAG_PERSUADED = 1 << 14, // persuaded, 2.0.8
    UNIT_FLAG_SWIMMING = 1 << 15, // controls water swimming animation - TODO: confirm whether dynamic or static
    UNIT_FLAG_NON_ATTACKABLE_2 = 1 << 16, // removes attackable icon, if on yourself, cannot assist self but can cast TARGET_UNIT_CASTER spells - added by SPELL_AURA_MOD_UNATTACKABLE
    UNIT_FLAG_PACIFIED = 1 << 17, // probably like the paladin's Repentance spell
    UNIT_FLAG_STUNNED = 1 << 18, // Unit is a subject to stun, turn and strafe movement disabled
    UNIT_FLAG_IN_COMBAT = 1 << 19,
    UNIT_FLAG_TAXI_FLIGHT = 1 << 20, // Unit is on taxi, paired with a duplicate loss of client control packet (likely a legacy serverside hack). Disables any spellcasts not allowed in taxi flight client-side.
    UNIT_FLAG_DISARMED = 1 << 21, // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED = 1 << 22, // Unit is a subject to confused movement, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_FLEEING = 1 << 23, // Unit is a subject to fleeing movement, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_POSSESSED = 1 << 24, // Unit is under remote control by another unit, movement checks disabled, paired with loss of client control packet. New master is allowed to use melee attack and can't select this unit via mouse in the world (as if it was own character).
    UNIT_FLAG_NOT_SELECTABLE = 1 << 25,
    UNIT_FLAG_SKINNABLE = 1 << 26,
    UNIT_FLAG_MOUNT = 1 << 27, // is mounted?
    UNIT_FLAG_UNK_28 = 1 << 28, // ??
    UNIT_FLAG_UNK_29 = 1 << 29, // used in Feing Death spell
    UNIT_FLAG_SHEATHE = 1 << 30, // ??
};

enum MovementFlags: unsigned int {
    MOVEFLAG_NONE = 0x00000000,
    MOVEFLAG_FORWARD = 0x00000001,
    MOVEFLAG_BACKWARD = 0x00000002,
    MOVEFLAG_STRAFE_LEFT = 0x00000004,
    MOVEFLAG_STRAFE_RIGHT = 0x00000008,
    MOVEFLAG_TURN_LEFT = 0x00000010,
    MOVEFLAG_TURN_RIGHT = 0x00000020,
    MOVEFLAG_PITCH_UP = 0x00000040, // ??
    MOVEFLAG_PITCH_DOWN = 0x00000080, // ??
    MOVEFLAG_WALK_MODE = 0x00000100, // Walking
    MOVEFLAG_ONTRANSPORT = 0x00000200, // Used for flying on some creatures
    MOVEFLAG_LEVITATING = 0x00000400,
    MOVEFLAG_ROOT = 0x00000800,
    MOVEFLAG_FALLING = 0x00001000,
    MOVEFLAG_FALLINGFAR = 0x00004000, // ??
    MOVEFLAG_SWIMMING = 0x00200000, // appears with fly flag also
    MOVEFLAG_ASCENDING = 0x00400000, // swim up also
    MOVEFLAG_CAN_FLY = 0x00800000, // ??
    MOVEFLAG_FLYING = 0x01000000, // ??
    MOVEFLAG_FLYING2 = 0x02000000, // Actual flying mode
    MOVEFLAG_SPLINE_ELEVATION = 0x04000000, // used for flight paths
    MOVEFLAG_SPLINE_ENABLED = 0x08000000, // used for flight paths
    MOVEFLAG_WATERWALKING = 0x10000000, // prevent unit from falling through water
    MOVEFLAG_SAFE_FALL = 0x20000000, // active rogue safe fall spell (passive)
    MOVEFLAG_HOVER = 0x40000000
};

class Position {
public:
    float X, Y, Z;

    Position();
    Position(float x, float y, float z);
    std::string ToString();
    float DistanceTo(Position position);
    float DistanceTo2D(Position position);
};

// === Objects === //

class WoWObject {
    public:
        unsigned long long Guid;
        uintptr_t Pointer;
        ObjectType objectType;

        WoWObject(uintptr_t pointer, unsigned long long guid, ObjectType objType);
        uintptr_t GetDescriptorPtr();

    protected:
        const uintptr_t DESCRIPTOR_OFFSET = 0x8;
};

class WoWUnit : public WoWObject {
    public:
        Position position; float prctHP; float prctMana; int rage; int energy;
        UnitFlags flags; MovementFlags movement_flags; int buff[30]; int debuff[16]; bool isdead;
        CreatureType creatureType; float speed; unsigned long long targetGuid;
        float facing; int level; char* name; int channelInfo; UnitReaction unitReaction;
        bool attackable; int hpLost;

        WoWUnit(uintptr_t pointer, unsigned long long guid, ObjectType objType);
        bool hasBuff(int* IDs, int size);
        bool hasDebuff(int* IDs, int size);
        bool isFacing(Position, float);
        bool isBehind(WoWUnit target);
        bool isChanneling(int* IDs, int size);
        UnitReaction getUnitReaction(uintptr_t);
        bool canAttack(uintptr_t);
        int getHealth(); int getMaxHealth();

    protected:
        const uintptr_t TARGET_GUID_OFFSET = 0x40;
        const uintptr_t HEALTH_OFFSET = 0x58;
        const uintptr_t MANA_OFFSET = 0x5C;
        const uintptr_t RAGE_OFFSET = 0x60;
        const uintptr_t ENERGY_OFFSET = 0x68;
        const uintptr_t MAX_HEALTH_OFFSET = 0x70;
        const uintptr_t MAXMANA_OFFSET = 0x74;
        const uintptr_t LEVEL_OFFSET = 0x88;
        const uintptr_t UNIT_FLAG_OFFSET = 0xB8;
        const uintptr_t MOVEMENT_FLAG_OFFSET = 0x9E8;
        const uintptr_t BUFF_BASE_OFFSET = 0xBC;
        const uintptr_t DEBUFF_BASE_OFFSET = 0x13C;
        const uintptr_t CHANNEL_OFFSET = 0x240;
        const uintptr_t POS_X_OFFSET = 0x9B8;
        const uintptr_t POS_Y_OFFSET = 0x9BC;
        const uintptr_t POS_Z_OFFSET = 0x9C0;
        const uintptr_t FACING_OFFSET = 0x9C4;
        const uintptr_t SPEED_OFFSET = 0xA2C;
        const uintptr_t NAME_OFFSET = 0xB30;
        const uintptr_t GET_CREATURE_TYPE_FUN_PTR = 0x00605570;
        const uintptr_t GET_UNIT_REACTION_FUN_PTR = 0x006061E0;
        const uintptr_t CAN_ATTACK_UNIT_FUN_PTR = 0x00606980;

        const uintptr_t NAME_BASE_OFFSET = 0xC0E230;
        const uintptr_t NEXT_NAME_OFFSET = 0xC;
        const uintptr_t PLAYER_NAME_OFFSET = 0x14;
};

class WoWPlayer : public WoWUnit {
    public:
        char* className;

        WoWPlayer(uintptr_t pointer, unsigned long long guid, ObjectType objectType);
};

class LocalPlayer : public WoWPlayer {
    public:
        int castInfo;

        LocalPlayer(uintptr_t pointer, unsigned long long guid, ObjectType objectType);
        void ClickToMove(ClickType, unsigned long long, Position);
        void SetTarget(unsigned long long tguid);
        WoWUnit* getTarget();
        Position getOppositeDirection(Position enemy_pos, float radius);
        bool isCasting();
        bool isCasting(int* IDs, int size);
    protected:
        const uintptr_t SET_TARGET_FUN_PTR = 0x00493540;
        const uintptr_t CLICK_TO_MOVE_FUN_PTR = 0x00611130;
        const uintptr_t LOCKED_TARGET_STATIC_OFFSET = 0x00B4E2D8;
        const uintptr_t CASTING_STATIC_OFFSET = 0x00CECA88;
};

extern std::vector<WoWUnit> ListUnits;
extern LocalPlayer* localPlayer;

#endif