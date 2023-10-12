#ifndef _ENTITY
#define _ENTITY

#include "raylib.h"
#include <chrono>
#include <unordered_map>
#include <vector>

enum EntityType {
    PLAYER,
    BALL,
    POWERUP
};

using EntityId = unsigned int;

struct Entity {
    static EntityId newId;
    EntityId id;
    Vector2 outputDims;
    Vector2 hitboxDims;
    Vector2 position;
    float rotation;
    EntityType type;
    bool destroyed;

    Entity(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, EntityType _type);
    void update();
    void draw();
};

using EntityMap = std::unordered_map<EntityId, Entity*>;

#endif // _ENTITY
