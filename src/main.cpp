#include <iostream>

#include "engine/game_loop.h"
#include "game/world.h"
#include "platform/terminal.h"
#include "render/renderer.h"

int main() {
  platform::Terminal terminal;
  if (!terminal.enterRawMode()) {
    std::cerr << "failed to initialize terminal raw mode" << std::endl;
    return 1;
  }

  game::World world;
  render::Renderer renderer;
  engine::GameLoop loop;

  loop.run(world, renderer, terminal);
  return 0;
}
