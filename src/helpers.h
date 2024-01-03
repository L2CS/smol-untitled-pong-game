#pragma once

#include "raylib.h"
#include "raymath.h"

#include <array>
#include <memory>
#include <vector>

struct Manager;

std::vector<Vector2> generateCirclePoints(int numPoints, Vector2 center, double radius);
std::array<std::vector<Vector2>, 2> generateGoalPoints(Manager* _manager, int numPointsEach, float start, float end, double radius);
