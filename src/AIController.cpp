#include "AIController.h"
#include "Player.h"
#include "Ball.h"
#include "Manager.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

AIController::AIController(int playerIndex, Difficulty difficulty)
    : aiPlayerIndex(playerIndex)
    , currentDifficulty(difficulty)
    , targetPosition({0, 0})
    , predictedBallPosition({0, 0})
    , reactionTimer(0.0f)
    , lastDecisionTime(0.0f)
    , ballMovingTowardsAI(false)
{
    updateDifficultyParams();
}

void AIController::setDifficulty(Difficulty difficulty)
{
    currentDifficulty = difficulty;
    updateDifficultyParams();
}

void AIController::updateDifficultyParams()
{
    switch (currentDifficulty) {
        case EASY:
            reactionTime = 0.15f;
            predictionAccuracy = 0.7f; 
            maxSpeed = 0.8f; 
            trackingRange = 1.2f; 
            break;
            
        case MEDIUM:
            reactionTime = 0.08f;
            predictionAccuracy = 0.9f;
            maxSpeed = 1.0f; 
            trackingRange = 2.0f; 
            break;
            
        case HARD:
            reactionTime = 0.05f; 
            predictionAccuracy = 0.98f;
            maxSpeed = 1.2f;  
            trackingRange = 3.0f;
            break;
    }
}

int AIController::update(Manager* manager, std::shared_ptr<Player> player, std::shared_ptr<Ball> ball, float deltaTime)
{
    reactionTimer += deltaTime;
    
    // Check if ball is moving towards
    ballMovingTowardsAI = isBallMovingTowardsPlayer(manager, player, ball);
    
    float actualReactionTime = reactionTime * 0.5f;
    
    if (reactionTimer < actualReactionTime) {
        return getMovementDirection(manager, player, targetPosition);
    }
    
    reactionTimer = 0.0f;
    lastDecisionTime = 0.0f;
    
    Vector2 interceptPoint = calculateInterceptPoint(manager, player, ball);
    
    float inaccuracy = (1.0f - predictionAccuracy) * 0.3f; 
    float randomOffset = (rand() / (float)RAND_MAX - 0.5f) * inaccuracy * 20.0f; 
    Vector2 center = {(float)manager->screenWidth / 2, (float)manager->screenHeight / 2};
    float radius = manager->levelRadius - manager->levelOffset;
    
    Vector2 toIntercept = Vector2Subtract(interceptPoint, center);
    float interceptAngle = atan2f(toIntercept.y, toIntercept.x);
    interceptAngle += randomOffset * (M_PI / 180.0f); 
    
    // Convert back to position
    targetPosition.x = center.x + radius * cosf(interceptAngle);
    targetPosition.y = center.y + radius * sinf(interceptAngle);
    
    return getMovementDirection(manager, player, targetPosition);
}

Vector2 AIController::predictBallTrajectory(std::shared_ptr<Ball> ball, float timeAhead)
{
    Vector2 pos = ball->position;
    Vector2 vel = ball->currentVelocity;
    float dt = 0.200f;
    float timeRemaining = timeAhead;
    
    while (timeRemaining > 0) {
        float stepTime = fminf(dt, timeRemaining);
        
        pos.x += vel.x * stepTime;
        pos.y += vel.y * stepTime;
        
        Vector2 center = {320, 240}; // Approximate center
        float radius = 200; // Approximate radius
        Vector2 toCenter = Vector2Subtract(pos, center);
        float distanceToCenter = Vector2Length(toCenter);
        
        if (distanceToCenter > radius) {
            // Ball hit boundary 
            Vector2 normal = Vector2Normalize(toCenter);
            vel = Vector2Reflect(vel, normal);
            // Move ball back inside
            pos = Vector2Add(center, Vector2Scale(normal, radius));
        }
        
        timeRemaining -= stepTime;
    }
    
    return pos;
}

Vector2 AIController::calculateInterceptPoint(Manager* manager, std::shared_ptr<Player> player, std::shared_ptr<Ball> ball)
{
    Vector2 center = {(float)manager->screenWidth / 2, (float)manager->screenHeight / 2};
    float radius = manager->levelRadius - manager->levelOffset;
    
    Vector2 ballPos = ball->position;
    Vector2 ballVel = ball->currentVelocity;
    float ballSpeed = Vector2Length(ballVel);
    
    // If ball is barely moving, just track its current position
    if (ballSpeed < 50.0f) {
        Vector2 toBall = Vector2Subtract(ballPos, center);
        float ballAngle = atan2f(toBall.y, toBall.x);
        
        Vector2 trackingPoint;
        trackingPoint.x = center.x + radius * cosf(ballAngle);
        trackingPoint.y = center.y + radius * sinf(ballAngle);
        return trackingPoint;
    }
    
    float bestTime = 0;
    Vector2 bestIntercept = ballPos;
    float minDistance = 10000;
    
    for (float t = 0.1f; t <= trackingRange; t += 0.1f) {
        Vector2 predictedPos = predictBallTrajectory(ball, t);
        
        Vector2 toPredicted = Vector2Subtract(predictedPos, center);
        float distance = Vector2Length(toPredicted);
        
        if (distance > 10.0f) { // Avoid division by zero
            Vector2 normalized = Vector2Scale(toPredicted, 1.0f / distance);
            Vector2 circlePoint = Vector2Add(center, Vector2Scale(normalized, radius));
            
            // Check if this is a good intercept point 
            Vector2 playerPos = player->position;
            float interceptDistance = Vector2Distance(playerPos, circlePoint);
            
            if (interceptDistance < minDistance) {
                minDistance = interceptDistance;
                bestIntercept = circlePoint;
                bestTime = t;
            }
        }
    }
    
    // If we found a good intercept, use it
    if (bestTime > 0) {
        return bestIntercept;
    }
    
    Vector2 toBall = Vector2Subtract(ballPos, center);
    float ballAngle = atan2f(toBall.y, toBall.x);
    
    Vector2 fallbackPoint;
    fallbackPoint.x = center.x + radius * cosf(ballAngle);
    fallbackPoint.y = center.y + radius * sinf(ballAngle);
    
    return fallbackPoint;
}

bool AIController::isBallMovingTowardsPlayer(Manager* manager, std::shared_ptr<Player> player, std::shared_ptr<Ball> ball)
{
    Vector2 ballVel = ball->currentVelocity;
    float ballSpeed = Vector2Length(ballVel);
    
    return ballSpeed > 10.0f;
}

float AIController::getAngleToTarget(Manager* manager, std::shared_ptr<Player> player, Vector2 target)
{
    Vector2 center = {(float)manager->screenWidth / 2, (float)manager->screenHeight / 2};
    
    Vector2 playerToCenter = Vector2Subtract(player->position, center);
    Vector2 targetToCenter = Vector2Subtract(target, center);
    
    float playerAngle = atan2f(playerToCenter.y, playerToCenter.x);
    float targetAngle = atan2f(targetToCenter.y, targetToCenter.x);
    
    float angleDiff = targetAngle - playerAngle;
    while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
    while (angleDiff < -M_PI) angleDiff += 2 * M_PI;
    
    return angleDiff;
}

int AIController::getMovementDirection(Manager* manager, std::shared_ptr<Player> player, Vector2 target)
{
    float angleToTarget = getAngleToTarget(manager, player, target);
    
    float threshold = 0.01f; 
    
    if (fabsf(angleToTarget) < threshold) {
        return 0;
    }
    
    float speedMultiplier = maxSpeed;
    
    if (aiPlayerIndex == 0) {
        return (angleToTarget > 0) ? 1 : -1;
    } else {
        return (angleToTarget > 0) ? -1 : 1;
    }
}

void AIController::drawDebugInfo(Manager* manager, std::shared_ptr<Player> player)
{
    DrawCircle(targetPosition.x, targetPosition.y, 8, RED);
    DrawCircleLines(targetPosition.x, targetPosition.y, 8, WHITE);
    
    Vector2 center = {(float)manager->screenWidth / 2, (float)manager->screenHeight / 2};
    DrawLine(center.x, center.y, predictedBallPosition.x, predictedBallPosition.y, YELLOW);
    
    const char* difficultyNames[] = {"EASY", "MEDIUM", "HARD"};
    const char* aiInfo = TextFormat("AI P%d: %s", aiPlayerIndex + 1, difficultyNames[currentDifficulty]);
    
    int textY = (aiPlayerIndex == 0) ? 100 : manager->screenHeight - 120;
    DrawText(aiInfo, 10, textY, 20, GREEN);
    
    const char* statusInfo = TextFormat("Tracking: %s", ballMovingTowardsAI ? "YES" : "NO");
    DrawText(statusInfo, 10, textY + 25, 16, GRAY);
}