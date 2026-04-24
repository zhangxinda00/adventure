#include "game/physics.h"

#include <algorithm>
#include <cmath>

namespace game {

namespace {

int floorToInt(float v) {
  return static_cast<int>(std::floor(v));
}

void resolveHorizontal(float& x,
                       float y,
                       float& vx,
                       float w,
                       float h,
                       const std::vector<std::string>& map,
                       float dt) {
  x += vx * dt;

  const int left = floorToInt(x);
  const int right = floorToInt(x + w - 0.001f);
  const int top = floorToInt(y);
  const int bottom = floorToInt(y + h - 0.001f);

  for (int ty = top; ty <= bottom; ++ty) {
    for (int tx = left; tx <= right; ++tx) {
      if (!isSolidTile(map, tx, ty)) {
        continue;
      }

      if (vx > 0.0f) {
        x = static_cast<float>(tx) - w;
      } else if (vx < 0.0f) {
        x = static_cast<float>(tx + 1);
      }
      vx = 0.0f;
      return;
    }
  }
}

void resolveVertical(float x,
                     float& y,
                     float& vy,
                     float w,
                     float h,
                     bool& on_ground,
                     const std::vector<std::string>& map,
                     float dt) {
  y += vy * dt;

  const int left = floorToInt(x);
  const int right = floorToInt(x + w - 0.001f);
  const int top = floorToInt(y);
  const int bottom = floorToInt(y + h - 0.001f);

  on_ground = false;
  for (int ty = top; ty <= bottom; ++ty) {
    for (int tx = left; tx <= right; ++tx) {
      if (!isSolidTile(map, tx, ty)) {
        continue;
      }

      if (vy > 0.0f) {
        y = static_cast<float>(ty) - h;
        on_ground = true;
      } else if (vy < 0.0f) {
        y = static_cast<float>(ty + 1);
      }
      vy = 0.0f;
      return;
    }
  }
}

}  // namespace

bool isSolidTile(const std::vector<std::string>& map, int tx, int ty) {
  if (ty < 0 || ty >= static_cast<int>(map.size())) {
    return true;
  }
  if (map.empty()) {
    return true;
  }
  if (tx < 0 || tx >= static_cast<int>(map[ty].size())) {
    return true;
  }
  return map[ty][tx] == '#';
}

bool intersects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
  return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

void moveWithCollisions(float& x,
                        float& y,
                        float& vx,
                        float& vy,
                        float w,
                        float h,
                        bool& on_ground,
                        const std::vector<std::string>& map,
                        float dt) {
  resolveHorizontal(x, y, vx, w, h, map, dt);
  resolveVertical(x, y, vy, w, h, on_ground, map, dt);
}

}  // namespace game
