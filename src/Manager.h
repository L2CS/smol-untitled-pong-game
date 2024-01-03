#pragma once

#include "Ball.h"
#include "helpers.h"
#include "Player.h"

#include "raymath.h"

#include <array>
#include <chrono>
#include <memory>
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

    int numPoints;

    std::vector<std::shared_ptr<Player>> players;

    // TODO: write handlers for adding powerups
    // std::vector<Powerup*> powerupsToAdd;

    // Constructor
    Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth);

    // Member functions for managing entities
    void addEntity(std::shared_ptr<Entity> entity);
    void deleteEntity(EntityId id);
    void addPlayer(std::shared_ptr<Player> player);
    void update();
    void draw();

private:
    // Map of entities
    EntityMap _entities;

public:
    // Level boundary points
    std::vector<Vector2> _boundaryPoints;

    // Goal points sections
    std::array<std::vector<Vector2>, 2> _goalSections;
    int numGoalPoints;
};
