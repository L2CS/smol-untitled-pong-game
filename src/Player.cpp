#include "Player.h"
#include "Manager.h"
#include "raymath.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>

Player::Player(Texture2D _spriteSheet, Vector2 _src, Vector2 _textureDims, Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity, float _force, float _frictionCoeff, float _normal, float _hp, Keybinds _binds)
    : Entity(_position, _outputDims, _hitboxDims, EntityType::PLAYER)
{
    spriteSheet = _spriteSheet;
    textureDims = _textureDims;
    src = _src;

    currentVelocity = { 0, 0 };
    maxVelocity = _maxVelocity;
    force = _force;
    frictionCoeff = _frictionCoeff;
    normal = _normal;

    src = _src;
    hp = _hp;
    binds = _binds;
    angularVelocity = 0.0f; // Initialize angular velocity
}

/**
 * Out-of-bounds check
 */
bool Player::outOfBounds(Manager* _manager, Vector2 _position, int playerIndex)
{
    Vector2 center = { (float)(_manager->screenWidth / 2), (float)(_manager->screenHeight / 2) };
    float radius = _manager->levelRadius - _manager->levelOffset;
    
    // Check if player is at the correct distance from center
    float distanceFromCenter = Vector2Distance(_position, center);
    if (fabsf(distanceFromCenter - radius) > 5.0f) { 
        return true;
    }
    
    return false;
}

/**
 * Update Player
 */
void Player::update(Manager* _manager, int _screenWidth, int _screenHeight, float dt, int playerIndex)
{
    Vector2 center = { (float)(_screenWidth / 2), (float)(_screenHeight / 2) };
    float radius = _manager->levelRadius - _manager->levelOffset;
    
    // Get current angle on the circle
    Vector2 toPlayer = Vector2Subtract(position, center);
    float currentAngle = atan2f(toPlayer.y, toPlayer.x);
    
    // Use the member variable for angular velocity
    bool engineOn = std::any_of(binds.LEFT.begin(), binds.LEFT.end(), [](int v) { return IsKeyDown(v); }) || 
                    std::any_of(binds.RIGHT.begin(), binds.RIGHT.end(), [](int v) { return IsKeyDown(v); });

    float engineForce = engineOn ? force : 0;

    float resultantForce = engineForce - (frictionCoeff * normal);
    
    float forceMultiplier = 10.0f;
    float directionMultiplier = (playerIndex == 0) ? 1.0f : -1.0f;
    
    if (std::any_of(binds.RIGHT.begin(), binds.RIGHT.end(), [](int v) { return IsKeyDown(v); })) {
        angularVelocity -= (resultantForce * forceMultiplier * dt * directionMultiplier) / radius;
    }
    if (std::any_of(binds.LEFT.begin(), binds.LEFT.end(), [](int v) { return IsKeyDown(v); })) {
        angularVelocity += (resultantForce * forceMultiplier * dt * directionMultiplier) / radius;
    }

    // Apply friction to angular velocity 
    if (!engineOn && fabsf(angularVelocity) > 0.001f) {
        float frictionMultiplier = 3.0f; // hmm we can maybe make this into game mech? 
        float frictionForce = frictionCoeff * normal * forceMultiplier * frictionMultiplier;
        float angularFriction = (frictionForce * dt) / radius;
        
        if (angularVelocity > 0) {
            angularVelocity -= angularFriction;
            if (angularVelocity < 0) angularVelocity = 0;
        } else if (angularVelocity < 0) {
            angularVelocity += angularFriction;
            if (angularVelocity > 0) angularVelocity = 0;
        }
    }

    // Limit angular velocity
    float maxAngularVel = (maxVelocity * forceMultiplier) / radius;
    angularVelocity = fmaxf(-maxAngularVel, fminf(maxAngularVel, angularVelocity));

    // Calculate new angle
    float newAngle = currentAngle + angularVelocity;
    
    // Get boundary constraints for this player
    float dynamicBoundaryWidth = _manager->getPlayerBoundaryWidth(playerIndex);
    float maxAngleRange = fminf(dynamicBoundaryWidth / radius, (float)M_PI); // Cap at semicircle
    
    // Define center angles for each player's allowed arc
    float centerAngle = (playerIndex == 0) ? (float)M_PI / 2 : 3 * (float)M_PI / 2; // Top or bottom
    
    // Normalize angles to handle wrap-around
    while (newAngle < 0) newAngle += 2 * (float)M_PI;
    while (newAngle >= 2 * (float)M_PI) newAngle -= 2 * (float)M_PI;
    
    // Calculate angular distance from center of allowed arc
    float angleDiff = newAngle - centerAngle;
    while (angleDiff > (float)M_PI) angleDiff -= 2 * (float)M_PI;
    while (angleDiff < -(float)M_PI) angleDiff += 2 * (float)M_PI;
    
    // Clamp to allowed range
    float maxHalfRange = maxAngleRange / 2;
    if (angleDiff > maxHalfRange) {
        newAngle = centerAngle + maxHalfRange;
        angularVelocity = 0; // Stop at boundary
    } else if (angleDiff < -maxHalfRange) {
        newAngle = centerAngle - maxHalfRange;
        angularVelocity = 0; // Stop at boundary
    }
    
    // Calculate new position on the circle
    position.x = center.x + radius * cosf(newAngle);
    position.y = center.y + radius * sinf(newAngle);
    
    // The sprite should point along the tangent of the circle
    rotation = (newAngle * 180.0f / (float)M_PI) - 90.0f;
    
    // Store velocity for physics calculations
    currentVelocity.x = angularVelocity * radius;
    currentVelocity.y = 0; 
}

void Player::draw()
{
    Rectangle srcRec = { src.x, src.y, textureDims.x, textureDims.y };
    Rectangle destRec = { position.x, position.y, outputDims.x, outputDims.y };
    Vector2 origin = { (float)outputDims.x / 2, (float)outputDims.y / 2 };
    DrawTexturePro(spriteSheet, srcRec, destRec, origin, rotation, WHITE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
