#pragma once

#include "Entity.h"

#include <memory>
#include <vector>

struct Manager; // Forward declaration of Manager

// TODO: Add CheckCollisionPointPoly instead of using a rectangle for collision detection

struct Ball : Entity {
    // Max ship velocity
    float maxVelocity;
    // Current ship velocity
    Vector2 currentVelocity;
    // Gravity strength toward goal areas
    float gravity;

    Ball(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity, float _gravity);
    int hitGoal(Manager* _manager, Vector2 _position); // Returns player index who scored (-1 if no goal)
    void handleCollisions(Manager* _manager);
    void update(Manager* _manager, int _screenWidth, int _screenHeight, float dt);
    void draw();
};
