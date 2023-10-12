#include "Manager.h"
#include "Player.h"
#include "raylib.h"

#include <iostream>

int main()
{
    int screenWidth = 1280;
    int screenHeight = 720;
    float levelRadius = 300.0;
    float offset = 20.0f;
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

    Player* p = new Player(
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
        1.0f);

    mgr->addEntity(p);

    SetTargetFPS(60);

    // Main game loop here
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        //----------------------------------------------------------------------------------

        // Update
        //----------------------------------------------------------------------------------
        mgr->update();

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        mgr->draw();

        ClearBackground(BLACK);

        EndDrawing();
    }

    // close window when done
    CloseWindow();

    return 0;
}
