#include "helpers.h"

#include "Manager.h"

#include <cmath>

std::vector<Vector2> generateCirclePoints(int numPoints, Vector2 center, double radius)
{
    std::vector<Vector2> points = {};

    for (int i = 0; i < numPoints; ++i) {
        double angle = 2 * M_PI * i / numPoints;
        Vector2 p;
        p.x = (float)(center.x + radius * cos(angle));
        p.y = (float)(center.y + radius * sin(angle));
        points.emplace_back(p);
    }

    return points;
}

std::array<std::vector<Vector2>, 2> generateGoalPoints(Manager* _manager, int numPointsEach, float start, float end, double radius)
{
    std::array<std::vector<Vector2>, 2> sections;

    std::vector<Vector2> topPoints(numPointsEach);
    std::vector<Vector2> bottomPoints(numPointsEach);

    for (int i = 0; i < numPointsEach; i++) {
        topPoints[i].x = (((float)i * (end - start)) / (float)numPointsEach) + start;
        bottomPoints[i].x = (((float)i * (end - start)) / (float)numPointsEach) + start;

        topPoints[i].y = -std::sqrt(std::pow((double)radius, 2) - std::pow((double)topPoints[i].x - (double)(_manager->screenWidth) / 2, 2)) + _manager->screenHeight / 2;
        bottomPoints[i].y = std::sqrt(std::pow((double)radius, 2) - std::pow((double)bottomPoints[i].x - (double)(_manager->screenWidth) / 2, 2)) + _manager->screenHeight / 2;
    }

    sections[0] = topPoints;
    sections[1] = bottomPoints;

    return sections;
}
