#include "platform/terminal.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <sstream>

namespace platform {

struct Terminal::termios_storage {
  termios original;
};

Terminal::Terminal()
    : raw_mode_enabled_(false),
      original_flags_(0),
      has_original_flags_(false),
      has_original_termios_(false),
      state_(new termios_storage()) {}

Terminal::~Terminal() {
  leaveRawMode();
  delete state_;
}

bool Terminal::enterRawMode() {
  if (raw_mode_enabled_) {
    return true;
  }

  termios raw;
  if (tcgetattr(STDIN_FILENO, &state_->original) == -1) {
    return false;
  }
  has_original_termios_ = true;

  raw = state_->original;
  raw.c_lflag &= static_cast<unsigned long>(~(ECHO | ICANON | IEXTEN | ISIG));
  raw.c_iflag &= static_cast<unsigned long>(~(IXON | ICRNL | BRKINT | INPCK | ISTRIP));
  raw.c_oflag &= static_cast<unsigned long>(~(OPOST));
  raw.c_cflag |= static_cast<unsigned long>(CS8);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    return false;
  }

  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  if (flags != -1) {
    original_flags_ = flags;
    has_original_flags_ = true;
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  }

  // Disable auto-wrap and switch to alternate buffer to prevent frame-by-frame scrolling.
  std::cout << "\x1b[?1049h\x1b[?25l\x1b[?7l\x1b[2J\x1b[H";
  std::cout.flush();

  raw_mode_enabled_ = true;
  return true;
}

void Terminal::leaveRawMode() {
  if (!raw_mode_enabled_) {
    return;
  }

  if (has_original_termios_) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &state_->original);
  }
  if (has_original_flags_) {
    fcntl(STDIN_FILENO, F_SETFL, original_flags_);
  }

  std::cout << "\x1b[0m\x1b[?7h\x1b[?25h\x1b[?1049l";
  std::cout.flush();

  raw_mode_enabled_ = false;
}

std::pair<int, int> Terminal::getSize() const {
  winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
    return std::make_pair(static_cast<int>(ws.ws_col), static_cast<int>(ws.ws_row));
  }
  return std::make_pair(80, 24);
}

engine::InputState Terminal::pollInput() {
  engine::InputState input;

  char buffer[64];
  while (true) {
    ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (n <= 0) {
      break;
    }

    for (ssize_t i = 0; i < n; ++i) {
      const unsigned char ch = static_cast<unsigned char>(buffer[i]);
      if (ch == 'a' || ch == 'A') {
        input.left = true;
      } else if (ch == 'd' || ch == 'D') {
        input.right = true;
      } else if (ch == 'w' || ch == 'W') {
        input.jump = true;
      } else if (ch == 'j' || ch == 'J') {
        input.attack = true;
      } else if (ch == 'q' || ch == 'Q') {
        input.quit = true;
      } else if (ch == 'r' || ch == 'R') {
        input.restart = true;
      } else if (ch == 27 && i + 2 < n && buffer[i + 1] == '[') {
        const char code = buffer[i + 2];
        if (code == 'A') {
          input.jump = true;
        } else if (code == 'D') {
          input.left = true;
        } else if (code == 'C') {
          input.right = true;
        }
        i += 2;
      }
    }
  }

  return input;
}

void Terminal::present(const std::vector<std::string>& lines) {
  std::ostringstream out;
  out << "\x1b[H";
  for (std::size_t i = 0; i < lines.size(); ++i) {
    // In raw mode OPOST is disabled, so '\n' does not behave like a terminal newline.
    // Use explicit cursor positioning per row to render a stable full-screen frame.
    out << "\x1b[" << (i + 1) << ";1H" << lines[i] << "\x1b[K";
  }
  std::cout << out.str();
  std::cout.flush();
}

}  // namespace platform
