#pragma once

#include "Entity.h"

#include <vector>

struct Manager; // Forward declaration of Manager

// TODO: Add CheckCollisionPointPoly instead of using a rectangle for collision detection

struct Ball : Entity {
    // Max ship velocity
    float maxVelocity;
    // Current ship velocity
    Vector2 currentVelocity;

    Ball(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity);
    bool hitGoal(Manager* _manager, Vector2 _position);
    bool outOfBounds(Manager* _manager);
    void handleCollisions(Manager* _manager);
    void update(Manager* _manager, int _screenWidth, int _screenHeight, float dt);
    void draw();
};
