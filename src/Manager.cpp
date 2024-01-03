#include "Manager.h"

#include <algorithm>
#include <chrono>
#include <iostream>

Manager::Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth)
{
    screenWidth = _screenWidth;
    screenHeight = _screenHeight;
    levelRadius = _levelRadius;
    levelOffset = _levelOffset;
    paddleBoundaryWidth = _paddleBoundaryWidth;

    lastUpdateTime = std::chrono::system_clock::now();
    lastDrawTime = std::chrono::system_clock::now();

    numPoints = 100;
    numGoalPoints = 20;

    Vector2 center = Vector2{ static_cast<float>(_screenWidth / 2), static_cast<float>(_screenHeight / 2) };
    _boundaryPoints = generateCirclePoints(numPoints, center, _levelRadius);

    Vector2 offset = Vector2{ static_cast<float>(_paddleBoundaryWidth / 2), 0 };
    Vector2 start = Vector2Subtract(center, offset);
    Vector2 end = Vector2Add(center, offset);
    _goalSections = generateGoalPoints(this, numGoalPoints, start.x, end.x, _levelRadius - 5);
    std::cout << start.x << "," << end.x << std::endl;
}

void Manager::addEntity(std::shared_ptr<Entity> entity)
{
    _entities[entity->id] = entity;
}

void Manager::deleteEntity(EntityId id)
{
    _entities.erase(id);
}

void Manager::addPlayer(std::shared_ptr<Player> player)
{
    players.push_back(player);
}

void Manager::update()
{
    //  Remove entities that need to be removed
    std::erase_if(_entities, [](const auto& pair) { return pair.second->destroyed; });

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastUpdateTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastUpdateTime = now;

    for (auto entry : _entities) {
        auto entity = entry.second;

        // TODO: Implement updating for player type
        if (entity->type == EntityType::PLAYER) {
            std::static_pointer_cast<Player>(entity)->update(this, screenWidth, screenHeight, dt);
        }
        else if (entity->type == EntityType::BALL) {
            std::static_pointer_cast<Ball>(entity)->update(this, screenWidth, screenHeight, dt);
        }
        else {
            entity->update();
        }
    }
}

void Manager::draw()
{
    DrawCircleLines(screenWidth / 2, screenHeight / 2, levelRadius, WHITE);
    // Draw goals on each side
    DrawLineStrip(&(_goalSections[0])[0], 20, RED);
    DrawLineStrip(&(_goalSections[1])[0], 20, BLUE);

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastDrawTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastDrawTime = now;

    for (auto entry : _entities) {
        auto entity = entry.second;
        // TODO: Implement drawing for player type
        if (entity->type == EntityType::PLAYER) {
            std::static_pointer_cast<Player>(entity)->draw();
        }
        else if (entity->type == EntityType::BALL) {
            std::static_pointer_cast<Ball>(entity)->draw();
        }
    }
}
