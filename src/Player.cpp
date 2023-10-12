#include "Player.h"
#include "Manager.h"
#include "raymath.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

Player::Player(Texture2D _spriteSheet, Vector2 _src, Vector2 _textureDims, Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity, float _force, float _frictionCoeff, float _normal, float _hp) :
    Entity(_position, _outputDims, _hitboxDims, EntityType::PLAYER)
{
    spriteSheet = _spriteSheet;
    textureDims = _textureDims;
    src = _src;
    
    currentVelocity = {0, 0};
    maxVelocity = _maxVelocity;
    force = _force;
    frictionCoeff = _frictionCoeff;
    normal = _normal;

    src = _src;
    hp = _hp;
}

//------------------------------------------------------------------------------------
// Out-Of-Bounds check
//------------------------------------------------------------------------------------
bool Player::outOfBounds(Manager* _manager, Vector2 entity, int screenWidth, int screenHeight)
{
    if (entity.x <= (float)(_manager->screenWidth/2) - _manager->paddleBoundaryWidth/2 || entity.x >= (float)(_manager->screenWidth/2) + _manager->paddleBoundaryWidth/2)
    {
        return true;
    }
    return false;
}


//------------------------------------------------------------------------------------
// Update player
//------------------------------------------------------------------------------------
void Player::update(Manager* _manager, int _screenWidth, int _screenHeight, float dt)
{
    Vector2 oldShipPosition = position;

    bool engineOn = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) ||
                    IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);

    Vector2 resultantVelocity = currentVelocity;

    float engineForce = engineOn ? force : 0;

    float resultantForce = engineForce - (frictionCoeff * normal);

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        resultantVelocity.x += ((float)resultantForce * dt);
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        resultantVelocity.x -= ((float)resultantForce * dt);
        std::cout << "x: " << position.x << ", y: " << position.y << std::endl; 

    if (!engineOn)
    {
        Vector2 resultantForceVec = Vector2Normalize(resultantVelocity);
        resultantForceVec.x *= resultantForce;
        resultantForceVec.y *= resultantForce;
        resultantVelocity.x += resultantForceVec.x * dt;
        resultantVelocity.y += resultantForceVec.y * dt;

        // stop moving by friction
        if (currentVelocity.x > 0 && resultantVelocity.x <= 0)
        {
            resultantVelocity.x = 0;
        }
        else if (currentVelocity.x < 0 && resultantVelocity.x >= 0)
        {
            resultantVelocity.x = 0;
        }

        if (currentVelocity.y > 0 && resultantVelocity.y <= 0)
        {
            resultantVelocity.y = 0;
        }
        else if (currentVelocity.y < 0 && resultantVelocity.y >= 0)
        {
            resultantVelocity.y = 0;
        }
    }

    float magVel = Vector2Length(resultantVelocity);
    // resultantVelocity = Vector2Normalize(resultantVelocity);

    if (magVel >= maxVelocity)
    {
        resultantVelocity = Vector2Normalize(resultantVelocity);
        resultantVelocity.x *= maxVelocity;
        resultantVelocity.y *= maxVelocity;
    }

    Vector2 positionDelta = {resultantVelocity.x * dt, resultantVelocity.y * dt};
    position = Vector2Add(position, positionDelta);

    float num = std::powf(_manager->levelRadius - _manager->levelOffset, 2)
            - std::powf(position.x - (float)(_manager->screenWidth/2), 2);


    float _y = (position.y - _manager->screenHeight/2);
    float _x = (position.x - _manager->screenWidth/2);

    if(position.y >= _manager->screenHeight/2)
    {
        position.y = std::sqrt
        (
            std::powf(_manager->levelRadius - _manager->levelOffset, 2)
            - std::powf(position.x - (float)(_manager->screenWidth/2), 2)
        ) + (float)(_manager->screenHeight/2);

        rotation = -((180/M_PI) * std::atan(-_y/_x)) - 90.0f;
        if(position.x < _manager->screenWidth/2)
        {
            rotation += 180.0f;
        }
    }
//     else
//     {
// 
//         position.y = - std::sqrt
//         (
//             std::powf(_manager->levelRadius - _manager->levelOffset, 2)
//             - std::powf(position.x - (float)(_manager->screenWidth/2), 2)
//         ) + (float)(_manager->screenHeight/2);
// 
//         // rotation = -((180/M_PI) * std::atan(-_y/_x)) - 90.f;
//     }

    // TODO: ADD GLOBAL HITBOX VAR
    // if (IsKeyDown(KEY_H)) showHitboxes = !showHitboxes;

    if (outOfBounds(_manager, position, _screenWidth, _screenHeight))
    {
        position = oldShipPosition;
    }

    currentVelocity = resultantVelocity;
}

void Player::draw()
{
    Rectangle srcRec = {src.x, src.y, textureDims.x, textureDims.y};
    Rectangle destRec = {position.x, position.y, outputDims.x, outputDims.y};
    Vector2 origin = {(float)outputDims.x / 2, (float)outputDims.y / 2};
    DrawTexturePro(spriteSheet, srcRec, destRec, origin, rotation, WHITE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
