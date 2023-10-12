#include "Manager.h"

#include <algorithm>
#include <chrono>

Manager::Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth)
{
    screenWidth = _screenWidth;
    screenHeight = _screenHeight;
    levelRadius = _levelRadius;
    levelOffset = _levelOffset;
    paddleBoundaryWidth = _paddleBoundaryWidth;

    lastUpdateTime = std::chrono::system_clock::now();
    lastDrawTime = std::chrono::system_clock::now();
}

void Manager::addEntity(Entity* _entity)
{
    entities[_entity->id] = _entity;
}

void Manager::deleteEntity(EntityId _id)
{
    entities.erase(_id);
}

void Manager::update()
{
    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastUpdateTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastUpdateTime = now;

    for (auto entry : entities) {
        auto entity = entry.second;

        // TODO: implement updating for player type
        if (entity->type == EntityType::PLAYER) {
            Player* player = static_cast<Player*>(entity);
            player->update(this, screenWidth, screenHeight, dt);
        }
        else {
            entity->update();
        }
    }
}

void Manager::draw()
{
    DrawCircleLines(screenWidth / 2, screenHeight / 2, levelRadius, WHITE);

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastDrawTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastDrawTime = now;

    for (auto entry : entities) {
        auto entity = entry.second;
        // TODO: implement drawing for player type
        if (entity->type == EntityType::PLAYER) {
            Player* player = static_cast<Player*>(entity);
            player->draw();
        }
        //         else
        //         {
        //             entity->draw();
        //         }
    }
}
