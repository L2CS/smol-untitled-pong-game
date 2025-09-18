#pragma once

#include "raylib.h"
#include <string>
#include <unordered_map>
#include <vector>

// Structure to hold a hit object from osu! beatmap
struct HitObject {
    int x, y;           // Position
    float time;         // Time in milliseconds
    int type;           // Object type (1=circle, 2=slider, etc.)
    int hitSound;       // Hit sound
    std::string params; // Additional parameters

    // Helper methods
    bool isCircle() const
    {
        return (type & 1) != 0;
    }
    bool isSlider() const
    {
        return (type & 2) != 0;
    }
    bool isSpinner() const
    {
        return (type & 8) != 0;
    }
};

// Structure to hold timing point data
struct TimingPoint {
    float time;       // Time in milliseconds
    float beatLength; // Beat length in milliseconds (or velocity multiplier if inherited)
    int meter;        // Time signature
    int sampleSet;    // Sample set
    int sampleIndex;  // Sample index
    int volume;       // Volume (0-100)
    bool inherited;   // Whether this is an inherited timing point
    int effects;      // Effects (Kiai time, etc.)
};

// Main osu! beatmap parser class
class OsuParser {
public:
    // Parsed data
    std::unordered_map<std::string, std::string> general;
    std::unordered_map<std::string, std::string> metadata;
    std::unordered_map<std::string, std::string> difficulty;
    std::vector<TimingPoint> timingPoints;
    std::vector<HitObject> hitObjects;

    // Parse methods
    bool parseFile(const std::string& filename);
    void clear();

    // Utility methods
    float getFirstBeatTime() const;
    float getBeatLength() const;
    std::vector<float> getHitObjectTimes() const;
    void printTimingInfo() const;

private:
    void parseSection(const std::string& section, const std::vector<std::string>& lines);
    void parseGeneral(const std::vector<std::string>& lines);
    void parseMetadata(const std::vector<std::string>& lines);
    void parseDifficulty(const std::vector<std::string>& lines);
    void parseTimingPoints(const std::vector<std::string>& lines);
    void parseHitObjects(const std::vector<std::string>& lines);

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
};