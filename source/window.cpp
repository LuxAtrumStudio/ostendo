#include "window.hpp"

#include <stdarg.h>
#include <sstream>
#include <string>

#include <ncurses.h>
#include <pessum/pessum.hpp>

#include "ostendo_core.hpp"

ostendo::Window::Window() {}

ostendo::Window::Window(unsigned int state) : pos_(std_scr) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(std::string name, unsigned int state)
    : title_str_(name), pos_(std_scr) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(int width, int height, unsigned int state)
    : pos_(width, height) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(std::string name, int width, int height,
                        unsigned int state)
    : title_str_(name), pos_(width, height) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(int width, int height, int x, int y, unsigned int state)
    : pos_(width, height, x, y) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(std::array<int, 4> pos, unsigned int state)
    : pos_(pos) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(std::string name, int width, int height, int x, int y,
                        unsigned int state)
    : title_str_(name), pos_(width, height, x, y) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(std::string name, std::array<int, 4> pos,
                        unsigned int state)
    : title_str_(name), pos_(pos) {
  GenerateWindow();
  ReadState(state);
}

ostendo::Window::Window(const Window& copy)
    : auto_update_(copy.auto_update_),
      title_(copy.title_),
      border_(copy.border_),
      word_break_(copy.word_break_),
      title_position_(copy.title_position_),
      last_line_action_(copy.last_line_action_),
      title_str_(copy.title_str_),
      border_char_set_(copy.border_char_set_),
      base_color_(copy.base_color_),
      cursor_(copy.cursor_),
      color_(copy.color_),
      pos_(copy.pos_),
      ptr_(copy.ptr_) {}

ostendo::Window::~Window() {
  if (ptr_.use_count() == 1) {
    wclear(*ptr_);
    wrefresh(*ptr_);
    delwin(*ptr_);
  }
  ptr_ = NULL;
}

void ostendo::Window::DeleteWindow() {
  if (ptr_.use_count() == 1) {
    wclear(*ptr_);
    wrefresh(*ptr_);
    delwin(*ptr_);
  }
  ptr_ = NULL;
}

void ostendo::Window::SetBorder(bool setting) {
  if (border_ != setting && ptr_ != NULL) {
    border_ = setting;
    if (border_ == true) {
      DrawBorder();
    } else if (border_ == false) {
      EraseBorder();
    }
  }
}

void ostendo::Window::SetTitle(bool setting) {
  if (title_ != setting && ptr_ != NULL) {
    title_ = setting;
    if (title_ == true) {
      DrawTitle();
    } else if (title_ == false) {
      EraseTitle();
    }
  }
}

void ostendo::Window::SetTitleStr(std::string title) {
  if (title_ == true) {
    EraseTitle();
  }
  title_str_ = title;
  if (title_ == true) {
    DrawTitle();
  }
}

void ostendo::Window::SetTitleStr(const char* title) {
  if (title_ == true) {
    EraseTitle();
  }
  title_str_ = title;
  if (title_ == true) {
    DrawTitle();
  }
}

void ostendo::Window::SetTitlePosition(int position) {
  title_position_ = position;
  if (title_ == true) {
    EraseTitle();
    DrawTitle();
  }
}

void ostendo::Window::SetAutoUpdate(bool setting) { auto_update_ = setting; }

void ostendo::Window::SetWordBreak(bool setting) { word_break_ = setting; }

bool ostendo::Window::GetBorder() { return border_; }

bool ostendo::Window::GetTitle() { return title_; }

std::string ostendo::Window::GetTitleStr() { return title_str_; }

bool ostendo::Window::GetAutoUpdate() { return auto_update_; }

bool ostendo::Window::GetWordBreak() { return word_break_; }

void ostendo::Window::Scale(int dwidth, int dheight) {
  pos_.w += dwidth;
  pos_.h += dheight;
  if (ptr_ != NULL) {
    Resize();
  }
}

void ostendo::Window::Resize(int width, int height) {
  pos_.w = width;
  pos_.h = height;
  if (ptr_ != NULL) {
    Resize();
  }
}

void ostendo::Window::Translate(int dx, int dy) {
  pos_.x += dx;
  pos_.y += dy;
  if (ptr_ != NULL) {
    Move();
  }
}

void ostendo::Window::SetPosition(int x, int y) {
  pos_.x = x;
  pos_.y = y;
  if (ptr_ != NULL) {
    Move();
  }
}

void ostendo::Window::SetPosition(int width, int height, int x, int y) {
  pos_.w = width;
  pos_.h = height;
  pos_.x = x;
  pos_.y = y;
  if (ptr_ != NULL) {
    Resize();
    Move();
  }
}

void ostendo::Window::SetPosition(Position pos) {
  pos_ = pos;
  if (ptr_ != NULL) {
    Resize();
    Move();
  }
}

ostendo::Position ostendo::Window::GetPosition() {
  pessum::Log(pessum::DATA, "%ix%i @ (%i,%i,%i)",
              "ostendo::Window::GetPosition", pos_.w, pos_.h, pos_.x, pos_.y);
  return pos_;
}

void ostendo::Window::SetCursor(int x, int y) {
  if (ptr_ != NULL) {
    cursor_[0] = x;
    cursor_[1] = y;
  }
}

void ostendo::Window::GetCursor(int& x, int& y) {
  if (ptr_ != NULL) {
    x = cursor_[0];
    y = cursor_[1];
  }
}
void ostendo::Window::Print(std::string fmt, ...) {
  if (ptr_ == NULL) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  std::string str = FormatString(fmt, args);
  va_end(args);
  PrintStr(cursor_[0], cursor_[1], str);
  if (auto_update_ == true) {
    Update();
  }
}

void ostendo::Window::mvPrint(int x, int y, std::string fmt, ...) {
  if (ptr_ == NULL) {
    return;
  }
  if (x == -1) {
    x = cursor_[0];
  }
  if (y == -1) {
    y = cursor_[1];
  }
  va_list args;
  va_start(args, fmt);
  std::string str = FormatString(fmt, args);
  va_end(args);
  PrintStr(x, y, str);
  if (auto_update_ == true) {
    Update();
  }
}

int ostendo::Window::PrintSize(std::string fmt, ...) {
  int total_length = 0;
  va_list args;
  va_start(args, fmt);
  std::string str = FormatString(fmt, args);
  va_end(args);

  std::vector<std::string> blocks;
  int block_length, buffer_width = pos_.w, buffer_height = pos_.h;
  std::string new_block;
  UpdateColor();
  for (int i = 0; i < str.size(); i++) {
    if (str[i] == '\n' || block_length + cursor_[0] == buffer_width) {
      if (str[i] != '\n' && word_break_ == true) {
        while (i >= 0 && str[i] != ' ' && new_block.size() > 0) {
          new_block.pop_back();
          i--;
        }
        i++;
      }
      total_length += new_block.size();
      block_length = 0;
      new_block = std::string();
      if (cursor_[1] >= buffer_height) {
        HandleLastLine();
      }
    }
    if (str[i] == '$') {
      total_length += new_block.size();
      block_length = 0;
      new_block = std::string();
      std::string escape_block_in;
      i++;
      while (i < str.size() && str[i] != '$') {
        escape_block_in += str[i];
        i++;
      }
      std::string escape_block_out = ReadEscapeBlock(escape_block_in, false);
      new_block += escape_block_out;
      block_length += escape_block_out.size();
    } else if (str[i] != '\n') {
      new_block += str[i];
      block_length++;
    }
  }
  total_length += new_block.size();
  return total_length;
}

void ostendo::Window::ToggleAttribute(int attr, bool setting) {
  if (ptr_ != NULL) {
    if (setting == true) {
      wattron(*ptr_, attr);
    } else if (setting == false) {
      wattroff(*ptr_, attr);
    }
  }
}

void ostendo::Window::SetAttribute(unsigned int attr) {
  if (ptr_ != NULL) {
    wattrset(*ptr_, attr);
  }
}

void ostendo::Window::SetColor(int foreground, int background) {
  color_ = {{foreground, background}};
  UpdateColor();
}

void ostendo::Window::SetBaseColor(int window_element, int foreground,
                                   int background) {
  base_color_[window_element] = {{foreground, background}};
  if (window_element == WE_BASE) {
    if (ptr_ != NULL) {
      wbkgd(*ptr_,
            COLOR_PAIR((short)(base_color_[0][0] * 10 + base_color_[0][1])));
      if (title_ == true) {
        EraseTitle();
        DrawTitle();
      }
      if (border_ == true) {
        EraseBorder();
        DrawBorder();
      }
    }
  } else if (window_element == WE_TITLE) {
    EraseTitle();
    DrawTitle();
  } else if (window_element == WE_BORDER) {
    EraseBorder();
    DrawBorder();
  }
  if (auto_update_ == true) {
    Update();
  }
}

void ostendo::Window::SetBorderChars(std::array<unsigned long, 10> char_set) {
  border_char_set_ = char_set;
  if (border_ == true) {
    EraseBorder();
    DrawBorder();
  }
}

void ostendo::Window::SetBorderChar(int character, unsigned long ch) {
  border_char_set_[character] = ch;
  if (border_ == true) {
    EraseBorder();
    DrawBorder();
  }
}

void ostendo::Window::SetLastLineAction(int action) {
  last_line_action_ = action;
}

void ostendo::Window::Update() {
  if (ptr_ != NULL) {
    wrefresh(*ptr_);
  }
}

void ostendo::Window::Clear() {
  if (ptr_ != NULL) {
    color_ = base_color_[0];
    UpdateColor();
    wclear(*ptr_);
  }
  if (auto_update_ == true) {
    Update();
  }
}

void ostendo::Window::ClearAll() {
  if (ptr_ != NULL) {
    color_ = base_color_[0];
    UpdateColor();
    wclear(*ptr_);
    bool border_set = border_;
    bool title_set = title_;
    title_ = false;
    border_ = false;
    if (border_set == true) {
      EraseBorder();
    }
    if (title_set == true) {
      EraseTitle();
    }
    border_ = border_set;
    title_ = title_set;
  }
  if (auto_update_ == true) {
    Update();
  }
}

void ostendo::Window::ClearLine(int line) {
  if (line == -1) {
    line = cursor_[1];
  }
  if (ptr_ != NULL) {
    int width = pos_.w;
    std::array<int, 2> offset = {{0, 0}};
    std::string new_block;
    if (border_ == true) {
      width -= 2;
      offset = {{1, 1}};
    } else if (title_ == true) {
      offset = {{0, 1}};
    }
    mvwhline(*ptr_, line + offset[1], offset[0], ' ', width);
  }
  if (auto_update_ == true) {
    Update();
  }
}

void ostendo::Window::GenerateWindow() {
  if (ptr_ != NULL) {
    pessum::Log(pessum::WARNING, "Window already initialized",
                "ostendo::Window::GenerateWindow");
  } else {
    ptr_ = std::make_shared<WINDOW*>(newwin(pos_.h, pos_.w, pos_.y, pos_.x));
    if (ptr_ == NULL) {
      pessum::Log(pessum::ERROR, "Failed to initialize window",
                  "ostendo::Window::GenerateWindow");
    }
  }
}

void ostendo::Window::ReadState(unsigned int state) {
  if ((state & CENTER) != 0) {
    SetTitlePosition(CENTER);
  }
  if ((state & LEFT) != 0) {
    SetTitlePosition(LEFT);
  }
  if ((state & RIGHT) != 0) {
    SetTitlePosition(RIGHT);
  }
  if ((state & BORDER) != 0) {
    SetBorder(true);
  }
  if ((state & TITLE) != 0) {
    SetTitle(true);
  }
  if ((state & AUTO_UPDATE) != 0) {
    SetAutoUpdate(true);
  }
  if ((state & WORD_BREAK) != 0) {
    SetWordBreak(true);
  }
}

void ostendo::Window::DrawTitle() {
  int title_width = title_str_.size();
  int title_start = 0;
  if (title_position_ == LEFT) {
    if (border_ == true) {
      title_start = 2;
    }
  } else if (title_position_ == CENTER) {
    title_start = (pos_.w - title_width) / 2;
  } else if (title_position_ == RIGHT) {
    title_start = pos_.w - title_width;
    if (border_ == true) {
      title_start -= 2;
    }
  }
  if (border_ == true) {
    color_ = base_color_[2];
    UpdateColor();
    mvwaddch(*ptr_, 0, title_start - 1, border_char_set_[8]);
    mvwaddch(*ptr_, 0, title_start + title_width, border_char_set_[9]);
  }
  color_ = base_color_[3];
  UpdateColor();
  mvwprintw(*ptr_, 0, title_start, title_str_.c_str());
  Update();
}

void ostendo::Window::DrawBorder() {
  color_ = base_color_[2];
  UpdateColor();
  wborder(*ptr_, border_char_set_[0], border_char_set_[1], border_char_set_[2],
          border_char_set_[3], border_char_set_[4], border_char_set_[5],
          border_char_set_[6], border_char_set_[7]);
  Update();
  if (title_ == true) {
    DrawTitle();
  }
}

void ostendo::Window::EraseTitle() {
  int title_width = title_str_.size();
  int title_start = 0;
  if (title_position_ == LEFT) {
    if (border_ == true) {
      title_start = 2;
    }
  } else if (title_position_ == CENTER) {
    title_start = (pos_.w - title_width) / 2;
  } else if (title_position_ == RIGHT) {
    title_start = pos_.w - title_width;
    if (border_ == true) {
      title_start -= 2;
    }
  }
  if (border_ == true) {
    color_ = base_color_[3];
    UpdateColor();
    mvwhline(*ptr_, 0, title_start - 1, border_char_set_[2], title_width + 2);
  } else if (border_ == false) {
    color_ = base_color_[0];
    UpdateColor();
    mvwhline(*ptr_, 0, title_start, ' ', title_width);
  }
  Update();
}

void ostendo::Window::EraseBorder() {
  color_ = base_color_[0];
  UpdateColor();
  wborder(*ptr_, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  Update();
  if (title_ == true) {
    DrawTitle();
  }
}

void ostendo::Window::Resize() {
  if (ptr_ != NULL) {
    if (pos_.w != 0 && pos_.h != 0) {
      ClearAll();
      wresize(*ptr_, pos_.h, pos_.w);
      Update();
      if (border_ == true) {
        DrawBorder();
      }
      if (title_ == true) {
        DrawTitle();
      }
    } else {
      pessum::Log(pessum::WARNING, "New window size is invalid %ix%i",
                  "ostendo::Window::Resize", pos_.w, pos_.h);
    }
  }
}

void ostendo::Window::Move() {
  if (ptr_ != NULL) {
    if (pos_.w >= 0 && pos_.h >= 0) {
      ClearAll();
      mvwin(*ptr_, pos_.y, pos_.x);
      Update();
      if (border_ == true) {
        DrawBorder();
      }
      if (title_ == true) {
        DrawTitle();
      }
    } else {
      pessum::Log(pessum::WARNING, "New window position is invalid (%i,%i)",
                  "ostendo::Window::Resize", pos_.x, pos_.y);
    }
  }
}

std::string ostendo::Window::FormatString(std::string fmt, va_list args) {
  std::string formated_str = "";
  va_list buff_args;
  va_copy(buff_args, args);
  ssize_t buff_size = vsnprintf(NULL, 0, fmt.c_str(), buff_args);
  char* formated_string = new char[buff_size + 1];
  formated_string[buff_size] = '\0';
  vsprintf(formated_string, fmt.c_str(), args);
  formated_str = std::string(formated_string);
  va_end(buff_args);
  return formated_str;
}

void ostendo::Window::PrintStr(int x, int y, std::string str) {
  std::vector<std::string> blocks;
  int block_length, buffer_width = pos_.w, buffer_height = pos_.h;
  std::array<int, 2> offset = {{0, 0}};
  cursor_ = {{x, y}};
  std::string new_block;
  if (border_ == true) {
    buffer_width -= 2;
    buffer_height -= 2;
    offset = {{1, 1}};
  } else if (title_ == true) {
    offset = {{0, 1}};
    buffer_height -= 1;
  }
  color_ = base_color_[1];
  UpdateColor();
  for (int i = 0; i < str.size(); i++) {
    if (str[i] == '\n' || block_length + cursor_[0] == buffer_width) {
      if (str[i] != '\n' && word_break_ == true) {
        while (i >= 0 && str[i] != ' ' && new_block.size() > 0) {
          new_block.pop_back();
          i--;
        }
        i++;
      }
      mvwprintw(*ptr_, cursor_[1] + offset[1], cursor_[0] + offset[0],
                new_block.c_str());
      cursor_[1]++;
      cursor_[0] = 0;
      block_length = 0;
      new_block = std::string();
      if (cursor_[1] >= buffer_height) {
        HandleLastLine();
      }
    }
    if (str[i] == '$') {
      mvwprintw(*ptr_, cursor_[1] + offset[1], cursor_[0] + offset[0],
                new_block.c_str());
      cursor_[0] += block_length;
      block_length = 0;
      new_block = std::string();
      std::string escape_block_in;
      i++;
      while (i < str.size() && str[i] != '$') {
        escape_block_in += str[i];
        i++;
      }
      std::string escape_block_out = ReadEscapeBlock(escape_block_in);
      new_block += escape_block_out;
      block_length += escape_block_out.size();
    } else if (str[i] != '\n') {
      new_block += str[i];
      block_length++;
    }
  }
  mvwprintw(*ptr_, cursor_[1] + offset[1], cursor_[0] + offset[0],
            new_block.c_str());
  cursor_[0] += new_block.size();
  color_ = base_color_[0];
  UpdateColor();
}

std::string ostendo::Window::ReadEscapeBlock(std::string str, bool action) {
  std::string return_str;
  if (str.size() == 0) {
    return ("$");
  } else if (str == "0" && action == true) {
    SetColor(DEFAULT, DEFAULT);
    SetAttribute(0);
  }
  std::vector<std::string> blocks;
  std::stringstream ss(str);
  std::string new_block;
  while (std::getline(ss, new_block, ';')) {
    blocks.push_back(new_block);
  }
  for (int i = 0; i < blocks.size(); i++) {
    std::vector<std::string> args;
    std::stringstream ss_b(blocks[i]);
    std::string new_block_opt;
    while (std::getline(ss_b, new_block_opt, ':')) {
      args.push_back(new_block_opt);
    }
    if (args.size() == 1) {
      if (args[0] == "c" && action == true) {
        color_ = {{DEFAULT, DEFAULT}};
        UpdateColor();
      } else if (args[0] == "a") {
        SetAttribute(0);
      } else {
        SetAttribute(ParseAttr(args)[0]);
      }
    } else if (args.size() == 2) {
      if (args[0] == "fg" && action == true) {
        color_[0] = ParseColor(args[1]);
        UpdateColor();
      } else if (args[0] == "bg" && action == true) {
        color_[1] = ParseColor(args[1]);
        UpdateColor();
      } else if (action == true) {
        std::array<int, 2> attr = ParseAttr(args);
        if (attr[1] == 1) {
          ToggleAttribute(attr[0], true);
        } else if (attr[1] == 2) {
          ToggleAttribute(attr[0], false);
        } else if (attr[1] == 3) {
          SetAttribute(attr[0]);
        }
      }
    }
  }
  return return_str;
}

std::array<int, 2> ostendo::Window::ParseAttr(std::vector<std::string> args) {
  std::array<int, 2> attr = {{0, 3}};
  if (args[0] == "ac") {
    attr[0] = ALTCHARSET;
  } else if (args[0] == "bl") {
    attr[0] = BLINK;
  } else if (args[0] == "bo") {
    attr[0] = BOLD;
  } else if (args[0] == "di") {
    attr[0] = DIM;
  } else if (args[0] == "in") {
    attr[0] = INVIS;
  } else if (args[0] == "pr") {
    attr[0] = PROTECT;
  } else if (args[0] == "re") {
    attr[0] = REVERSE;
  } else if (args[0] == "st") {
    attr[0] = STANDOUT;
  } else if (args[0] == "un") {
    attr[0] = UNDERLINE;
  }
  if (args.size() == 2) {
    if (args[1] == "0" || args[1] == "off") {
      attr[1] = 2;
    } else if (args[1] == "1" || args[1] == "on") {
      attr[1] = 1;
    } else if (args[1] == "2" || args[1] == "set") {
      attr[1] = 3;
    }
  }
  return attr;
}

int ostendo::Window::ParseColor(std::string str) {
  int color = BLACK;
  if (str == "black" || str == "0") {
    color = BLACK;
  } else if (str == "red" || str == "1") {
    color = RED;
  } else if (str == "green" || str == "2") {
    color = GREEN;
  } else if (str == "yellow" || str == "3") {
    color = YELLOW;
  } else if (str == "blue" || str == "4") {
    color = BLUE;
  } else if (str == "magenta" || str == "5") {
    color = MAGENTA;
  } else if (str == "cyan" || str == "6") {
    color = CYAN;
  } else if (str == "white" || str == "7") {
    color = WHITE;
  } else if (str == "default" || str == "d") {
    color = DEFAULT;
  }
  return color;
}

void ostendo::Window::UpdateColor() {
  if (color_[0] == DEFAULT) {
    color_[0] = WHITE;
  }
  if (color_[1] == DEFAULT) {
    color_[1] = BLACK;
  }
  if (ptr_ != NULL) {
    wattron(*ptr_, COLOR_PAIR((short)(color_[0] * 10 + color_[1])));
  }
}

void ostendo::Window::HandleLastLine() {
  if (last_line_action_ == LLA_NONE) {
    cursor_[1]--;
    ClearLine();
  } else if (last_line_action_ == LLA_CLEAR) {
    Clear();
    if (title_ == true) {
      DrawTitle();
    }
    if (border_ == true) {
      DrawBorder();
    }
    cursor_[1] = 0;
  } else if (last_line_action_ == LLA_SCROLL) {
    cursor_[1]--;
    std::array<int, 2> start = {{0, 0}};
    if (border_ == true) {
      start = {{1, 1}};
    } else if (title_ == true) {
      start = {{0, 1}};
    }
    for (int i = start[1] + 1; i < pos_.h - start[1]; i++) {
      for (int j = start[0]; j < pos_.w - start[0]; j++) {
        unsigned long ch = mvwinch(*ptr_, i, j);
        mvwaddch(*ptr_, i - 1, j, ch);
      }
    }
    ClearLine();
  }
}
