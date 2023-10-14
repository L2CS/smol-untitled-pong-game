#include "Ball.h"
#include "Manager.h"
#include "raymath.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <iostream>

Ball::Ball(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity)
    : Entity(_position, _outputDims, _hitboxDims, EntityType::BALL)
{
    float upDown = (float)rand() / RAND_MAX;

    currentVelocity = (Vector2){ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };
    currentVelocity = upDown > 0.5f ? currentVelocity : (Vector2){ -currentVelocity.x, -currentVelocity.y };

    float mag = Vector2Length(currentVelocity);
    currentVelocity = Vector2Normalize(currentVelocity);
    currentVelocity = (Vector2){ _maxVelocity * currentVelocity.x, _maxVelocity * currentVelocity.y };

    maxVelocity = _maxVelocity;
}

//------------------------------------------------------------------------------------
// Out-Of-Bounds check
//------------------------------------------------------------------------------------
bool Ball::hitGoal(Manager* _manager, Vector2 _position)
{
    // TODO: is there a faster way to do this???
    for (int i = 0; i < _manager->numGoalPoints - 1; i++) {
        Vector2* topPoints = _manager->goalSections[0];
        Vector2* bottomPoints = _manager->goalSections[1];

        Vector2 p1 = topPoints[i];
        Vector2 p2 = topPoints[i + 1];

        Vector2 p3 = bottomPoints[i];
        Vector2 p4 = bottomPoints[i + 1];

        if (CheckCollisionPointLine(_position, p1, p2, 2.0f)) {
            std::cout << "GOOOAALLLLLL!!!!!" << std::endl;
            return true;
        }

        if (CheckCollisionPointLine(_position, p3, p4, 2.0f)) {
            std::cout << "GOOOAALLLLLL!!!!!" << std::endl;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------------
// Out-Of-Bounds check
//------------------------------------------------------------------------------------
bool Ball::outOfBounds(Manager* _manager)
{
    if (CheckCollisionPointCircle({ position.x, position.y }, { float(_manager->screenWidth / 2), float(_manager->screenHeight / 2) }, 300.0f)) {
        return false;
    }
    else {
        Vector2 circleCenter = { float(_manager->screenWidth / 2), float(_manager->screenHeight / 2) };
        double circleRadius = 300.0;

        // Calculate the vector from the ball's position to the circle's center
        Vector2 toCenter = Vector2Subtract(circleCenter, position);

        // Calculate the normalised vector representing the inside of the circle
        Vector2 insideNormal = Vector2Normalize(toCenter);

        // Calculate the reflection of the current velocity inside the circle
        currentVelocity = Vector2Reflect(currentVelocity, insideNormal);
    }

    return true;
}

//------------------------------------------------------------------------------------
// Update ball
//------------------------------------------------------------------------------------
void Ball::update(Manager* _manager, int _screenWidth, int _screenHeight, float dt)
{
    Vector2 oldPosition = position;

    if (hitGoal(_manager, position)) {
        position = (Vector2){ (float)(_manager->screenWidth / 2), (float)(_manager->screenHeight / 2) };
    }

    if (outOfBounds(_manager)) {
        position = oldPosition;
    }

    Vector2 resultantVelocity = currentVelocity;
    Vector2 positionDelta = { resultantVelocity.x * dt, resultantVelocity.y * dt };
    position = Vector2Add(position, positionDelta);

    currentVelocity = resultantVelocity;

    // Call handleCollisions for the ball to handle collisions
    handleCollisions(_manager);

    // TODO: ADD GLOBAL HITBOX VAR
    // if (IsKeyDown(KEY_H)) showHitboxes = !showHitboxes;
}

void Ball::handleCollisions(Manager* _manager)
{
    // Iterate through players and check collisions
    for (Player* player : _manager->players) {
        // TODO: Replace with poly collisions using CheckCollisionPointPoly()
        if (CheckCollisionPointCircle({ position.x, position.y }, { player->position.x, player->position.y }, 25.0f)) {
            // Calculate the collision point relative to the center of the circle
            float relativeX = position.x - player->position.x;
            float relativeY = position.y - player->position.y;

            // Calculate the angle of collision
            float collisionAngle = atan2f(relativeY, relativeX);

            // Calculate the new velocity based on the angle
            float speed = Vector2Length(currentVelocity);
            currentVelocity.x = cosf(collisionAngle) * speed;
            currentVelocity.y = sinf(collisionAngle) * speed;
        }
    }
}

void Ball::draw()
{
    DrawCircle(position.x, position.y, outputDims.x, WHITE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
