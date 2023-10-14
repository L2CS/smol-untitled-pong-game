#ifndef _MANAGER
#define _MANAGER

#include "helpers.h"
#include "Player.h"
#include "Ball.h"

#include "raymath.h"

#include <chrono>
#include <vector>

struct Manager {
    // screen dims
    int screenWidth;
    int screenHeight;
    // level radius
    // TODO: write level loader to load levels instead of this
    float levelRadius;
    // offset of paddles from the level boundary
    float levelOffset;
    // boundary width of each paddle (1v1 only)
    // TODO: make this dependent on the number of players
    float paddleBoundaryWidth;
    // game clock -> used for physics
    std::chrono::system_clock::time_point lastUpdateTime;
    std::chrono::system_clock::time_point lastDrawTime;
    // map of entities
    EntityMap entities;
    // level boundary points
    Vector2* boundaryPoints;
    int numPoints;
    // goal points sections
    Vector2** goalSections;
    int numGoalPoints;
    // TODO: write handlers for adding powerups
    // std::vector<Powerup*> powerupsToAdd;

    // constructor
    Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth);
    // member functions for managing entities
    void addEntity(Entity* _entity);
    void deleteEntity(EntityId _id);
    void update();
    void draw();
};

#endif // _MANAGER
