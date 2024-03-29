#ifndef CONFIG_H
#define CONFIG_H

/*
 * Global variables go here.
 */

const float kCylinderRadius = 0.25;
const int kMaxBones = 128;
/*
 * Extra credit: what would happen if you set kNear to 1e-5? How to solve it?
 */
const float kNear = 0.1f;
const float kFar = 10.0f;
const float kFov = 45.0f;

// Floor info.
const float kFloorEps = 0.5 * (0.025 + 0.0175);
const float kFloorXMin = -4.0f;
const float kFloorXMax = 4.0f;
const float kFloorZMin = -4.0f;
const float kFloorZMax = 4.0f;
const float kFloorY = -2.0f;

const float kScrollSpeed = 64.0f;
constexpr unsigned NUM_LIGHTS = 2;

#endif
