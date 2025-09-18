#pragma once

#include "raylib.h"
#include <memory>

struct Player;
struct Ball;
struct Manager;

class AIController {
public:
    enum Difficulty {
        EASY = 0,
        MEDIUM = 1,
        HARD = 2
    };
    
    AIController(int playerIndex, Difficulty difficulty = MEDIUM);
    
    int update(Manager* manager, std::shared_ptr<Player> player, std::shared_ptr<Ball> ball, float deltaTime);
    
    // Configuration
    void setDifficulty(Difficulty difficulty);
    Difficulty getDifficulty() const { return currentDifficulty; }
    
    // Debug info
    void drawDebugInfo(Manager* manager, std::shared_ptr<Player> player);

private:
    int aiPlayerIndex;
    Difficulty currentDifficulty;
    
    // AI state
    Vector2 targetPosition;
    Vector2 predictedBallPosition;
    float reactionTimer;
    float lastDecisionTime;
    bool ballMovingTowardsAI;
    
    float reactionTime;
    float predictionAccuracy;
    float maxSpeed;
    float trackingRange;
    
    // Internal methods
    void updateDifficultyParams();
    Vector2 predictBallTrajectory(std::shared_ptr<Ball> ball, float timeAhead);
    Vector2 calculateInterceptPoint(Manager* manager, std::shared_ptr<Player> player, std::shared_ptr<Ball> ball);
    bool isBallMovingTowardsPlayer(Manager* manager, std::shared_ptr<Player> player, std::shared_ptr<Ball> ball);
    float getAngleToTarget(Manager* manager, std::shared_ptr<Player> player, Vector2 target);
    int getMovementDirection(Manager* manager, std::shared_ptr<Player> player, Vector2 target);
};