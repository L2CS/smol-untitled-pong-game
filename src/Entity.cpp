#include "Entity.h"

EntityId Entity::newId = 0;

Entity::Entity(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, EntityType _entityType)
{
    position = _position;
    outputDims = _outputDims;
    hitboxDims = _hitboxDims;
    type = _entityType;
    id = Entity::newId;
    Entity::newId++;
    destroyed = false;
}

void Entity::update()
{
}

void Entity::draw()
{
    Rectangle destRec = {position.x, position.y, outputDims.x, outputDims.y};
    Vector2 origin = {(float)outputDims.x / 2, (float)outputDims.y / 2};
    DrawRectanglePro(destRec, origin, 0.0, PINK);
}
