#include "game/world.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "game/level_data.h"
#include "game/physics.h"

namespace game {

namespace {

constexpr float kGravity = 26.0f;
constexpr float kMaxFallSpeed = 20.0f;
constexpr float kMoveSpeed = 7.0f;
constexpr float kMonsterPatrolSpeed = 2.0f;
constexpr float kMonsterChaseSpeed = 3.5f;
constexpr float kJumpVelocity = -12.0f;

float sign(float v) {
  if (v > 0.0f) {
    return 1.0f;
  }
  if (v < 0.0f) {
    return -1.0f;
  }
  return 0.0f;
}

}  // namespace

World::World()
    : state_(GameState::Running),
      kills_(0),
      attack_x_(0.0f),
      attack_y_(0.0f),
      attack_w_(0.0f),
      attack_h_(0.0f),
      attack_active_this_frame_(false),
      fire_x_(0.0f),
      fire_y_(0.0f),
      fire_w_(0.0f),
      fire_h_(0.0f),
      fire_active_this_frame_(false) {
  reset();
}

void World::reset() {
  map_ = createDefaultLevel();
  monsters_.clear();

  player_ = Player();
  kills_ = 0;
  state_ = GameState::Running;
  input_ = engine::InputState();
  attack_active_this_frame_ = false;
  fire_active_this_frame_ = false;

  for (int y = 0; y < static_cast<int>(map_.size()); ++y) {
    for (int x = 0; x < static_cast<int>(map_[y].size()); ++x) {
      if (map_[y][x] == 'P') {
        player_.x = static_cast<float>(x);
        player_.y = static_cast<float>(y);
        map_[y][x] = '.';
      } else if (map_[y][x] == 'M') {
        Monster m;
        m.x = static_cast<float>(x);
        m.y = static_cast<float>(y);
        monsters_.push_back(m);
        map_[y][x] = '.';
      }
    }
  }
}

void World::handleInput(const engine::InputState& input) {
  input_ = input;
}

void World::update(float dt) {
  attack_active_this_frame_ = false;
  fire_active_this_frame_ = false;

  if (state_ != GameState::Running) {
    return;
  }

  updatePlayer(dt);
  updateMonsters(dt);
  resolveCombat();

  int alive_count = 0;
  for (std::size_t i = 0; i < monsters_.size(); ++i) {
    if (monsters_[i].alive) {
      ++alive_count;
    }
  }

  if (player_.hp <= 0) {
    state_ = GameState::Lost;
  } else if (alive_count == 0 || atGoal()) {
    state_ = GameState::Won;
  }
}

void World::updatePlayer(float dt) {
  if (player_.attack_cooldown > 0.0f) {
    player_.attack_cooldown = std::max(0.0f, player_.attack_cooldown - dt);
  }
  if (player_.attack_timer > 0.0f) {
    player_.attack_timer = std::max(0.0f, player_.attack_timer - dt);
  }
  if (player_.invuln_timer > 0.0f) {
    player_.invuln_timer = std::max(0.0f, player_.invuln_timer - dt);
  }

  float target_vx = 0.0f;
  if (input_.left && !input_.right) {
    target_vx = -kMoveSpeed;
    player_.facing_right = false;
  } else if (input_.right && !input_.left) {
    target_vx = kMoveSpeed;
    player_.facing_right = true;
  }

  if (target_vx == 0.0f) {
    player_.vx *= 0.7f;
    if (std::fabs(player_.vx) < 0.05f) {
      player_.vx = 0.0f;
    }
  } else {
    player_.vx = target_vx;
  }

  if (input_.jump && player_.on_ground) {
    player_.vy = kJumpVelocity;
    player_.on_ground = false;
  }

  if (input_.attack && player_.attack_cooldown <= 0.0f) {
    player_.attack_cooldown = 0.35f;
    player_.attack_timer = 0.10f;
    attack_active_this_frame_ = true;

    attack_w_ = 1.4f;
    attack_h_ = 0.9f;
    attack_y_ = player_.y;
    attack_x_ = player_.facing_right ? player_.x + player_.width : player_.x - attack_w_;
  }

  // Fire-breath skill: shoots a horizontal flame that instantly kills all
  // monsters in a straight line up to 10 tiles in the facing direction.
  constexpr float kFireRange = 10.0f;
  constexpr float kFireCooldown = 2.5f;
  constexpr float kFireDuration = 0.20f;
  if (player_.fire_cooldown > 0.0f) {
    player_.fire_cooldown = std::max(0.0f, player_.fire_cooldown - dt);
  }
  if (player_.fire_timer > 0.0f) {
    player_.fire_timer = std::max(0.0f, player_.fire_timer - dt);
  }
  if (input_.fire && player_.fire_cooldown <= 0.0f) {
    player_.fire_cooldown = kFireCooldown;
    player_.fire_timer = kFireDuration;
    fire_active_this_frame_ = true;

    fire_w_ = kFireRange;
    fire_h_ = player_.height;
    fire_y_ = player_.y;
    fire_x_ = player_.facing_right ? player_.x + player_.width : player_.x - kFireRange;
  }

  player_.vy = std::min(kMaxFallSpeed, player_.vy + kGravity * dt);

  moveWithCollisions(player_.x,
                     player_.y,
                     player_.vx,
                     player_.vy,
                     player_.width,
                     player_.height,
                     player_.on_ground,
                     map_,
                     dt);
}

void World::updateMonsters(float dt) {
  for (std::size_t i = 0; i < monsters_.size(); ++i) {
    Monster& m = monsters_[i];
    if (!m.alive) {
      continue;
    }

    if (m.touch_cooldown > 0.0f) {
      m.touch_cooldown = std::max(0.0f, m.touch_cooldown - dt);
    }

    const float dx = player_.x - m.x;
    if (std::fabs(dx) < 10.0f) {
      m.vx = sign(dx) * kMonsterChaseSpeed;
      m.facing_right = dx >= 0.0f;
    } else {
      m.vx = static_cast<float>(m.patrol_dir) * kMonsterPatrolSpeed;
    }

    m.vy = std::min(kMaxFallSpeed, m.vy + kGravity * dt);

    bool on_ground = false;
    const float old_x = m.x;
    moveWithCollisions(m.x, m.y, m.vx, m.vy, m.width, m.height, on_ground, map_, dt);

    if (std::fabs(m.x - old_x) < 0.001f) {
      m.patrol_dir *= -1;
    }
  }
}

void World::resolveCombat() {
  if (attack_active_this_frame_) {
    for (std::size_t i = 0; i < monsters_.size(); ++i) {
      Monster& m = monsters_[i];
      if (!m.alive) {
        continue;
      }
      if (intersects(attack_x_, attack_y_, attack_w_, attack_h_, m.x, m.y, m.width, m.height)) {
        m.hp -= 1;
        if (m.hp <= 0) {
          m.alive = false;
          ++kills_;
        }
      }
    }
  }

  // Fire-breath: instantly kills every monster inside the flame line.
  if (fire_active_this_frame_) {
    for (std::size_t i = 0; i < monsters_.size(); ++i) {
      Monster& m = monsters_[i];
      if (!m.alive) {
        continue;
      }
      if (intersects(fire_x_, fire_y_, fire_w_, fire_h_, m.x, m.y, m.width, m.height)) {
        m.alive = false;
        ++kills_;
      }
    }
  }

  for (std::size_t i = 0; i < monsters_.size(); ++i) {
    Monster& m = monsters_[i];
    if (!m.alive) {
      continue;
    }

    if (m.touch_cooldown <= 0.0f && player_.invuln_timer <= 0.0f &&
        intersects(player_.x, player_.y, player_.width, player_.height, m.x, m.y, m.width, m.height)) {
      m.touch_cooldown = 0.5f;
      player_.invuln_timer = 1.2f;
      player_.hp -= 1;
      // Knockback: push player away from monster with upward kick
      const float kKnockbackSpeed = 9.0f;
      player_.vx = (player_.x >= m.x ? 1.0f : -1.0f) * kKnockbackSpeed;
      player_.vy = -7.0f;
    }
  }
}

bool World::atGoal() const {
  const int cx = static_cast<int>(player_.x + player_.width * 0.5f);
  const int cy = static_cast<int>(player_.y + player_.height * 0.5f);
  if (cy < 0 || cy >= static_cast<int>(map_.size())) {
    return false;
  }
  if (cx < 0 || cx >= static_cast<int>(map_[cy].size())) {
    return false;
  }
  return map_[cy][cx] == 'G';
}

const std::vector<std::string>& World::tileMap() const {
  return map_;
}

const Player& World::player() const {
  return player_;
}

const std::vector<Monster>& World::monsters() const {
  return monsters_;
}

GameState World::state() const {
  return state_;
}

int World::kills() const {
  return kills_;
}

int World::totalMonsters() const {
  return static_cast<int>(monsters_.size());
}

bool World::playerAttackActive() const {
  return player_.attack_timer > 0.0f;
}

float World::playerAttackX() const {
  return attack_x_;
}

float World::playerAttackY() const {
  return attack_y_;
}

float World::playerAttackW() const {
  return attack_w_;
}

float World::playerAttackH() const {
  return attack_h_;
}

bool World::playerFireActive() const {
  return player_.fire_timer > 0.0f;
}

float World::playerFireX() const {
  return fire_x_;
}

float World::playerFireY() const {
  return fire_y_;
}

float World::playerFireW() const {
  return fire_w_;
}

float World::playerFireH() const {
  return fire_h_;
}

std::string World::buildHudText() const {
  std::ostringstream ss;
  ss << "HP:" << player_.hp << "  Kills:" << kills_ << "/" << monsters_.size();
  if (state_ == GameState::Won) {
    ss << "  [YOU WIN] press R to restart or Q to quit";
  } else if (state_ == GameState::Lost) {
    ss << "  [YOU LOSE] press R to restart or Q to quit";
  }
  return ss.str();
}

}  // namespace game
