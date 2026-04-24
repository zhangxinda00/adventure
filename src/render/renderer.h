#pragma once

namespace game {
class World;
}
namespace platform {
class Terminal;
}

namespace render {

class Renderer {
 public:
  void render(const game::World& world, platform::Terminal& terminal);
};

}  // namespace render
