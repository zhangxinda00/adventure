#include "engine/game_loop.h"

#include <chrono>
#include <thread>

#include "game/world.h"
#include "platform/terminal.h"
#include "render/renderer.h"

namespace engine {

void GameLoop::run(game::World& world, render::Renderer& renderer, platform::Terminal& terminal) {
  using clock = std::chrono::steady_clock;
  const std::chrono::duration<double> frame_budget(1.0 / 30.0);

  bool running = true;
  while (running) {
    const auto frame_start = clock::now();

    const InputState input = terminal.pollInput();
    if (input.quit) {
      running = false;
      continue;
    }
    if (input.restart && world.state() != game::GameState::Running) {
      world.reset();
    }

    world.handleInput(input);
    world.update(static_cast<float>(frame_budget.count()));
    renderer.render(world, terminal);

    const auto frame_elapsed = clock::now() - frame_start;
    if (frame_elapsed < frame_budget) {
      std::this_thread::sleep_for(frame_budget - frame_elapsed);
    }
  }
}

}  // namespace engine
