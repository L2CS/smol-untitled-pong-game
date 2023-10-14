#include "helpers.h"
#include "Manager.h"

Vector2* generateCirclePoints(int numPoints, Vector2 center, double radius) {
    Vector2* points = new Vector2[numPoints];

    for (int i = 0; i < numPoints; ++i) {
        double angle = 2 * M_PI * i / numPoints;
        points[i].x = (float)(center.x + radius * cos(angle));
        points[i].y = (float)(center.y + radius * sin(angle));
    }

    return points;
}


Vector2** generateGoalPoints(Manager* _manager, int numPointsEach, float start, float end, double radius) {
    Vector2** sections = new Vector2*[2];


    Vector2* topPoints = new Vector2[numPointsEach];
    Vector2* bottomPoints = new Vector2[numPointsEach];

    for (int i = 0; i < numPointsEach; i++) {
        topPoints[i].x = (( (float)i*(end - start) ) / (float)numPointsEach) + start;
        bottomPoints[i].x = (( (float)i*(end - start) ) / (float)numPointsEach) + start;

        topPoints[i].y = -std::sqrt( std::pow((double)radius, 2) - std::pow((double)topPoints[i].x - (double)(_manager->screenWidth)/2, 2))
            + _manager->screenHeight/2;
        bottomPoints[i].y = std::sqrt( std::pow((double)radius, 2) - std::pow((double)bottomPoints[i].x - (double)(_manager->screenWidth)/2, 2))
            + _manager->screenHeight/2;
    }

    sections[0] = topPoints;
    sections[1] = bottomPoints;

    return sections;
}
