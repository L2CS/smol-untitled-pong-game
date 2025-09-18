#include "Visualizer.h"
#include "MusicManager.h"
#include <cmath>
#include <cstdlib>

Visualizer::Visualizer(int _screenWidth, int _screenHeight, float _levelRadius)
    : screenWidth(_screenWidth), screenHeight(_screenHeight), levelRadius(_levelRadius)
{
    // Initialize visualizer arrays
    visualizerBars.resize(VISUALIZER_BARS, 0.0f);
    visualizerTargets.resize(VISUALIZER_BARS, 0.0f);
    visualizerColors.resize(VISUALIZER_BARS);

    // Create rainbow colors around the circle
    for (int i = 0; i < VISUALIZER_BARS; i++) {
        float hue = (float)i / VISUALIZER_BARS * 360.0f;
        visualizerColors[i] = ColorFromHSV(hue, 0.8f, 1.0f);
    }
}

void Visualizer::update(const MusicManager& musicManager)
{
    const auto& magnitudes = musicManager.getMagnitudes();

    // Map FFT magnitudes to visualizer bars
    for (int i = 0; i < VISUALIZER_BARS; i++) {
        // Map frequency bins to bars
        float freqStart = powf(2.0f, (float)i / VISUALIZER_BARS * 10.0f);
        float freqEnd = powf(2.0f, (float)(i + 1) / VISUALIZER_BARS * 10.0f);

        // Convert frequency to FFT bin indices
        int binStart = (int)(freqStart * MusicManager::FFT_SIZE / MusicManager::SAMPLE_RATE);
        int binEnd = (int)(freqEnd * MusicManager::FFT_SIZE / MusicManager::SAMPLE_RATE);

        // Clamp to valid range
        binStart = fmaxf(1, fminf(binStart, MusicManager::FFT_SIZE / 2 - 1));
        binEnd = fmaxf(binStart + 1, fminf(binEnd, MusicManager::FFT_SIZE / 2));

        // Average the magnitudes in this frequency range
        float avgMagnitude = 0.0f;
        if (!magnitudes.empty()) {
            for (int bin = binStart; bin < binEnd; bin++) {
                avgMagnitude += magnitudes[bin];
            }
            avgMagnitude /= (binEnd - binStart);
        }

        // Scale and apply to visualizer
        float targetHeight = avgMagnitude * 200.0f; // Doubled scale factor

        // Apply logarithmic scaling for better visual range
        targetHeight = logf(1.0f + targetHeight) * 25.0f;

        visualizerTargets[i] = targetHeight;

        // Smooth interpolation towards target
        float diff = visualizerTargets[i] - visualizerBars[i];
        visualizerBars[i] += diff * 0.3f;

        // Decay over time
        visualizerBars[i] *= 0.85f;

        if (visualizerBars[i] < 5.0f) visualizerBars[i] = 5.0f;
    }
}

void Visualizer::draw()
{
    Vector2 center = { (float)(screenWidth / 2), (float)(screenHeight / 2) };
    float outerRadius = levelRadius + 20.0f;

    for (int i = 0; i < VISUALIZER_BARS; i++) {
        // Calculate angle for this bar
        float angle = (float)i / VISUALIZER_BARS * 2.0f * M_PI;

        // Calculate bar height
        float barHeight = visualizerBars[i];

        // Calculate positions
        float innerRadius = outerRadius;
        float barOuterRadius = innerRadius + barHeight;

        Vector2 innerPos = {
            center.x + innerRadius * cosf(angle),
            center.y + innerRadius * sinf(angle)
        };

        Vector2 outerPos = {
            center.x + barOuterRadius * cosf(angle),
            center.y + barOuterRadius * sinf(angle)
        };

        // Calculate bar width
        float barWidth = 6.0f;

        // Draw the bar as a thick line with color
        Color barColor = visualizerColors[i];
        barColor.a = (unsigned char)(255 * fminf(barHeight / 80.0f, 1.0f));

        DrawLineEx(innerPos, outerPos, barWidth, barColor);

        Color glowColor = barColor;
        glowColor.a = (unsigned char)(glowColor.a * 0.3f);
        DrawLineEx(innerPos, outerPos, barWidth * 2.0f, glowColor);
    }
}

void Visualizer::triggerBeat(float intensity)
{
    // Trigger a beat effect
    for (int i = 0; i < VISUALIZER_BARS; i++) {
        if (rand() % 100 < 30) { // 30% chance for each bar
            visualizerBars[i] += intensity * 20.0f;
            if (visualizerBars[i] > 80.0f) visualizerBars[i] = 80.0f;
        }
    }
}

void Visualizer::updateRadius(float newRadius)
{
    levelRadius = newRadius;
}