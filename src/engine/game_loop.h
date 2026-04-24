#pragma once

namespace game {
class World;
}
namespace render {
class Renderer;
}
namespace platform {
class Terminal;
}

namespace engine {

class GameLoop {
 public:
  void run(game::World& world, render::Renderer& renderer, platform::Terminal& terminal);
};

}  // namespace engine
