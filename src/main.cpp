#include "Manager.h"
#include "Player.h"
#include "raylib.h"

#include <memory>
#include <cstring>

int main(int argc, char* argv[])
{
    // Parse command line arguments
    const char* musicFile = "./backgorund.wav"; // Default music file
    const char* backgroundImage = "./bg.png"; // Default background (supports .png, .jpg, .bmp, etc.)
    const char* osuFile = nullptr; // Optional osu! beatmap file
    bool enableAI = false;
    int aiDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard
    int aiPlayer = 1; // Which player to make AI (0 or 1)
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ai") == 0 || strcmp(argv[i], "-ai") == 0) {
            enableAI = true;
            // Check for difficulty parameter
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                i++;
                if (strcmp(argv[i], "easy") == 0) aiDifficulty = 0;
                else if (strcmp(argv[i], "medium") == 0) aiDifficulty = 1;
                else if (strcmp(argv[i], "hard") == 0) aiDifficulty = 2;
                else {
                    printf("Invalid AI difficulty: %s (using medium)\n", argv[i]);
                    aiDifficulty = 1;
                }
            }
        } else if (strcmp(argv[i], "--ai-player") == 0) {
            if (i + 1 < argc) {
                i++;
                aiPlayer = atoi(argv[i]);
                if (aiPlayer < 0 || aiPlayer > 1) {
                    printf("Invalid AI player index: %d (using 1)\n", aiPlayer);
                    aiPlayer = 1;
                }
            }
        } else if (i == 1) {
            musicFile = argv[i];
        } else if (i == 2) {
            backgroundImage = argv[i];
        } else if (i == 3) {
            osuFile = argv[i];
        }
    }
    
    if (argc == 1) {
        printf("Usage: %s [music_file] [background_image] [osu_beatmap] [options]\n", argv[0]);
        printf("Options:\n");
        printf("  --ai [easy|medium|hard]  Enable AI opponent (default: medium)\n");
        printf("  --ai-player [0|1]        Which player to make AI (default: 1)\n");
        printf("\nSupported formats:\n");
        printf("  Music: .wav files (for FFT analysis)\n");
        printf("  Background: .png, .jpg, .jpeg, .bmp, .tga files\n");
        printf("  Beatmap: .osu files (for score multiplier system)\n");
        printf("\nExamples:\n");
        printf("  %s song.wav bg.png map.osu --ai hard\n", argv[0]);
        printf("  %s song.wav bg.png --ai easy --ai-player 0\n", argv[0]);
    }
    
    printf("Configuration:\n");
    printf("  Music: %s\n", musicFile);
    printf("  Background: %s\n", backgroundImage);
    printf("  Beatmap: %s\n", osuFile ? osuFile : "None (default scoring)");
    if (enableAI) {
        const char* diffNames[] = {"Easy", "Medium", "Hard"};
        printf("  AI: Player %d - %s\n", aiPlayer + 1, diffNames[aiDifficulty]);
    } else {
        printf("  AI: Disabled (human vs human)\n");
    }
    
    // Start with a reasonable window size, then go fullscreen
    int screenWidth = 1920;
    int screenHeight = 1080;
    
    InitWindow(screenWidth, screenHeight, "smol-pong");
    ToggleFullscreen(); // Make it fullscreen
    
    // Get actual screen dimensions after fullscreen
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    float levelRadius = fminf(screenWidth, screenHeight) * 0.25f; // Scale radius to screen
    float offset = 40.0f;
    float boundaryWidth = 150.0f; // Start with half the radius, can expand up to full radius
    
    printf("Screen dimensions: %dx%d, Level radius: %.1f\n", screenWidth, screenHeight, levelRadius);
    
    // Raylib audio has issues on this macOS system, use system audio instead
    printf("Using system audio player (afplay) for macOS compatibility...\n");
    
    // Load texture, sounds, etc.
    const char* playerSpriteLocation = "./resources/textures/paddle.png";
    Texture2D playerSprite = LoadTexture(playerSpriteLocation);
    
    // Load background image with better error handling
    printf("Attempting to load background image: %s\n", backgroundImage);
    
    // First try to load as an Image to get more detailed error info
    Image backgroundImg = LoadImage(backgroundImage);
    Texture2D backgroundTexture = {0};
    
    if (backgroundImg.data != NULL) {
        printf("Image loaded successfully - Format: %d, Width: %d, Height: %d\n", 
               backgroundImg.format, backgroundImg.width, backgroundImg.height);
        
        // Convert to a supported format if needed
        if (backgroundImg.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
            printf("Converting image format...\n");
            ImageFormat(&backgroundImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        }
        
        // Create texture from image
        backgroundTexture = LoadTextureFromImage(backgroundImg);
        UnloadImage(backgroundImg);
        
        if (backgroundTexture.id != 0) {
            printf("Background texture created successfully: %s\n", backgroundImage);
        } else {
            printf("Failed to create texture from image\n");
        }
    } else {
        printf("Warning: Could not load background image: %s\n", backgroundImage);
        printf("This may be due to unsupported JPG format or corrupted file\n");
        printf("Try converting to PNG format for better compatibility\n");
        printf("Using default background (black)\n");
    }

    // TODO: Make into unique_ptr, stop passing around manager everywhere
    std::shared_ptr<Manager> mgr = std::make_shared<Manager>(
        screenWidth,
        screenHeight,
        levelRadius,
        offset,
        boundaryWidth,
        backgroundTexture,
        musicFile,
        osuFile);

    // Start background music using macOS system command with reduced volume
    char musicCommand[512];
    snprintf(musicCommand, sizeof(musicCommand), "afplay -v 0.3 \"%s\" &", musicFile);
    system(musicCommand);
    printf("Background music started: %s at 30% volume.\n", musicFile);
    
    // Set the song start time for visualizer sync
    mgr->setSongStartTime();
    
    // System audio is now playing in background
    bool musicLoaded = true; // We're using system audio
    
    // TODO: Let the user set binds in the game menu :)
    // Set up keybinds based on AI configuration
    Keybinds p1Binds, p2Binds;
    
    if (enableAI) {
        if (aiPlayer == 0) {
            // AI is Player 1, human is Player 2 - give human both key sets
            std::vector<int> humanLeft{ KEY_LEFT, KEY_A };
            std::vector<int> humanRight{ KEY_RIGHT, KEY_D };
            p1Binds = { {}, {} }; // AI doesn't need keys
            p2Binds = { humanLeft, humanRight };
            printf("Human player (P2) controls: Arrow Keys + A/D Keys\n");
        } else {
            // AI is Player 2, human is Player 1 - give human both key sets  
            std::vector<int> humanLeft{ KEY_LEFT, KEY_A };
            std::vector<int> humanRight{ KEY_RIGHT, KEY_D };
            p1Binds = { humanLeft, humanRight };
            p2Binds = { {}, {} }; // AI doesn't need keys
            printf("Human player (P1) controls: Arrow Keys + A/D Keys\n");
        }
    } else {
        // Human vs Human - separate key sets
        std::vector<int> left1{ KEY_LEFT };
        std::vector<int> right1{ KEY_RIGHT };
        p1Binds = { left1, right1 };

        std::vector<int> left2{ KEY_A };
        std::vector<int> right2{ KEY_D };
        p2Binds = { left2, right2 };
        printf("Player 1 controls: Arrow Keys, Player 2 controls: A/D Keys\n");
    }

    std::shared_ptr<Player> p1 = std::make_shared<Player>(
        playerSprite,
        (Vector2){ 3.0f, 11.0f },
        (Vector2){ 32.0f, 32.0f },
        (Vector2){ (float)screenWidth / 2, (float)(screenHeight / 1.5) },
        (Vector2){ 65.0f, 20.0f }, // Increased from 50x16 to 65x20
        (Vector2){ 32.0f, 10.0f }, // Increased hitbox proportionally
        0.5f,
        0.05f,
        0.0001f,
        9.8f,
        1.0f,
        p1Binds);

    std::shared_ptr<Player> p2 = std::make_shared<Player>(
        playerSprite,
        (Vector2){ 3.0f, 11.0f },
        (Vector2){ 32.0f, 32.0f },
        (Vector2){ (float)screenWidth / 2, (float)(screenHeight / 4) },
        (Vector2){ 65.0f, 20.0f }, // Increased from 50x16 to 65x20
        (Vector2){ 32.0f, 10.0f }, // Increased hitbox proportionally
        0.5f,
        0.05f,
        0.0001f,
        9.8f,
        1.0f,
        p2Binds);

    std::shared_ptr<Ball> b = std::make_shared<Ball>(
        (Vector2){ (float)screenWidth / 2, (float)(screenHeight / 2) },
        (Vector2){ 5.0, 5.0 },
        (Vector2){ 5.0, 5.0 },
        0.2f,
        0.005f); // Much lighter gravity - subtle pull toward goal areas

    mgr->addEntity(p1);
    mgr->addEntity(p2);
    mgr->addEntity(b);

    mgr->addPlayer(p1);
    mgr->addPlayer(p2);
    
    // Enable AI if requested
    if (enableAI) {
        AIController::Difficulty difficulty = static_cast<AIController::Difficulty>(aiDifficulty);
        mgr->enableAI(aiPlayer, difficulty);
    }

    // Define a target frame rate and calculate the frame time
    const int targetFPS = 60;
    const float targetFrameTime = 1.0f / targetFPS;

    // Timing variables
    float accumulatedTime = 0.0f;
    double currentTime = GetTime();

    // Main game loop - run until window closes or game ends with ESC
    while (!WindowShouldClose())
    {
        // Check if game ended and ESC is pressed
        if (mgr->isGameEnded() && IsKeyPressed(KEY_ESCAPE)) {
            break; // Exit game loop
        }
        
        // No need to update music stream - system handles it
        
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
        
        ClearBackground(BLACK);
        
        mgr->draw();

        DrawFPS(10, 10);

        EndDrawing();
    }

    // Cleanup - stop system audio
    system("killall afplay"); // Stop any running afplay processes
    CloseWindow();

    return 0;
}
