#include "raylib.h"
#include "Manager.h"
#include "Player.h"

#include <iostream>

int main()
{
    int screenWidth = 1280;
    int screenHeight = 720;
    float levelRadius = 300.0;

    InitWindow(screenWidth, screenHeight, "smol-pong");
    // InitAudioDevice();
    // SetMasterVolume(0.2);

    Manager* mgr = new Manager(screenWidth, screenHeight, levelRadius);


    // Load texture, sounds, etc.
    const char* playerSpriteLocation = "./resources/textures/paddle.png";
    Texture2D playerSprite = LoadTexture(playerSpriteLocation);

    Player* p = new Player
    (
        playerSprite,
        (Vector2){3.0f, 11.0f},
        (Vector2){32.0f, 32.0f},
        (Vector2){100.0f, 100.0f},
        (Vector2){50.0f, 16.0f},
        (Vector2){25.0f, 8.0f},
        0.5f,
        0.1f,
        0.0001f,
        9.8f,
        1.0f
    );

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
