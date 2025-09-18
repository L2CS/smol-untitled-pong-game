#include "Manager.h"

#include <algorithm>
#include <chrono>
#include <iostream>

Manager::Manager(int _screenWidth, int _screenHeight, float _levelRadius, float _levelOffset, float _paddleBoundaryWidth, Texture2D _backgroundTexture, const char* _musicFile, const char* _osuFile)
    : musicManager(), visualizer(_screenWidth, _screenHeight, _levelRadius)
{
    screenWidth = _screenWidth;
    screenHeight = _screenHeight;
    levelRadius = _levelRadius;
    baseLevelRadius = _levelRadius; // Store original radius
    levelOffset = _levelOffset;
    paddleBoundaryWidth = _paddleBoundaryWidth;

    lastUpdateTime = std::chrono::system_clock::now();
    lastDrawTime = std::chrono::system_clock::now();

    numPoints = 100;
    numGoalPoints = 20;

    // Initialize scoring system
    playerScores = { 0, 0 };               // Two players start with 0 score
    playerMultipliers = { 1.0f, 1.0f };    // Start with 1x multiplier
    lastHitTimes = { -1000.0f, -1000.0f }; // Initialize to very old times

    // Initialize hit feedback system
    consecutiveHits = { 0, 0 };
    lastHitFeedbackTime = { 0.0f, 0.0f };
    hitFeedbackText = { "", "" };
    hitFeedbackColor = { WHITE, WHITE };

    Vector2 center = Vector2{ static_cast<float>(_screenWidth / 2), static_cast<float>(_screenHeight / 2) };
    _boundaryPoints = generateCirclePoints(numPoints, center, _levelRadius);

    Vector2 offset = Vector2{ static_cast<float>(_paddleBoundaryWidth / 2), 0 };
    Vector2 start = Vector2Subtract(center, offset);
    Vector2 end = Vector2Add(center, offset);
    _goalSections = generateGoalPoints(this, numGoalPoints, start.x, end.x, _levelRadius - 5);
    std::cout << start.x << "," << end.x << std::endl;

    backgroundTexture = _backgroundTexture;
    gameEnded = false;
    winner = -1;

    // Initialize music manager
    musicManager.initializeSounds();
    musicManager.loadAudioFile(_musicFile);

    // Parse osu! beatmap if provided (after music manager is initialized)
    if (_osuFile && osuParser.parseFile(_osuFile)) {
        hitObjectTimes = osuParser.getHitObjectTimes();

        // Print detailed timing information
        osuParser.printTimingInfo();

        printf("Loaded %zu hit objects for score multiplier system\n", hitObjectTimes.size());

        // Pass timing points to music manager for accurate beat detection
        musicManager.loadBeatMapFromOsu(osuParser.timingPoints);
    }
    else {
        printf("No osu! beatmap loaded - using default scoring and timing\n");
    }
}

Manager::~Manager()
{
    // Cleanup is handled by component destructors
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
    // Check if game should end first
    checkGameEnd();

    // If game has ended, only update music manager for final audio processing
    if (gameEnded) {
        auto now = std::chrono::system_clock::now();
        auto elapsed = now - lastUpdateTime;
        float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        lastUpdateTime = now;

        // Only update music manager to handle final audio state
        musicManager.update(dt / 1000.0f);
        return; // Skip all game logic
    }

    //  Remove entities that need to be removed
    std::erase_if(_entities, [](const auto& pair) { return pair.second->destroyed; });

    auto now = std::chrono::system_clock::now();
    auto elapsed = now - lastUpdateTime;
    float dt = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    lastUpdateTime = now;

    // Update music manager first to get current beat intensity
    musicManager.update(dt / 1000.0f); // Convert ms to seconds

    // Scale arena based on beat intensity
    float beatIntensity = musicManager.getCurrentBeatIntensity();
    float scaleMultiplier = 1.0f + (beatIntensity * 0.5f); // Scale up to 10% larger on strong beats
    levelRadius = baseLevelRadius * scaleMultiplier;

    // Update visualizer with new radius
    visualizer.updateRadius(levelRadius);

    // Update boundary points and goal sections with new radius
    Vector2 center = Vector2{ static_cast<float>(screenWidth / 2), static_cast<float>(screenHeight / 2) };
    _boundaryPoints = generateCirclePoints(numPoints, center, levelRadius);

    Vector2 offset = Vector2{ static_cast<float>(paddleBoundaryWidth / 2), 0 };
    Vector2 start = Vector2Subtract(center, offset);
    Vector2 end = Vector2Add(center, offset);
    _goalSections = generateGoalPoints(this, numGoalPoints, start.x, end.x, levelRadius - 5);

    for (auto entry : _entities) {
        auto entity = entry.second;

        // TODO: Implement updating for player type
        if (entity->type == EntityType::PLAYER) {
            // Find player index
            int playerIndex = -1;
            for (int i = 0; i < players.size(); i++) {
                if (players[i] == entity) {
                    playerIndex = i;
                    break;
                }
            }
            std::static_pointer_cast<Player>(entity)->update(this, screenWidth, screenHeight, dt, playerIndex);
        }
        else if (entity->type == EntityType::BALL) {
            std::static_pointer_cast<Ball>(entity)->update(this, screenWidth, screenHeight, dt);
        }
        else {
            entity->update();
        }
    }

    // Update visualizer with audio data
    visualizer.update(musicManager);

    // Update multiplier system
    updateMultipliers();
}

void Manager::draw()
{
    // Draw background image if loaded
    if (backgroundTexture.id != 0) {
        // Scale background to fit screen
        float scaleX = (float)screenWidth / backgroundTexture.width;
        float scaleY = (float)screenHeight / backgroundTexture.height;
        float scale = fmaxf(scaleX, scaleY); // Use larger scale to fill screen

        Rectangle sourceRec = { 0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height };
        Rectangle destRec = {
            (screenWidth - backgroundTexture.width * scale) / 2,
            (screenHeight - backgroundTexture.height * scale) / 2,
            backgroundTexture.width * scale,
            backgroundTexture.height * scale
        };

        // Draw with slight transparency to not overpower the game
        DrawTexturePro(backgroundTexture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, (Color){ 255, 255, 255, 180 });
    }

    // Draw audio visualizer on top
    visualizer.draw();

    // Arena border is now drawn in drawHitTimingCues() with dynamic color

    // Draw goals on each side with thicker lines
    for (int i = 0; i < 3; i++) {
        // Draw multiple offset lines to create thickness
        for (int j = 0; j < _goalSections[0].size() - 1; j++) {
            Vector2 p1 = _goalSections[0][j];
            Vector2 p2 = _goalSections[0][j + 1];
            DrawLineEx(p1, p2, 3.0f, RED);
        }
        for (int j = 0; j < _goalSections[1].size() - 1; j++) {
            Vector2 p1 = _goalSections[1][j];
            Vector2 p2 = _goalSections[1][j + 1];
            DrawLineEx(p1, p2, 3.0f, BLUE);
        }
    }

    // Draw scores (goals scored by each player)
    // Player 1 (index 0) score at top
    DrawText(TextFormat("Player 1 Score: %d", getPlayerScore(0)), screenWidth / 2 - 80, 20, 20, WHITE);
    // Player 2 (index 1) score at bottom
    DrawText(TextFormat("Player 2 Score: %d", getPlayerScore(1)), screenWidth / 2 - 80, screenHeight - 40, 20, WHITE);

    // Draw game end screen if game is over
    if (gameEnded) {
        // Semi-transparent overlay
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 0, 0, 0, 180 });

        // Winner announcement
        const char* winnerText;
        Color winnerColor;
        if (winner == 0) {
            winnerText = "PLAYER 1 WINS!";
            winnerColor = (Color){ 255, 100, 100, 255 }; // Light red
        }
        else if (winner == 1) {
            winnerText = "PLAYER 2 WINS!";
            winnerColor = (Color){ 100, 100, 255, 255 }; // Light blue
        }
        else {
            winnerText = "IT'S A TIE!";
            winnerColor = (Color){ 255, 255, 100, 255 }; // Yellow
        }

        // Large winner text
        int fontSize = 60;
        int textWidth = MeasureText(winnerText, fontSize);
        DrawText(winnerText, screenWidth / 2 - textWidth / 2, screenHeight / 2 - 60, fontSize, winnerColor);

        // Final scores
        const char* finalScoreText = TextFormat("Final Score - Player 1: %d  Player 2: %d",
                                                getPlayerScore(0), getPlayerScore(1));
        int scoreWidth = MeasureText(finalScoreText, 30);
        DrawText(finalScoreText, screenWidth / 2 - scoreWidth / 2, screenHeight / 2 + 20, 30, WHITE);

        // Exit instruction
        const char* exitText = "Press ESC to exit";
        int exitWidth = MeasureText(exitText, 20);
        DrawText(exitText, screenWidth / 2 - exitWidth / 2, screenHeight / 2 + 80, 20, GRAY);
    }

    // Draw score multipliers
    drawMultipliers();

    // Draw hit timing visual cues
    drawHitTimingCues();

    // Draw hit feedback
    drawHitFeedback();

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

void Manager::scoreGoal(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < playerScores.size()) {
        // Use multiplier value directly as score increment
        float multiplier = playerMultipliers[playerIndex];
        int scoreIncrement = (int)roundf(multiplier); // Round to nearest integer

        playerScores[playerIndex] += scoreIncrement;

        std::cout << "Player " << (playerIndex + 1) << " scored! ";
        std::cout << "Multiplier: " << multiplier << "x = +" << scoreIncrement << " points";
        std::cout << " Total Score: " << playerScores[playerIndex] << std::endl;

        // Reset multiplier after scoring
        playerMultipliers[playerIndex] = 1.0f;

        // Trigger big visualizer beat for goals
        triggerVisualizerBeat(2.0f);
    }
}

int Manager::getPlayerScore(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < playerScores.size()) {
        return playerScores[playerIndex];
    }
    return 0;
}

float Manager::getPlayerBoundaryWidth(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < playerScores.size()) {
        // Use the actual player movement radius (levelRadius - levelOffset)
        float playerRadius = levelRadius - levelOffset;
        // Calculate the arc length of a semicircle: π * radius
        float maxArcLength = M_PI * playerRadius; // Maximum is the arc length of semicircle

        // Each goal scored by the opponent adds 50 units of movement range (compensation)
        float expansionPerGoal = 50.0f;
        int opponentIndex = (playerIndex == 0) ? 1 : 0;
        float totalExpansion = playerScores[opponentIndex] * expansionPerGoal;
        float newWidth = paddleBoundaryWidth + totalExpansion;

        // Cap the boundary width to the arc length of semicircle
        float finalWidth = fminf(newWidth, maxArcLength);

        // Debug output (commented out)
        // std::cout << "Player " << (playerIndex + 1) << " - Goals against: " << playerScores[playerIndex]
        //           << ", Base width: " << paddleBoundaryWidth
        //           << ", Expansion: " << totalExpansion
        //           << ", New width: " << newWidth
        //           << ", Max arc length: " << maxArcLength
        //           << ", Final width: " << finalWidth << std::endl;

        return finalWidth;
    }
    return paddleBoundaryWidth;
}

void Manager::triggerVisualizerBeat(float intensity)
{
    visualizer.triggerBeat(intensity);
}

void Manager::setSongStartTime()
{
    musicManager.setSongStartTime();
}

float Manager::getCurrentBeatIntensity() const
{
    return musicManager.getCurrentBeatIntensity();
}

bool Manager::isGameEnded() const
{
    return gameEnded;
}

int Manager::getWinner() const
{
    return winner;
}

void Manager::checkGameEnd()
{
    if (gameEnded) return; // Already ended

    // Check if music has finished using MusicManager
    if (musicManager.isGameEnded()) {
        gameEnded = true;

        // Determine winner based on scores
        int player1Score = getPlayerScore(0);
        int player2Score = getPlayerScore(1);

        if (player1Score > player2Score) {
            winner = 0; // Player 1 wins (more goals scored = better)
        }
        else if (player2Score > player1Score) {
            winner = 1; // Player 2 wins (more goals scored = better)
        }
        else {
            winner = -1; // Tie
        }

        printf("Game ended! Music duration: %.1f seconds\n", musicManager.getMusicDuration());
        printf("Final scores - Player 1: %d, Player 2: %d\n", player1Score, player2Score);
        if (winner >= 0) {
            printf("Winner: Player %d\n", winner + 1);
        }
        else {
            printf("Game ended in a tie!\n");
        }
    }
}
void Manager::playHitSound()
{
    musicManager.playHitSound();
}

void Manager::onPaddleHit()
{
    musicManager.onPaddleHit();
}

void Manager::onNonPaddleHit()
{
    musicManager.onNonPaddleHit();
}

void Manager::onPlayerHitBall(int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= playerMultipliers.size()) return;

    float currentTime = musicManager.getCurrentSongTime() * 1000.0f; // Convert to milliseconds
    lastHitTimes[playerIndex] = currentTime;

    // Calculate new multiplier based on timing
    float newMultiplier = calculateMultiplier(playerIndex, currentTime);
    playerMultipliers[playerIndex] = newMultiplier;

    // Find accuracy for feedback
    float accuracy = 1000000.0f;
    if (!hitObjectTimes.empty()) {
        for (float objTime : hitObjectTimes) {
            float distance = fabsf(currentTime - objTime);
            if (distance < accuracy) {
                accuracy = distance;
            }
        }
    }

    // Show hit feedback
    showHitFeedback(playerIndex, accuracy);

    printf("Player %d hit at %.1fms - Multiplier: %.1fx - Accuracy: %.1fms\n",
           playerIndex + 1, currentTime, newMultiplier, accuracy);
}

float Manager::calculateMultiplier(int playerIndex, float hitTime)
{
    if (hitObjectTimes.empty()) {
        return 1.0f; // No beatmap data, default multiplier
    }

    // Find the closest hit object time
    float closestDistance = 1000000.0f;
    for (float objTime : hitObjectTimes) {
        float distance = fabsf(hitTime - objTime);
        if (distance < closestDistance) {
            closestDistance = distance;
        }
    }

    // Calculate multiplier based on timing accuracy
    // Perfect timing (within 50ms) = 3x multiplier
    // Good timing (within 100ms) = 2x multiplier
    // Okay timing (within 200ms) = 1.5x multiplier
    // Otherwise = 1x multiplier

    if (closestDistance <= 50.0f) {
        return 3.0f; // Perfect!
    }
    else if (closestDistance <= 100.0f) {
        return 2.0f; // Great!
    }
    else if (closestDistance <= 200.0f) {
        return 1.5f; // Good!
    }
    else {
        return 1.0f; // Normal
    }
}

void Manager::updateMultipliers()
{
    float currentTime = musicManager.getCurrentSongTime() * 1000.0f;

    // Decay multipliers over time if no recent hits
    for (int i = 0; i < playerMultipliers.size(); i++) {
        float timeSinceHit = currentTime - lastHitTimes[i];

        // Multiplier decays after 2 seconds of no hits
        if (timeSinceHit > 2000.0f) {
            playerMultipliers[i] = fmaxf(1.0f, playerMultipliers[i] - 0.01f);
        }
    }
}

void Manager::drawMultipliers()
{
    // Draw multipliers for each player
    for (int i = 0; i < playerMultipliers.size(); i++) {
        float multiplier = playerMultipliers[i];

        // Choose color based on multiplier level
        Color color = WHITE;
        if (multiplier >= 3.0f) {
            color = (Color){ 255, 215, 0, 255 }; // Gold
        }
        else if (multiplier >= 2.0f) {
            color = (Color){ 255, 165, 0, 255 }; // Orange
        }
        else if (multiplier >= 1.5f) {
            color = (Color){ 255, 255, 0, 255 }; // Yellow
        }

        // Position multipliers on sides of screen
        int x = (i == 0) ? 50 : screenWidth - 150;
        int y = screenHeight / 2;

        // Draw multiplier text
        const char* multiplierText = TextFormat("%.1fx", multiplier);
        int fontSize = 40;
        int textWidth = MeasureText(multiplierText, fontSize);

        // Draw background circle with glow effect for speed bonus (increased size)
        int circleRadius = 50; // Increased from 35 to 50
        DrawCircle(x + textWidth / 2, y, circleRadius, (Color){ 0, 0, 0, 150 });

        // Add glow effect when speed is boosted
        if (multiplier > 1.0f) {
            float glowIntensity = (multiplier - 1.0f) / 2.0f; // 0 to 1 based on multiplier
            glowIntensity = fminf(glowIntensity, 1.0f);

            // Draw multiple circles for glow effect
            for (int glow = 0; glow < 3; glow++) {
                Color glowColor = color;
                glowColor.a = (unsigned char)(50 * glowIntensity * (3 - glow) / 3);
                DrawCircleLines(x + textWidth / 2, y, circleRadius + glow * 3, glowColor);
            }
        }

        DrawCircleLines(x + textWidth / 2, y, circleRadius, color);

        // Draw multiplier text
        DrawText(multiplierText, x, y - fontSize / 2, fontSize, color);

        // Draw player label with AI indicator (increased size and padding)
        const char* playerLabel;
        if (i < players.size() && players[i]->isAIControlled) {
            playerLabel = TextFormat("AI%d", i + 1);
        }
        else {
            playerLabel = TextFormat("P%d", i + 1);
        }
        int labelFontSize = 24; // Increased from 20 to 24
        int labelWidth = MeasureText(playerLabel, labelFontSize);
        int labelY = y + 65; // Increased padding from 45 to 65
        DrawText(playerLabel, x + textWidth / 2 - labelWidth / 2, labelY, labelFontSize,
                 (i < players.size() && players[i]->isAIControlled) ? GREEN : GRAY);

        // Draw speed bonus indicator (increased size and padding)
        if (multiplier > 1.0f) {
            float speedBonus = 1.0f + (multiplier - 1.0f) * 0.25f;
            const char* speedText = TextFormat("Speed: %.0f%%", speedBonus * 100);
            int speedFontSize = 20; // Increased from 16 to 20
            int speedWidth = MeasureText(speedText, speedFontSize);
            int speedY = labelY + 35; // More padding between label and speed text
            DrawText(speedText, x + textWidth / 2 - speedWidth / 2, speedY, speedFontSize, color);
        }
    }
}

void Manager::enableAI(int playerIndex, AIController::Difficulty difficulty)
{
    if (playerIndex >= 0 && playerIndex < players.size()) {
        players[playerIndex]->enableAI(difficulty);
        // Set the correct player index in the AI controller
        if (players[playerIndex]->aiController) {
            players[playerIndex]->aiController = std::make_unique<AIController>(playerIndex, difficulty);
            players[playerIndex]->isAIControlled = true;
        }
        printf("AI enabled for player %d\n", playerIndex + 1);
    }
}

void Manager::disableAI(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < players.size()) {
        players[playerIndex]->disableAI();
        printf("AI disabled for player %d\n", playerIndex + 1);
    }
}

void Manager::setAIDifficulty(int playerIndex, AIController::Difficulty difficulty)
{
    if (playerIndex >= 0 && playerIndex < players.size()) {
        players[playerIndex]->setAIDifficulty(difficulty);
    }
}

std::vector<float> Manager::getUpcomingHitObjects(float currentTime, float lookAheadTime)
{
    std::vector<float> upcomingHits;

    for (float hitTime : hitObjectTimes) {
        float timeDiff = hitTime - currentTime;

        // Include hits that are coming up within the look-ahead window
        if (timeDiff > 0 && timeDiff <= lookAheadTime) {
            upcomingHits.push_back(hitTime);
        }
    }

    return upcomingHits;
}

Color Manager::getBorderColorForTiming()
{
    if (hitObjectTimes.empty()) return WHITE; // No beatmap data

    float currentTime = musicManager.getCurrentSongTime() * 1000.0f;
    float closestDistance = 1000000.0f;

    // Find the closest upcoming hit object
    for (float hitTime : hitObjectTimes) {
        float timeDiff = hitTime - currentTime;
        if (timeDiff > 0 && timeDiff < closestDistance) {
            closestDistance = timeDiff;
        }
    }

    // Change border color based on proximity to hit timing
    if (closestDistance <= 50.0f) {
        // Perfect timing window - Bright Gold
        return (Color){ 255, 215, 0, 255 };
    }
    else if (closestDistance <= 100.0f) {
        // Great timing window - Orange
        return (Color){ 255, 165, 0, 255 };
    }
    else if (closestDistance <= 200.0f) {
        // Good timing window - Yellow
        return (Color){ 255, 255, 0, 255 };
    }
    else if (closestDistance <= 500.0f) {
        // Approaching - Light Blue
        return (Color){ 173, 216, 230, 255 };
    }
    else {
        // Normal - White
        return WHITE;
    }
}

void Manager::drawHitTimingCues()
{
    // Always draw the arena border with timing-based color
    Color borderColor = getBorderColorForTiming();

    // Draw the arena border with dynamic color (3 lines for thickness)
    for (int i = 0; i < 3; i++) {
        DrawCircleLines(screenWidth / 2, screenHeight / 2, levelRadius + i, borderColor);
    }
}

void Manager::showHitFeedback(int playerIndex, float accuracy)
{
    if (playerIndex < 0 || playerIndex >= hitFeedbackText.size()) return;

    float currentTime = musicManager.getCurrentSongTime();
    lastHitFeedbackTime[playerIndex] = currentTime + 1.5f; // Show for 1.5 seconds

    if (accuracy <= 50.0f) {
        // Perfect hit
        consecutiveHits[playerIndex]++;
        hitFeedbackColor[playerIndex] = (Color){ 255, 215, 0, 255 }; // Gold

        // Progressive feedback for consecutive perfect hits
        if (consecutiveHits[playerIndex] >= 5) {
            hitFeedbackText[playerIndex] = "MAGNIFICENT!";
        }
        else if (consecutiveHits[playerIndex] >= 3) {
            hitFeedbackText[playerIndex] = "FANTASTIC!";
        }
        else if (consecutiveHits[playerIndex] >= 2) {
            hitFeedbackText[playerIndex] = "NICE!";
        }
        else {
            hitFeedbackText[playerIndex] = "PERFECT!";
        }
    }
    else if (accuracy <= 100.0f) {
        // Great hit
        consecutiveHits[playerIndex] = 0; // Reset streak
        hitFeedbackText[playerIndex] = "GREAT!";
        hitFeedbackColor[playerIndex] = (Color){ 255, 165, 0, 255 }; // Orange
    }
    else if (accuracy <= 200.0f) {
        // Good hit
        consecutiveHits[playerIndex] = 0; // Reset streak
        hitFeedbackText[playerIndex] = "GOOD!";
        hitFeedbackColor[playerIndex] = (Color){ 255, 255, 0, 255 }; // Yellow
    }
    else if (accuracy <= 2000.0f) {
        // Nearly there
        consecutiveHits[playerIndex] = 0; // Reset streak
        hitFeedbackText[playerIndex] = "Nearly there...";
        hitFeedbackColor[playerIndex] = (Color){ 173, 216, 230, 255 }; // Light blue
    }
    // If accuracy > 2000ms, show nothing (not on beat)
}

void Manager::drawHitFeedback()
{
    float currentTime = musicManager.getCurrentSongTime();

    for (int i = 0; i < hitFeedbackText.size(); i++) {
        // Only draw if feedback is still active
        if (currentTime < lastHitFeedbackTime[i] && !hitFeedbackText[i].empty()) {
            // Position feedback above each player's area
            int x = screenWidth / 2;
            int y = (i == 0) ? screenHeight / 4 : 3 * screenHeight / 4;

            const char* text = hitFeedbackText[i].c_str();
            int fontSize = 32;
            int textWidth = MeasureText(text, fontSize);

            // Draw with fade out effect
            float timeLeft = lastHitFeedbackTime[i] - currentTime;
            float alpha = fminf(timeLeft / 0.5f, 1.0f); // Fade out in last 0.5 seconds

            Color textColor = hitFeedbackColor[i];
            textColor.a = (unsigned char)(255 * alpha);

            DrawText(text, x - textWidth / 2, y, fontSize, textColor);
        }
    }
}