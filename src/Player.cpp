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
    
    // Initialize AI control
    aiController = nullptr;
    isAIControlled = false;
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
    // Determine input source (human or AI)
    bool leftPressed = false;
    bool rightPressed = false;
    
    if (isAIControlled && aiController) {
        // Get AI input
        std::shared_ptr<Ball> ball = nullptr;
        // Find the ball in the manager's entities
        for (auto& entity : _manager->_entities) {
            if (entity.second->type == EntityType::BALL) {
                ball = std::static_pointer_cast<Ball>(entity.second);
                break;
            }
        }
        
        if (ball) {
            int aiInput = aiController->update(_manager, std::static_pointer_cast<Player>(shared_from_this()), ball, dt / 1000.0f);
            leftPressed = (aiInput == -1);
            rightPressed = (aiInput == 1);
        }
    } else {
        // Get human input
        leftPressed = std::any_of(binds.LEFT.begin(), binds.LEFT.end(), [](int v) { return IsKeyDown(v); });
        rightPressed = std::any_of(binds.RIGHT.begin(), binds.RIGHT.end(), [](int v) { return IsKeyDown(v); });
    }
    
    bool engineOn = leftPressed || rightPressed;
    float engineForce = engineOn ? force : 0;
    float resultantForce = engineForce - (frictionCoeff * normal);
    
    // Get current multiplier for speed boost
    float currentMultiplier = 1.0f;
    if (playerIndex >= 0 && playerIndex < _manager->playerMultipliers.size()) {
        currentMultiplier = _manager->playerMultipliers[playerIndex];
    }
    
    // Scale movement speed based on multiplier (1x = normal, 3x = 50% faster)
    float speedBonus = 1.0f + (currentMultiplier - 1.0f) * 0.25f; // 25% speed increase per multiplier level
    
    float forceMultiplier = 10.0f * speedBonus;
    float directionMultiplier = (playerIndex == 0) ? 1.0f : -1.0f;
    
    if (rightPressed) {
        angularVelocity -= (resultantForce * forceMultiplier * dt * directionMultiplier) / radius;
    }
    if (leftPressed) {
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

void Player::enableAI(AIController::Difficulty difficulty)
{
    // Find player index by checking manager's player list
    int playerIndex = -1;
    // This will be set by the manager when enabling AI
    
    aiController = std::make_unique<AIController>(playerIndex, difficulty);
    isAIControlled = true;
    
    printf("AI enabled for player with difficulty: %d\n", (int)difficulty);
}

void Player::disableAI()
{
    aiController.reset();
    isAIControlled = false;
    
    printf("AI disabled for player\n");
}

void Player::setAIDifficulty(AIController::Difficulty difficulty)
{
    if (aiController) {
        aiController->setDifficulty(difficulty);
        printf("AI difficulty set to: %d\n", (int)difficulty);
    }
}
