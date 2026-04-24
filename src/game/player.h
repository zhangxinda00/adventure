#pragma once

namespace game {

struct Player {
  float x = 2.0f;
  float y = 2.0f;
  float vx = 0.0f;
  float vy = 0.0f;
  float width = 0.9f;
  float height = 0.9f;

  int hp = 5;
  bool on_ground = false;
  bool facing_right = true;

  float attack_cooldown = 0.0f;
  float attack_timer = 0.0f;
  float invuln_timer = 0.0f;

  // Fire-breath skill: available from the start, key F
  float fire_cooldown = 0.0f;
  float fire_timer = 0.0f;
};

}  // namespace game
