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
    boundaryPoints = generateCirclePoints(numPoints, center, _levelRadius);

    Vector2 offset = Vector2{ static_cast<float>(_paddleBoundaryWidth / 2), 0 };
    Vector2 start = Vector2Subtract(center, offset);
    Vector2 end = Vector2Add(center, offset);
    goalSections = generateGoalPoints(this, numGoalPoints, start.x, end.x, _levelRadius - 5);
    std::cout << start.x << "," << end.x << std::endl;
}

void Manager::addEntity(Entity* _entity)
{
    entities[_entity->id] = _entity;
}

void Manager::deleteEntity(EntityId _id)
{
    entities.erase(_id);
}

void Manager::addPlayer(Player* player)
{
    players.push_back(player);
}

void Manager::update()
{
    //  Remove entities that need to be removed
    std::erase_if(entities, [](const auto& pair) { return pair.second->destroyed; });

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastUpdateTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastUpdateTime = now;

    for (auto entry : entities) {
        auto entity = entry.second;

        // TODO: Implement updating for player type
        if (entity->type == EntityType::PLAYER) {
            Player* player = static_cast<Player*>(entity);
            player->update(this, screenWidth, screenHeight, dt);
        }
        else if (entity->type == EntityType::BALL) {
            Ball* ball = static_cast<Ball*>(entity);
            ball->update(this, screenWidth, screenHeight, dt);
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
    DrawLineStrip(goalSections[0], 20, RED);
    DrawLineStrip(goalSections[1], 20, BLUE);

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastDrawTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastDrawTime = now;

    for (auto entry : entities) {
        auto entity = entry.second;
        // TODO: Implement drawing for player type
        if (entity->type == EntityType::PLAYER) {
            Player* player = static_cast<Player*>(entity);
            player->draw();
        }
        else if (entity->type == EntityType::BALL) {
            Ball* ball = static_cast<Ball*>(entity);
            ball->draw();
        }
    }
}
