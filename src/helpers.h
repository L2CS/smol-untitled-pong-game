#ifndef _HELPERS
#define _HELPERS
#include "raylib.h"
#include "raymath.h"

#include <cmath>

struct Manager;

extern Vector2* generateCirclePoints(int numPoints, Vector2 center, double radius);
extern Vector2** generateGoalPoints(Manager* _manager, int numPointsEach, float start, float end, double radius);

#endif // _HELPERS
