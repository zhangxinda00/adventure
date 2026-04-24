#include "render/renderer.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "game/world.h"
#include "platform/terminal.h"

namespace render {

namespace {

std::string fitLine(const std::string& src, int width) {
  if (width <= 0) {
    return std::string();
  }
  if (static_cast<int>(src.size()) >= width) {
    return src.substr(0, static_cast<std::size_t>(width));
  }
  return src + std::string(static_cast<std::size_t>(width - src.size()), ' ');
}

int clampInt(int v, int lo, int hi) {
  if (v < lo) {
    return lo;
  }
  if (v > hi) {
    return hi;
  }
  return v;
}

void drawEntity(std::vector<std::string>& frame, int screen_x, int screen_y, char c) {
  if (screen_y < 0 || screen_y >= static_cast<int>(frame.size())) {
    return;
  }
  if (screen_x < 0 || screen_x >= static_cast<int>(frame[screen_y].size())) {
    return;
  }
  frame[screen_y][screen_x] = c;
}

}  // namespace

void Renderer::render(const game::World& world, platform::Terminal& terminal) {
  const auto size = terminal.getSize();
  const int width = size.first;
  const int height = size.second;

  if (width < 40 || height < 12) {
    std::vector<std::string> tiny;
    tiny.push_back(fitLine("Terminal too small. Need at least 40x12.", width));
    tiny.push_back(fitLine("Resize window, then continue.", width));
    for (int i = 2; i < height; ++i) {
      tiny.push_back(std::string(static_cast<std::size_t>(std::max(0, width)), ' '));
    }
    terminal.present(tiny);
    return;
  }

  const std::vector<std::string>& map = world.tileMap();
  const game::Player& p = world.player();
  const int map_h = static_cast<int>(map.size());
  const int map_w = map_h == 0 ? 0 : static_cast<int>(map[0].size());

  const int world_h = std::max(1, height - 2);
  const int camera_x = clampInt(static_cast<int>(std::floor(p.x)) - width / 2, 0, std::max(0, map_w - width));

  std::vector<std::string> frame;
  frame.reserve(static_cast<std::size_t>(height));
  frame.push_back(fitLine(world.buildHudText(), width));

  for (int sy = 0; sy < world_h; ++sy) {
    const int my = sy;
    std::string row;
    row.reserve(static_cast<std::size_t>(width));

    for (int sx = 0; sx < width; ++sx) {
      const int mx = camera_x + sx;
      char tile = ' ';
      if (my >= 0 && my < map_h && mx >= 0 && mx < map_w) {
        const char raw = map[my][mx];
        if (raw == '#') {
          tile = '#';
        } else if (raw == 'G') {
          tile = 'X';
        }
      }
      row.push_back(tile);
    }

    frame.push_back(row);
  }

  const int px = static_cast<int>(std::floor(p.x)) - camera_x;
  const int py = static_cast<int>(std::floor(p.y)) + 1;
  drawEntity(frame, px, py, '@');

  if (world.playerAttackActive()) {
    const int ax = static_cast<int>(std::floor(world.playerAttackX())) - camera_x;
    const int ay = static_cast<int>(std::floor(world.playerAttackY())) + 1;
    drawEntity(frame, ax, ay, '-');
    drawEntity(frame, ax + 1, ay, '-');
  }

  const std::vector<game::Monster>& monsters = world.monsters();
  for (std::size_t i = 0; i < monsters.size(); ++i) {
    const game::Monster& m = monsters[i];
    if (!m.alive) {
      continue;
    }
    const int mx = static_cast<int>(std::floor(m.x)) - camera_x;
    const int my = static_cast<int>(std::floor(m.y)) + 1;
    drawEntity(frame, mx, my, 'm');
  }

  frame.push_back(fitLine("A/D move  W jump  J attack  Q quit", width));
  terminal.present(frame);
}

}  // namespace render
