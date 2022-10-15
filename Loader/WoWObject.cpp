#include "WoWObject.h"

#include <string>

#include "MemoryManager.h"
#include <iostream>

std::vector<WoWUnit> ListUnits;
LocalPlayer* localPlayer = NULL;

Position::Position() {
    X = 0;
    Y = 0;
    Z = 0;
}

Position::Position(float x, float y, float z) {
    X = x;
    Y = y;
    Z = z;
}

std::string Position::ToString() {
    std::string txt = "x: " + std::to_string(X) + " y: " + std::to_string(Y) + " z :" + std::to_string(Z);
    return txt;
}

float Position::DistanceTo(Position position) {
    float deltaX = X - position.X;
    float deltaY = Y - position.Y;
    float deltaZ = Z - position.Z;

    return (float)sqrt((deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ));
}

/* === Object === */

WoWObject::WoWObject(uintptr_t pointer, unsigned long long guid, ObjectType objType) {
    Guid = guid;
    Pointer = pointer;
    objectType = objType;
}

uintptr_t WoWObject::GetDescriptorPtr() {
    return *(uintptr_t*)(Pointer + DESCRIPTOR_OFFSET);
}

/* === Unit === */

WoWUnit::WoWUnit(uintptr_t pointer, unsigned long long guid, ObjectType objType) : WoWObject(pointer, guid, objType) {
    uintptr_t descriptor = WoWObject::GetDescriptorPtr();

    float x = *(float*)(Pointer + POS_X_OFFSET);
    float y = *(float*)(Pointer + POS_Y_OFFSET);
    float z = *(float*)(Pointer + POS_Z_OFFSET);
    position = Position(x, y, z);

    int health = *(int*)(descriptor + HEALTH_OFFSET);
    isdead = false; if (health <= 1) isdead = true;
    int max_health = *(int*)(descriptor + MAX_HEALTH_OFFSET);
    prctHP = ((float)health / (float)max_health) * 100;
    int max_mana = *(int*)(descriptor + MAXPOWER1_OFFSET);
    if (max_mana > 0) {
        int mana = *(int*)(descriptor + POWER1_OFFSET);
        prctMana = ((float)mana / (float)max_mana) * 100;
    }
    else prctMana = 0;
    rage = *(int*)(descriptor + POWER2_OFFSET);
    energy = *(int*)(descriptor + POWER4_OFFSET);

    flags = *(UnitFlags*)(descriptor + UNIT_FLAG_OFFSET);

    uintptr_t currentBuffOffset = BUFF_BASE_OFFSET;
    for (int i = 0; i < 30; i++) {
        buff[i] = *(int*)(descriptor + currentBuffOffset);
        currentBuffOffset += 4;
    }
    uintptr_t currentDebuffOffset = DEBUFF_BASE_OFFSET;
    for (int i = 0; i < 16; i++) {
        debuff[i] = *(int*)(descriptor + currentDebuffOffset);
        currentDebuffOffset += 4;
    }

    typedef CreatureType __fastcall func(uintptr_t);
    func* getCreatureType = (func*)GET_CREATURE_TYPE_FUN_PTR;
    creatureType = getCreatureType(Pointer);

    speed = *(float*)(Pointer + SPEED_OFFSET);
    targetGuid = *(unsigned long long*)(descriptor + TARGET_GUID_OFFSET);
    facing = *(float*)(Pointer + FACING_OFFSET);
    level = *(int*)(descriptor + LEVEL_OFFSET);
    if(objType == Unit) name = (char*)(*(uintptr_t*)(*(uintptr_t*)(Pointer + NAME_OFFSET)));
    else {
        uintptr_t namePtr = *(uintptr_t*)(NAME_BASE_OFFSET);
        unsigned int i = 0;
        while (i <= ListUnits.size()) {
            unsigned long long nextGuid = *(unsigned long long*)(namePtr + NEXT_NAME_OFFSET);
            if (nextGuid != Guid) namePtr = *(uintptr_t*)(namePtr);
            else break;
            i++;
        }
        name = (char*)(namePtr + PLAYER_NAME_OFFSET);
    }

    channelInfo = *(int*)(descriptor + CHANNEL_OFFSET);

    unitReaction = Neutral;
}

int WoWUnit::getHealth() {
    return *(int*)(WoWObject::GetDescriptorPtr() + HEALTH_OFFSET);
}

int WoWUnit::getMaxHealth() {
    return *(int*)(WoWObject::GetDescriptorPtr() + MAX_HEALTH_OFFSET);
}

bool WoWUnit::hasBuff(int* IDs, int size) {
    for (int i = 0; i < size; i++) {
        for (int y = 0; y < 30; y++) {
            if (IDs[i] == buff[y]) return true;
        }
    }
    return false;
}

bool WoWUnit::hasDebuff(int* IDs, int size) {
    for (int i = 0; i < size; i++) {
        for (int y = 0; y < 16; y++) {
            if (IDs[i] == debuff[y]) return true;
        }
    }
    return false;
}

bool WoWUnit::isFacing(Position pos, float angle) {
    float f = atan2f(pos.Y - position.Y, pos.X - position.X);
    float PI = 2 * acos(0.0);
    if (f < 0.0f) f += PI * 2.0f;
    else {
        if (f > PI * 2) f -= PI * 2.0f;
    }
    if (abs(f - facing) < angle) return true;
    else return false;
}

bool WoWUnit::isBehind(WoWUnit target) {
    float halfPi = acos(0.0);
    float twoPi = halfPi * 4;
    float leftThreshold = target.facing - halfPi;
    float rightThreshold = target.facing + halfPi;

    bool condition;
    if (leftThreshold < 0)
        condition = facing < rightThreshold || facing > twoPi + leftThreshold;
    else if (rightThreshold > twoPi)
        condition = facing > leftThreshold || facing < rightThreshold - twoPi;
    else
        condition = facing > leftThreshold && facing < rightThreshold;

    return condition && isFacing(target.position, 0.4f);
}

UnitReaction WoWUnit::getUnitReaction(uintptr_t unitPtr2) {
    typedef UnitReaction(__thiscall* func)(uintptr_t unitPtr1, uintptr_t unitPtr2);
    func function = (func)GET_UNIT_REACTION_FUN_PTR;
    return function(Pointer, unitPtr2);
}

/* === Player === */

WoWPlayer::WoWPlayer(uintptr_t pointer, unsigned long long guid, ObjectType objType)
    : WoWUnit(pointer, guid, objType) {
    //
}

/* === Local Player === */

LocalPlayer::LocalPlayer(uintptr_t pointer, unsigned long long guid, ObjectType objType)
    : WoWPlayer(pointer, guid, objType) {
    castInfo = *(int*)(CASTING_STATIC_OFFSET);
    targetGuid = *(unsigned long long*)LOCKED_TARGET_STATIC_OFFSET;
}

void LocalPlayer::ClickToMove(ClickType clickType, unsigned long long interactGuid, Position pos) {
    float* xyz = new float[3];
    xyz[0] = pos.X; xyz[1] = pos.Y; xyz[2] = pos.Z;
    unsigned long long* interactGuidPtr = &interactGuid;
    typedef void (__thiscall* func)(uintptr_t, ClickType, unsigned long long*, float*, float);
    func function = (func)CLICK_TO_MOVE_FUN_PTR;
    function(Pointer, clickType, interactGuidPtr, xyz, 2);
    delete[] xyz;
}

void LocalPlayer::SetTarget(unsigned long long tguid) {
    typedef void __stdcall func(unsigned long long tguid);
    func* function = (func*)SET_TARGET_FUN_PTR;
    function(tguid);
}

WoWUnit* LocalPlayer::getTarget() {
    for (unsigned int i = 0; i < ListUnits.size(); i++) {
        if (ListUnits[i].Guid == targetGuid) return (&ListUnits[i]);
    }
    return NULL;
}

Position LocalPlayer::getOppositeDirection(Position enemy_pos) {
    float radius = 10;
    float m = (float)(enemy_pos.Y - position.Y) / (float)(enemy_pos.X - position.X);
    float p = position.Y - (m * position.X);
    float a = 1 + (m * m);
    float b = (-2 * position.X) - (2 * m * m * position.X);
    float c = (position.X * position.X) + (m * m * position.X * position.X) - (radius * radius);
    float delta = (b * b) - (4 * a * c);
    float x1 = (-b - sqrt(delta)) / (2 * a);
    float y1 = (m * x1 + p);
    float x2 = (-b + sqrt(delta)) / (2 * a);
    float y2 = (m * x2 + p);
    Position x1_pos = Position(x1, y1, position.Z + 5);
    Position x2_pos = Position(x2, y2, position.Z + 5);
    if (enemy_pos.DistanceTo(x1_pos) > enemy_pos.DistanceTo(x2_pos)) return x1_pos;
    else return x2_pos;
}

bool LocalPlayer::isCasting() {
    int casting = *(int*)(CASTING_STATIC_OFFSET);
    if (casting > 0) return true;
    else return false;
}

bool LocalPlayer::isCasting(int* IDs, int size) {
    for (int i = 0; i < size; i++) {
        if (IDs[i] == castInfo) return true;
    }
    return false;
}