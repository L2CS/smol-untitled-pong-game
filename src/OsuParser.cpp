#include "OsuParser.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

bool OsuParser::parseFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        printf("Failed to open osu! beatmap file: %s\n", filename.c_str());
        return false;
    }

    clear();

    std::string line;
    std::string currentSection;
    std::vector<std::string> sectionLines;

    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '/' && line[1] == '/') {
            continue;
        }

        // Check for section headers
        if (line[0] == '[' && line.back() == ']') {
            // Process previous section
            if (!currentSection.empty()) {
                parseSection(currentSection, sectionLines);
            }

            // Start new section
            currentSection = line.substr(1, line.length() - 2);
            sectionLines.clear();
        }
        else {
            // Add line to current section
            sectionLines.push_back(line);
        }
    }

    // Process final section
    if (!currentSection.empty()) {
        parseSection(currentSection, sectionLines);
    }

    file.close();

    printf("Parsed osu! beatmap: %zu hit objects, %zu timing points\n",
           hitObjects.size(), timingPoints.size());

    return true;
}

void OsuParser::clear()
{
    general.clear();
    metadata.clear();
    difficulty.clear();
    timingPoints.clear();
    hitObjects.clear();
}

void OsuParser::parseSection(const std::string& section, const std::vector<std::string>& lines)
{
    if (section == "General") {
        parseGeneral(lines);
    }
    else if (section == "Metadata") {
        parseMetadata(lines);
    }
    else if (section == "Difficulty") {
        parseDifficulty(lines);
    }
    else if (section == "TimingPoints") {
        parseTimingPoints(lines);
    }
    else if (section == "HitObjects") {
        parseHitObjects(lines);
    }
    // Skip other sections for now
}

void OsuParser::parseGeneral(const std::vector<std::string>& lines)
{
    for (const auto& line : lines) {
        auto parts = split(line, ':');
        if (parts.size() >= 2) {
            std::string key = trim(parts[0]);
            std::string value = trim(parts[1]);
            general[key] = value;
        }
    }
}

void OsuParser::parseMetadata(const std::vector<std::string>& lines)
{
    for (const auto& line : lines) {
        auto parts = split(line, ':');
        if (parts.size() >= 2) {
            std::string key = trim(parts[0]);
            std::string value = trim(parts[1]);
            metadata[key] = value;
        }
    }
}

void OsuParser::parseDifficulty(const std::vector<std::string>& lines)
{
    for (const auto& line : lines) {
        auto parts = split(line, ':');
        if (parts.size() >= 2) {
            std::string key = trim(parts[0]);
            std::string value = trim(parts[1]);
            difficulty[key] = value;
        }
    }
}

void OsuParser::parseTimingPoints(const std::vector<std::string>& lines)
{
    for (const auto& line : lines) {
        auto parts = split(line, ',');
        if (parts.size() >= 8) {
            TimingPoint tp;
            tp.time = std::stof(parts[0]);
            tp.beatLength = std::stof(parts[1]);
            tp.meter = std::stoi(parts[2]);
            tp.sampleSet = std::stoi(parts[3]);
            tp.sampleIndex = std::stoi(parts[4]);
            tp.volume = std::stoi(parts[5]);
            tp.inherited = (std::stoi(parts[6]) == 0);
            tp.effects = std::stoi(parts[7]);

            timingPoints.push_back(tp);
        }
    }
}

void OsuParser::parseHitObjects(const std::vector<std::string>& lines)
{
    for (const auto& line : lines) {
        auto parts = split(line, ',');
        if (parts.size() >= 5) {
            HitObject obj;
            obj.x = std::stoi(parts[0]);
            obj.y = std::stoi(parts[1]);
            obj.time = std::stof(parts[2]);
            obj.type = std::stoi(parts[3]);
            obj.hitSound = std::stoi(parts[4]);

            // Store remaining parameters as string
            if (parts.size() > 5) {
                obj.params = "";
                for (size_t i = 5; i < parts.size(); i++) {
                    if (i > 5) obj.params += ",";
                    obj.params += parts[i];
                }
            }

            hitObjects.push_back(obj);
        }
    }
}

float OsuParser::getFirstBeatTime() const
{
    if (!timingPoints.empty()) {
        return timingPoints[0].time;
    }
    return 0.0f;
}

float OsuParser::getBeatLength() const
{
    if (!timingPoints.empty()) {
        return timingPoints[0].beatLength;
    }
    return 500.0f; // Default 120 BPM
}

std::vector<float> OsuParser::getHitObjectTimes() const
{
    std::vector<float> times;
    for (const auto& obj : hitObjects) {
        times.push_back(obj.time);
    }
    return times;
}

std::vector<std::string> OsuParser::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string OsuParser::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

void OsuParser::printTimingInfo() const
{
    if (metadata.find("Title") != metadata.end()) {
        printf("Title: %s\n", metadata.at("Title").c_str());
    }
    if (metadata.find("Artist") != metadata.end()) {
        printf("Artist: %s\n", metadata.at("Artist").c_str());
    }

    printf("Timing Points: %zu\n", timingPoints.size());

    for (size_t i = 0; i < timingPoints.size() && i < 5; i++) {
        const auto& tp = timingPoints[i];
        printf("  [%zu] Time: %.1fms, Beat Length: %.3fms, Inherited: %s\n",
               i, tp.time, tp.beatLength, tp.inherited ? "Yes" : "No");

        if (!tp.inherited && tp.beatLength > 0) {
            float bpm = 60000.0f / tp.beatLength;
            printf("       BPM: %.1f\n", bpm);
        }
    }

    if (timingPoints.size() > 5) {
        printf("  ... and %zu more timing points\n", timingPoints.size() - 5);
    }

    printf("Hit Objects: %zu\n", hitObjects.size());
}