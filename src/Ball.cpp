#include "Ball.h"

#include "Manager.h"
#include "raymath.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

// Particle system implementation
void ParticleSystem::update(float dt)
{
    // Update existing particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= dt;
        if (it->life <= 0) {
            it = particles.erase(it);
        }
        else {
            // Update particle position
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;

            // Fade out over time
            float alpha = it->life / it->maxLife;
            it->color.a = (unsigned char)(255 * alpha);

            // Shrink over time
            it->size *= 0.98f;

            ++it;
        }
    }
}

void ParticleSystem::draw()
{
    for (const auto& particle : particles) {
        DrawCircleV(particle.position, particle.size, particle.color);
    }
}

void ParticleSystem::emit(Vector2 position, Vector2 velocity, float intensity)
{
    if (particles.size() >= MAX_PARTICLES) return;

    // Create new particle
    Particle p;
    p.position = position;

    // Random velocity based on ball velocity and intensity
    float angle = (rand() / (float)RAND_MAX) * 2 * M_PI;
    float speed = intensity * 50.0f + (rand() / (float)RAND_MAX) * 30.0f;

    p.velocity.x = cosf(angle) * speed - velocity.x * 0.3f; // Trail behind ball
    p.velocity.y = sinf(angle) * speed - velocity.y * 0.3f;

    p.life = 0.3f + (rand() / (float)RAND_MAX) * 0.4f; // 0.3-0.7 seconds
    p.maxLife = p.life;

    // Color based on intensity - blue to white to yellow
    if (intensity < 0.5f) {
        p.color = (Color){ 100, 150, 255, 255 }; // Blue
    }
    else if (intensity < 1.0f) {
        p.color = (Color){ 200, 200, 255, 255 }; // Light blue/white
    }
    else {
        p.color = (Color){ 255, 255, 150, 255 }; // Yellow/white
    }

    p.size = 2.0f + intensity * 3.0f; // Size based on intensity

    particles.push_back(p);
}

void ParticleSystem::clear()
{
    particles.clear();
}

/**
 * Collision check for circle and polygon
 */
bool CheckCollisionCirclePolygon(Vector2 circleCenter, float circleRadius, Vector2 polygonPoints[], int numPoints)
{
    // Check if the circle collides with the edges of the polygon
    for (int i = 0; i < numPoints; ++i) {
        Vector2 p1 = polygonPoints[i];
        Vector2 p2 = polygonPoints[(i + 1) % numPoints]; // Wrap around to the first point

        // Calculate the vector between the circle center and the line segment
        Vector2 v = { circleCenter.x - p1.x, circleCenter.y - p1.y };
        Vector2 edge = { p2.x - p1.x, p2.y - p1.y };

        // Project the circle center onto the line segment
        float projection = (v.x * edge.x + v.y * edge.y) / (edge.x * edge.x + edge.y * edge.y);
        projection = fmaxf(0, fminf(1, projection));

        // Calculate the closest point on the line segment to the circle center
        Vector2 closest = {
            p1.x + projection * edge.x,
            p1.y + projection * edge.y
        };

        // Check if the distance between the circle center and the closest point is within the circle's radius
        float distance = sqrtf((circleCenter.x - closest.x) * (circleCenter.x - closest.x) +
                               (circleCenter.y - closest.y) * (circleCenter.y - closest.y));

        if (distance <= circleRadius) {
            return true; // Collision detected
        }
    }

    return false; // No collision detected
}

Ball::Ball(Vector2 _position, Vector2 _outputDims, Vector2 _hitboxDims, float _maxVelocity, float _gravity)
    : Entity(_position, _outputDims, _hitboxDims, EntityType::BALL)
{
    // Start with straight vertical drop (randomly choose up or down)
    float direction = (rand() % 2 == 0) ? -1.0f : 1.0f; // -1 = up, 1 = down
    currentVelocity = (Vector2){ 0.0f, direction * _maxVelocity };

    maxVelocity = _maxVelocity;
    gravity = _gravity;

    printf("Ball starting with %s drop\n", direction < 0 ? "upward" : "downward");
}

/**
 * Hitting goal check
 */
int Ball::hitGoal(Manager* _manager, Vector2 _position)
{
    // TODO: Is there a faster way to do this???
    for (int i = 0; i < _manager->numGoalPoints - 1; i++) {
        std::vector<Vector2> topPoints = _manager->_goalSections[0];
        std::vector<Vector2> bottomPoints = _manager->_goalSections[1];

        Vector2 p1 = topPoints[i];
        Vector2 p2 = topPoints[i + 1];

        Vector2 p3 = bottomPoints[i];
        Vector2 p4 = bottomPoints[i + 1];

        if (CheckCollisionPointLine(_position, p1, p2, 2.0f)) {
            std::cout << "GOOOAALLLLLL!!!!! Player 2 scored!" << std::endl;
            return 1; // Player 2 scored
        }

        if (CheckCollisionPointLine(_position, p3, p4, 2.0f)) {
            std::cout << "GOOOAALLLLLL!!!!! Player 1 scored!" << std::endl;
            return 0; // Player 1 scored
        }
    }

    return -1; // No goal
}

/**
 * Update ball
 */
void Ball::update(Manager* _manager, int _screenWidth, int _screenHeight, float dt)
{
    // Check for goals at current position
    int scoringPlayer = hitGoal(_manager, position);
    if (scoringPlayer != -1) {
        _manager->scoreGoal(scoringPlayer);
        position = (Vector2){ (float)(_manager->screenWidth / 2), (float)(_manager->screenHeight / 2) };
        return; // Reset ball position and exit early
    }

    // Subtle beat-based speed boost
    float beatIntensity = _manager->getCurrentBeatIntensity();
    float speedBoost = 1.0f + beatIntensity * 0.5f; // Small 15% max boost on beats

    // Apply gentle speed boost
    currentVelocity = Vector2Scale(currentVelocity, speedBoost);

    // Speed clamping
    float currentSpeed = Vector2Length(currentVelocity);
    if (currentSpeed > maxVelocity * 2.0f) {
        currentVelocity = Vector2Scale(Vector2Normalize(currentVelocity), maxVelocity * 2.0f);
    }

    // Emit particles based on speed and beat intensity
    float speedRatio = currentSpeed / maxVelocity;
    float particleIntensity = (speedRatio - 1.0f) + beatIntensity;

    if (particleIntensity > 0.2f) {
        particles.emit(position, currentVelocity, particleIntensity);
    }

    Vector2 center = { (float)_screenWidth / 2, (float)_screenHeight / 2 };

    float topGoalY = _screenHeight / 2 - _manager->levelRadius + 15;
    float bottomGoalY = _screenHeight / 2 + _manager->levelRadius - 15;

    Vector2 pullForce = { 0, 0 };

    float distanceToTopGoal = fabsf(position.y - topGoalY);
    float distanceToBottomGoal = fabsf(position.y - bottomGoalY);

    if (distanceToTopGoal < distanceToBottomGoal) {
        pullForce.y = -gravity * 0.02f * dt;
    }
    else {
        pullForce.y = gravity * 0.02f * dt;
    }

    float horizontalDistance = fabsf(position.x - center.x);
    float maxDistance = _manager->levelRadius * 0.6f;

    if (horizontalDistance > maxDistance) {
        float pullStrength = (horizontalDistance - maxDistance) / (_manager->levelRadius * 0.4f);
        pullStrength = fminf(pullStrength, 1.0f); // Cap at 1.0
        pullStrength *= gravity * 0.08f * dt;

        if (position.x < center.x) {
            pullForce.x = pullStrength;
        }
        else {
            pullForce.x = -pullStrength;
        }
    }

    currentVelocity = Vector2Add(currentVelocity, pullForce);

    // Calculate new position
    Vector2 positionDelta = { currentVelocity.x * dt, currentVelocity.y * dt };
    Vector2 newPosition = Vector2Add(position, positionDelta);

    // Check for circular boundary collision and bounce
    Vector2 circleCenter = { float(_manager->screenWidth / 2), float(_manager->screenHeight / 2) };
    float circleRadius = _manager->levelRadius;

    Vector2 toCenter = Vector2Subtract(newPosition, circleCenter);
    float distanceToCenter = Vector2Length(toCenter);

    if (distanceToCenter > circleRadius) {
        // Ball hit the circular boundary simple reflection
        Vector2 normal = Vector2Normalize(toCenter);
        Vector2 contactPoint = Vector2Add(circleCenter, Vector2Scale(normal, circleRadius));

        // Move ball to contact point
        newPosition = contactPoint;

        currentVelocity = Vector2Reflect(currentVelocity, normal);

        // Add randomness to break perfect bouncing patterns
        float randomAngle = (rand() / (float)RAND_MAX - 0.5f) * 0.3f;
        float currentAngle = atan2f(currentVelocity.y, currentVelocity.x);
        float newAngle = currentAngle + randomAngle;
        float speed = Vector2Length(currentVelocity);

        currentVelocity = (Vector2){
            cosf(newAngle) * speed * 0.95f,
            sinf(newAngle) * speed * 0.95f
        };

        // Reset paddle hit tracking since ball hit boundary
        _manager->onNonPaddleHit();
    }

    position = newPosition;

    // Update particle system
    particles.update(dt / 1000.0f); // Convert ms to seconds

    // Call handleCollisions for the ball to handle collisions
    handleCollisions(_manager);

    // TODO: ADD GLOBAL HITBOX VAR
    // if (IsKeyDown(KEY_H)) showHitboxes = !showHitboxes;
}

void Ball::handleCollisions(Manager* _manager)
{
    const int numPoints = 8; // Number of points in the collider

    // Offsets for the collider points
    Vector2 offsets[numPoints] = {
        { 15.0f, -5.0f },
        { 2.0f, -3.0f },
        { -10.0f, -3.0f },
        { -23.0f, -5.0f },
        { -23.0f, -8.0f },
        { -10.0f, -6.0f },
        { 2.0f, -6.0f },
        { 15.0f, -8.0f }
    };

    // Iterate through players and check collisions
    for (int playerIndex = 0; playerIndex < _manager->players.size(); playerIndex++) {
        auto player = _manager->players[playerIndex];
        Vector2 polyPoints[8];
        float rotationAngle = player->rotation * (float)M_PI / 180.0f;

        for (int i = 0; i < numPoints; ++i) {
            float x = offsets[i].x * cosf(rotationAngle) -
                      offsets[i].y * sinf(rotationAngle);
            float y = offsets[i].x * sinf(rotationAngle) +
                      offsets[i].y * cosf(rotationAngle);

            polyPoints[i] = { player->position.x + x, player->position.y + y };
        }

        // Check collisions using CheckCollisionCirclePoly
        if (CheckCollisionCirclePolygon(position, 5.0f, polyPoints, 8)) {
            // Calculate the collision point relative to the center of the circle
            float relativeX = position.x - player->position.x;
            float relativeY = position.y - player->position.y;

            // Calculate the angle of collision
            float collisionAngle = atan2f(relativeY, relativeX);

            // Calculate the new velocity based on the angle
            float speed = Vector2Length(currentVelocity);
            float bounceMultiplier = 1.3f; // Increase speed when hit by paddle
            float newSpeed = speed * bounceMultiplier;

            // Cap the speed
            float maxBounceSpeed = maxVelocity * 2.0f;
            newSpeed = fminf(newSpeed, maxBounceSpeed);

            currentVelocity.x = cosf(collisionAngle) * newSpeed;
            currentVelocity.y = sinf(collisionAngle) * newSpeed;

            // Trigger multiplier system for this player
            _manager->onPlayerHitBall(playerIndex);

            // Play paddle hit sound
            _manager->onPaddleHit();
        }
    }
}

void Ball::draw()
{
    // Draw particles first (behind ball)
    particles.draw();

    DrawCircle(position.x, position.y, outputDims.x, WHITE);
    // DrawRectangleLines(position.x - origin.x, position.y - origin.y, hitboxDims.x, hitboxDims.y, RED);
}
