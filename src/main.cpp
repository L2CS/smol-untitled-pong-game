#include "Manager.h"
#include "Player.h"
#include "raylib.h"

#include <iostream>

int main()
{
    int screenWidth = 1280;
    int screenHeight = 720;
    float levelRadius = 300.0;
    float offset = 40.0f;
    float boundaryWidth = 300.0f;

    InitWindow(screenWidth, screenHeight, "smol-pong");
    // InitAudioDevice();
    // SetMasterVolume(0.2);

    Manager* mgr = new Manager(
        screenWidth,
        screenHeight,
        levelRadius,
        offset,
        boundaryWidth);

    // Load texture, sounds, etc.
    const char* playerSpriteLocation = "./resources/textures/paddle.png";
    Texture2D playerSprite = LoadTexture(playerSpriteLocation);

    // TODO: Let the user set binds in the game menu :)
    std::vector<int> left{ KEY_LEFT };
    std::vector<int> right{ KEY_RIGHT };
    Keybinds p1Binds = { left, right };

    std::vector<int> left2{ KEY_A };
    std::vector<int> right2{ KEY_D };
    Keybinds p2Binds = { left2, right2 };

    Player* p1 = new Player(
        playerSprite,
        (Vector2){ 3.0f, 11.0f },
        (Vector2){ 32.0f, 32.0f },
        (Vector2){ (float)screenWidth / 2, (float)(screenHeight / 1.5) },
        (Vector2){ 50.0f, 16.0f },
        (Vector2){ 25.0f, 8.0f },
        0.25f,
        0.05f,
        0.0001f,
        9.8f,
        1.0f,
        p1Binds);

    Player* p2 = new Player(
        playerSprite,
        (Vector2){ 3.0f, 11.0f },
        (Vector2){ 32.0f, 32.0f },
        (Vector2){ (float)screenWidth / 2, (float)(screenHeight / 4) },
        (Vector2){ 50.0f, 16.0f },
        (Vector2){ 25.0f, 8.0f },
        0.25f,
        0.05f,
        0.0001f,
        9.8f,
        1.0f,
        p2Binds);

    Ball* b = new Ball(
        (Vector2){ (float)screenWidth / 2, (float)(screenHeight / 2) },
        (Vector2){ 5.0, 5.0 },
        (Vector2){ 5.0, 5.0 },
        0.2f);

    mgr->addEntity(p1);
    mgr->addEntity(p2);
    mgr->addEntity(b);

    mgr->addPlayer(p1);
    mgr->addPlayer(p2);

    // Define a target frame rate and calculate the frame time
    const int targetFPS = 60;
    const float targetFrameTime = 1.0f / targetFPS;

    // Timing variables
    float accumulatedTime = 0.0f;
    double currentTime = GetTime();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Calculate elapsed time since last frame
        double newTime = GetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;

        accumulatedTime += frameTime;

        // Update as many times as necessary to catch up with the target frame rate
        while (accumulatedTime >= targetFrameTime) {
            // Update
            mgr->update();

            accumulatedTime -= targetFrameTime;
        }

        // Draw
        BeginDrawing();
        mgr->draw();

        DrawFPS(10, 10);

        ClearBackground(BLACK);

        EndDrawing();
    }

    // Close window when done
    CloseWindow();

    return 0;
}
