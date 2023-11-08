#pragma once

#include "Entity.h"

#include <vector>

struct Manager; // Forward declaration of Manager

struct Keybinds {
    std::vector<int> LEFT;
    std::vector<int> RIGHT;
};

// TODO: add CheckCollisionPointPoly instead of using a rectangle for collision detection

struct Player : Entity {
    Texture2D spriteSheet;   // Player sprite
    Vector2 textureDims;     // Texture dimensions of player sprite
    float force;             // Force produced for movement
    float maxVelocity;       // Max ship velocity
    Vector2 currentVelocity; // Current ship velocity
    float frictionCoeff;     // Friction coefficient
    float normal;            // Normal force -> used to produce friction
    Vector2 src;             // Coords of first image of spritesheet for player
    float hp;                // Health points
    Keybinds binds;          // Keybinds

    Player(Texture2D _spriteSheet, Vector2 _src, Vector2 _textureDims, Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity, float _force, float _frictionCoeff, float _normal, float _hp, Keybinds _binds);
    bool outOfBounds(Manager* _manager, Vector2 _position);
    void update(Manager* _manager, int _screenWidth, int _screenHeight, float dt);
    void draw();
};
