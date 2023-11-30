#include "Ball.h"
#include "Manager.h"
#include "raymath.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <iostream>

/**
 * Collision check for circle and polygon
 */
bool CheckCollisionCirclePolygon(Vector2 circleCenter, float circleRadius, Vector2 polygonPoints[], int numPoints)
{
    // Check if the circle collides with the edges of the polygon
    for (int i = 0; i < numPoints; ++i) {
        Vector2 p1 = polygonPoints[i];
        Vector2 p2 = polygonPoints[(i + 1) % numPoints]; // Wrap around to the first point

        // Calculate the vector between the circle center and the line segment
        Vector2 v = { circleCenter.x - p1.x, circleCenter.y - p1.y };
        Vector2 edge = { p2.x - p1.x, p2.y - p1.y };

        // Project the circle center onto the line segment
        float projection = (v.x * edge.x + v.y * edge.y) / (edge.x * edge.x + edge.y * edge.y);
        projection = fmaxf(0, fminf(1, projection));

        // Calculate the closest point on the line segment to the circle center
        Vector2 closest = {
            p1.x + projection * edge.x,
            p1.y + projection * edge.y
        };

        // Check if the distance between the circle center and the closest point is within the circle's radius
        float distance = sqrtf((circleCenter.x - closest.x) * (circleCenter.x - closest.x) +
                               (circleCenter.y - closest.y) * (circleCenter.y - closest.y));

        if (distance <= circleRadius) {
            return true; // Collision detected
        }
    }

    return false; // No collision detected
}

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

void Ball::handleCollisions(Manager* _manager)
{
    const int numPoints = 8; // Number of points in the collider

    // Offsets for the collider points
    Vector2 offsets[numPoints] = {
        { 15.0f, -5.0f },
        { 2.0f, -3.0f },
        { -10.0f, -3.0f },
        { -23.0f, -5.0f },
        { -23.0f, -8.0f },
        { -10.0f, -6.0f },
        { 2.0f, -6.0f },
        { 15.0f, -8.0f }
    };

    // Iterate through players and check collisions
    for (Player* player : _manager->players) {
        Vector2 polyPoints[8];
        float rotationAngle = player->rotation * (float)M_PI / 180.0f;

        for (int i = 0; i < numPoints; ++i) {
            float x = offsets[i].x * cosf(rotationAngle) -
                      offsets[i].y * sinf(rotationAngle);
            float y = offsets[i].x * sinf(rotationAngle) +
                      offsets[i].y * cosf(rotationAngle);

            polyPoints[i] = { player->position.x + x, player->position.y + y };
        }

        // Check collisions using CheckCollisionCirclePoly
        if (CheckCollisionCirclePolygon(position, 5.0f, polyPoints, 8)) {
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

    // Draw player collider
    // for (int i = 0; i < 8; i++) {
    //     DrawPixel(polyPoints[i].x, polyPoints[i].y, RED);
    // }
}
