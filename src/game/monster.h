#pragma once

namespace game {

struct Monster {
  float x = 0.0f;
  float y = 0.0f;
  float vx = 0.0f;
  float vy = 0.0f;
  float width = 0.9f;
  float height = 0.9f;

  int hp = 2;
  bool alive = true;
  bool facing_right = false;
  float touch_cooldown = 0.0f;
  int patrol_dir = -1;
};

}  // namespace game
