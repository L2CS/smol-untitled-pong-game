#ifndef _HELPERS
#define _HELPERS
#include "raylib.h"

#include <cmath>

Vector2* generateCirclePoints(int numPoints, Vector2 center, double radius) {
    Vector2* points = new Vector2[numPoints];

    for (int i = 0; i < numPoints; ++i) {
        double angle = 2 * M_PI * i / numPoints;
        points[i].x = (float)(center.x + radius * cos(angle));
        points[i].y = (float)(center.y + radius * sin(angle));
    }

    return points;
}


#endif // _HELPERS

