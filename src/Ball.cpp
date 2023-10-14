#include "Ball.h"
#include "Manager.h"
#include "raymath.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <iostream>

Ball::Ball(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity)
    : Entity(_position, _outputDims, _hitboxDims, EntityType::BALL) {

    float upDown = (float)rand()/RAND_MAX;

    currentVelocity = (Vector2){ (float)rand()/RAND_MAX, (float)rand()/RAND_MAX };
    currentVelocity = upDown > 0.5f ? currentVelocity : (Vector2){-currentVelocity.x, -currentVelocity.y};

    float mag = Vector2Length(currentVelocity);
    currentVelocity = Vector2Normalize(currentVelocity);
    currentVelocity = (Vector2){_maxVelocity * currentVelocity.x, _maxVelocity * currentVelocity.y};

    maxVelocity = _maxVelocity;
}

//------------------------------------------------------------------------------------
// Out-Of-Bounds check
//------------------------------------------------------------------------------------
bool Ball::hitGoal(Manager* _manager, Vector2 _position) {
    // TODO: is there a faster way to do this???
    for(int i=0; i<_manager->numGoalPoints-1; i++) {
        Vector2* topPoints = _manager->goalSections[0];
        Vector2* bottomPoints = _manager->goalSections[1];

        Vector2 p1 = topPoints[i];
        Vector2 p2 = topPoints[i+1];

        Vector2 p3 = bottomPoints[i];
        Vector2 p4 = bottomPoints[i+1];


        if(CheckCollisionPointLine(_position, p1, p2, 2.0f))
        {
            std::cout << "GOOOAALLLLLL!!!!!" << std::endl;
            return true;
        }

        if(CheckCollisionPointLine(_position, p3, p4, 2.0f))
        {
            std::cout << "GOOOAALLLLLL!!!!!" << std::endl;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------------
// Out-Of-Bounds check
//------------------------------------------------------------------------------------
bool Ball::outOfBounds(Manager* _manager, Vector2 _position) {
    // TODO: is there a faster way to do this???
    for(int i=0; i<_manager->numPoints-1; i++) {
        Vector2 p1 = _manager->boundaryPoints[i];
        Vector2 p2 = _manager->boundaryPoints[i+1];

        if(CheckCollisionPointLine(_position, p1, p2, 2.0f))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------
// Update ball
//------------------------------------------------------------------------------------
void Ball::update(Manager* _manager, int _screenWidth, int _screenHeight, float dt) {
    Vector2 oldPosition = position;

    Vector2 resultantVelocity = currentVelocity;
    Vector2 positionDelta = { resultantVelocity.x * dt, resultantVelocity.y * dt };
    position = Vector2Add(position, positionDelta);

    // TODO: ADD GLOBAL HITBOX VAR
    // if (IsKeyDown(KEY_H)) showHitboxes = !showHitboxes;

    if (hitGoal(_manager, position))
    {

        position = (Vector2){(float)(_manager->screenWidth/2), (float)(_manager->screenHeight/2)};
    }

    if (outOfBounds(_manager, position)) {
        position = (Vector2){(float)(_manager->screenWidth/2), (float)(_manager->screenHeight/2)};
    }

    currentVelocity = resultantVelocity;
}

void Ball::draw() {
    DrawCircle(position.x, position.y, outputDims.x, WHITE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
