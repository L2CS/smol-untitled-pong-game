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
    // player sprite
    Texture2D spriteSheet;
    // texture dimensions of player sprite
    Vector2 textureDims;
    // force produced for movement
    float force;
    // max ship velocity
    float maxVelocity;
    // current ship velocity
    Vector2 currentVelocity;
    // friction coefficient
    float frictionCoeff;
    // normal force -> used to produce friction
    float normal;
    // coords of first image of spritesheet for player
    Vector2 src;
    // health points
    float hp;
    // keybinds
    Keybinds binds;

    Player(Texture2D _spriteSheet, Vector2 _src, Vector2 _textureDims, Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity, float _force, float _frictionCoeff, float _normal, float _hp, Keybinds _binds);
    bool outOfBounds(Manager* _manager, Vector2 _position);
    void update(Manager* _manager, int _screenWidth, int _screenHeight, float dt);
    void draw();
};
