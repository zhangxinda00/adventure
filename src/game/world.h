#pragma once

#include <string>
#include <vector>

#include "engine/input_state.h"
#include "game/monster.h"
#include "game/player.h"

namespace game {

enum class GameState {
  Running,
  Won,
  Lost,
};

class World {
 public:
  World();

  void reset();
  void handleInput(const engine::InputState& input);
  void update(float dt);

  const std::vector<std::string>& tileMap() const;
  const Player& player() const;
  const std::vector<Monster>& monsters() const;

  GameState state() const;
  int kills() const;
  int totalMonsters() const;

  bool playerAttackActive() const;
  float playerAttackX() const;
  float playerAttackY() const;
  float playerAttackW() const;
  float playerAttackH() const;

  std::string buildHudText() const;

 private:
  void updatePlayer(float dt);
  void updateMonsters(float dt);
  void resolveCombat();
  bool atGoal() const;

  std::vector<std::string> map_;
  Player player_;
  std::vector<Monster> monsters_;
  engine::InputState input_;

  GameState state_;
  int kills_;

  float attack_x_;
  float attack_y_;
  float attack_w_;
  float attack_h_;
  bool attack_active_this_frame_;
};

}  // namespace game
