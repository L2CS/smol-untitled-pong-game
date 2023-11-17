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
    float upDown = static_cast<float>(rand()) / RAND_MAX;

    currentVelocity = Vector2{ static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX };

    currentVelocity = upDown > 0.5f ? currentVelocity : Vector2{ -currentVelocity.x, -currentVelocity.y };

    float mag = Vector2Length(currentVelocity);
    currentVelocity = Vector2Normalize(currentVelocity);
    currentVelocity = (Vector2){ _maxVelocity * currentVelocity.x, _maxVelocity * currentVelocity.y };

    maxVelocity = _maxVelocity;
}

/**
 * Hitting goal check
 */
bool Ball::hitGoal(Manager* _manager, Vector2 _position)
{
    // TODO: Is there a faster way to do this???
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

/**
 * Out-of-bounds check
 */
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

/**
 * Update ball
 */
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
Vector2 polyPoints[8];
Vector2 polyPoints2[8];
void Ball::handleCollisions(Manager* _manager)
{
    const int numPoints = 8; // Number of points in the collider

    // Offsets for the collider points
    Vector2 offsets[numPoints] = {
        { 20.0f, -1.0f },
        { 2.0f, 2.0f },
        { -10.0f, 2.0f },
        { -28.0f, -1.0f },
        { -26.0f, -14.0f },
        { -10.0f, -12.0f },
        { 2.0f, -12.0f },
        { 18.0f, -14.0f }
    };
    // Iterate through players and check collisions
    for (Player* player : _manager->players) {
        float rotationAngle = player->rotation * (float)M_PI / 180.0f;
        for (int i = 0; i < numPoints; ++i) {
            float x = offsets[i].x * cosf(rotationAngle) -
                      offsets[i].y * sinf(rotationAngle);
            float y = offsets[i].x * sinf(rotationAngle) +
                      offsets[i].y * cosf(rotationAngle);

            polyPoints[i] = { player->position.x + x, player->position.y + y };
        }
        // TODO: Use the correct number of points and angles based on your
        // game's requirements

        // Check collisions using CheckCollisionPointPoly
        if (CheckCollisionPointPoly(position, polyPoints, 8)) {
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
    for (int i = 0; i < 8; i++) {
        DrawPixel(polyPoints[i].x, polyPoints[i].y, RED);
    }
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
