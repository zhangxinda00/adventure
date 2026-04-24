#pragma once

namespace engine {

struct InputState {
  bool left = false;
  bool right = false;
  bool jump = false;
  bool attack = false;
  bool fire = false;
  bool restart = false;
  bool quit = false;
};

}  // namespace engine
