#pragma once

#include <vector>
#include <string>

namespace game {

bool isSolidTile(const std::vector<std::string>& map, int tx, int ty);
bool intersects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

void moveWithCollisions(float& x,
                        float& y,
                        float& vx,
                        float& vy,
                        float w,
                        float h,
                        bool& on_ground,
                        const std::vector<std::string>& map,
                        float dt);

}  // namespace game
