#pragma once

#include "Ball.h"
#include "helpers.h"
#include "Player.h"

#include "raymath.h"

#include <chrono>
#include <vector>

struct Manager {
    // Screen dims
    int screenWidth;
    int screenHeight;
    // Level radius
    // TODO: Write level loader to load levels instead of this
    float levelRadius;
    // Offset of paddles from the level boundary
    float levelOffset;
    // Boundary width of each paddle (1v1 only)
    // TODO: Make this dependent on the number of players
    float paddleBoundaryWidth;
    // Game clock -> used for physics
    std::chrono::system_clock::time_point lastUpdateTime;
    std::chrono::system_clock::time_point lastDrawTime;
    // Map of entities
    EntityMap entities;
    // Level boundary points
    Vector2* boundaryPoints;
    int numPoints;
    // Goal points sections
    Vector2** goalSections;
    int numGoalPoints;
    std::vector<Player*> players;
    // TODO: write handlers for adding powerups
    // std::vector<Powerup*> powerupsToAdd;

    // Constructor
    Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth);
    // Member functions for managing entities
    void addEntity(Entity* _entity);
    void deleteEntity(EntityId _id);
    void addPlayer(Player* player);
    void update();
    void draw();
};
