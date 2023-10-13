#include "Ball.h"
#include "Manager.h"
#include "raymath.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>

Ball::Ball(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity)
    : Entity(_position, _outputDims, _hitboxDims, EntityType::BALL)
{
    currentVelocity = { (float)rand()/RAND_MAX, (float)rand()/RAND_MAX };
    float mag = Vector2Length(currentVelocity);
    currentVelocity = Vector2Normalize(currentVelocity);
    currentVelocity = (Vector2){_maxVelocity * currentVelocity.x, _maxVelocity * currentVelocity.y};

    maxVelocity = _maxVelocity;
}

//------------------------------------------------------------------------------------
// Out-Of-Bounds check
//------------------------------------------------------------------------------------
bool Player::outOfBounds(Manager* _manager, Vector2 entity) {
{
    if (entity.x <= (float)(_manager->screenWidth / 2) - _manager->paddleBoundaryWidth / 2 
        || entity.x >= (float)(_manager->screenWidth / 2) + _manager->paddleBoundaryWidth / 2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------
// Update ball
//------------------------------------------------------------------------------------
void Ball::update(Manager* _manager, int _screenWidth, int _screenHeight, float dt)
{
    Vector2 oldPosition = position;

    Vector2 resultantVelocity = currentVelocity;
    Vector2 positionDelta = { resultantVelocity.x * dt, resultantVelocity.y * dt };
    position = Vector2Add(position, positionDelta);

    // TODO: ADD GLOBAL HITBOX VAR
    // if (IsKeyDown(KEY_H)) showHitboxes = !showHitboxes;

    if (outOfBounds(_manager, position)) {
        if(hitGoal())
        {
            posititon = (Vector2){ (float)_manager->screenWidth / 2, (float)(_manager->screenHeight / 2) };
        }
        position = oldPosition;
    }

    currentVelocity = resultantVelocity;
}

void Ball::draw()
{
    DrawCircle(position.x, position.y, outputDims.x, WHITE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
