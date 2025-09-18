#pragma once

#include "raylib.h"
#include <vector>

class MusicManager; // Forward declaration

class Visualizer {
public:
    static const int VISUALIZER_BARS = 64;

    // Constructor
    Visualizer(int screenWidth, int screenHeight, float levelRadius);

    // Update and draw
    void update(const MusicManager& musicManager);
    void draw();
    void triggerBeat(float intensity);
    void updateRadius(float newRadius);

private:
    // Screen dimensions
    int screenWidth;
    int screenHeight;
    float levelRadius;

    // Visualizer data
    std::vector<float> visualizerBars;
    std::vector<float> visualizerTargets;
    std::vector<Color> visualizerColors;
};