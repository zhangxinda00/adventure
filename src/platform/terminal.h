#pragma once

#include <string>
#include <utility>
#include <vector>

#include "engine/input_state.h"

namespace platform {

class Terminal {
 public:
  Terminal();
  ~Terminal();

  Terminal(const Terminal&) = delete;
  Terminal& operator=(const Terminal&) = delete;

  bool enterRawMode();
  void leaveRawMode();

  std::pair<int, int> getSize() const;
  engine::InputState pollInput();
  void present(const std::vector<std::string>& lines);

 private:
  bool raw_mode_enabled_;
  int original_flags_;
  bool has_original_flags_;
  bool has_original_termios_;
  struct termios_storage;
  termios_storage* state_;
};

}  // namespace platform
