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
Vector2 polyPoints[3];
void Ball::handleCollisions(Manager* _manager)
{
    // Iterate through players and check collisions
    for (Player* player : _manager->players) {
        // Define the offsets for the collider points
        Vector2 offset1 = { -3.0f, -1.0f };
        Vector2 offset2 = { -22.0f, -8.0f };
        Vector2 offset3 = { 14.0f, -8.0f };
        float rotationAngle = player->rotation * (float)M_PI / 180.0f;

        // Rotate each point and apply the offsets
        polyPoints[0] = {
            player->position.x + (offset1.x * cosf(rotationAngle) - offset1.y * sinf(rotationAngle)),
            player->position.y + (offset1.x * sinf(rotationAngle) + offset1.y * cosf(rotationAngle))
        };

        polyPoints[1] = {
            player->position.x + (offset2.x * cosf(rotationAngle) - offset2.y * sinf(rotationAngle)),
            player->position.y + (offset2.x * sinf(rotationAngle) + offset2.y * cosf(rotationAngle))
        };

        polyPoints[2] = {
            player->position.x + (offset3.x * cosf(rotationAngle) - offset3.y * sinf(rotationAngle)),
            player->position.y + (offset3.x * sinf(rotationAngle) + offset3.y * cosf(rotationAngle))
        };
        // TODO: Use the correct number of points and angles based on your game's requirements
        // Check collisions using CheckCollisionPointPoly
        if (CheckCollisionPointPoly(position, polyPoints, 3)) {
            // Calculate the collision point relative to the center of the circle
            float relativeX = position.x - player->position.x;
            float relativeY = position.y - player->position.y;
            std::cout << relativeX << ", " << relativeY << std::endl;

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
    DrawPixel(polyPoints[0].x, polyPoints[0].y, RED);
    DrawPixel(polyPoints[1].x, polyPoints[1].y, RED);
    DrawPixel(polyPoints[2].x, polyPoints[2].y, RED);
    DrawTriangle(polyPoints[1], polyPoints[0], polyPoints[2], BLUE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
