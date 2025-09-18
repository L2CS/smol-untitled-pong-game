#pragma once

#include "Ball.h"
#include "helpers.h"
#include "MusicManager.h"
#include "OsuParser.h"
#include "Player.h"
#include "Visualizer.h"

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
    float baseLevelRadius; // Original radius for scaling calculations
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

    // Scoring system
    std::vector<int> playerScores;
    std::vector<float> playerMultipliers; // Current score multipliers for each player
    std::vector<float> lastHitTimes;      // Last time each player hit the ball

    // Osu! beatmap integration
    OsuParser osuParser;
    std::vector<float> hitObjectTimes;

    // Hit feedback system
    std::vector<int> consecutiveHits;         // Track consecutive on-beat hits per player
    std::vector<float> lastHitFeedbackTime;   // When to stop showing feedback
    std::vector<std::string> hitFeedbackText; // Current feedback text per player
    std::vector<Color> hitFeedbackColor;      // Feedback text color per player

public:
    // Modular components
    MusicManager musicManager;
    Visualizer visualizer;

    // Background and game state
    Texture2D backgroundTexture;
    float gameStartTime;
    bool gameEnded;
    int winner; // 0 = Player 1, 1 = Player 2, -1 = tie

    // TODO: write handlers for adding powerups
    // std::vector<Powerup*> powerupsToAdd;

    // Constructor and Destructor
    Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth, Texture2D _backgroundTexture, const char* _musicFile, const char* _osuFile = nullptr);
    ~Manager();

    // Member functions for managing entities
    void addEntity(std::shared_ptr<Entity> entity);
    void deleteEntity(EntityId id);
    void addPlayer(std::shared_ptr<Player> player);
    void update();
    void draw();

    // Scoring functions
    void scoreGoal(int playerIndex);
    int getPlayerScore(int playerIndex);
    float getPlayerBoundaryWidth(int playerIndex);

    // Game state functions
    void setSongStartTime();
    float getCurrentBeatIntensity() const;
    bool isGameEnded() const;
    int getWinner() const;
    void checkGameEnd();
    void playHitSound();
    void onPaddleHit();
    void onNonPaddleHit();
    void triggerVisualizerBeat(float intensity);

    // Score multiplier system
    void onPlayerHitBall(int playerIndex);
    float calculateMultiplier(int playerIndex, float hitTime);
    void updateMultipliers();
    void drawMultipliers();

    // Hit timing visual cues
    void drawHitTimingCues();
    void showHitFeedback(int playerIndex, float accuracy);
    void drawHitFeedback();
    Color getBorderColorForTiming();
    std::vector<float> getUpcomingHitObjects(float currentTime, float lookAheadTime = 2000.0f);

    // AI management
    void enableAI(int playerIndex, AIController::Difficulty difficulty = AIController::MEDIUM);
    void disableAI(int playerIndex);
    void setAIDifficulty(int playerIndex, AIController::Difficulty difficulty);

public:
    // Map of entities
    EntityMap _entities;

    // Level boundary points
    std::vector<Vector2> _boundaryPoints;

    // Goal points sections
    std::array<std::vector<Vector2>, 2> _goalSections;
    int numGoalPoints;
};
