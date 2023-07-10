#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <raylib.h>
#include <string>
#include <vector>

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"

const int window_width = 1024;
const int window_height = 768;

const Color bg_color = GetColor(0x111111FF);
const Color lane_zone_color = GetColor(0x161616FF);
const Color highlight_color = GetColor(0xAAAAAA0D);
const Color lane_sep_color = GetColor(0xFFFFFF88);
const Color primary_color = GetColor(0x888888FF);

const int sqr_size = window_height / 7;
const int lane_zone_size = sqr_size * 3;

struct Zone {
  int x, y;
  int width, height;

  int lane_size;

  std::vector<std::vector<int>> lanes;

  int highlight = -1;

  bool invert = false;

  Zone(int x, int y, int width, int height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->lane_size = width / 3;

    lanes.resize(3, std::vector<int>());
  }

  bool insert_roll(int roll) {

    if (this->highlight < 0)
      return false;

    std::vector<int> &lane = this->lanes[this->highlight];

    if (lane.size() == 3) {
      return false;
    }

    lane.push_back(roll);

    return true;
  }

  void remove_from_lane(int roll, int lane_idx) {

    assert(lane_idx >= 0);

    std::vector<int> &lane = this->lanes[lane_idx];

    lane.erase(std::remove_if(lane.begin(), lane.end(),
                              [&](const int &i) { return i == roll; }),
               lane.end());
  }

  void draw_raylib() {
    DrawRectangle(x, y, width, height, lane_zone_color);

    if (highlight >= 0) {
      DrawRectangle(x + lane_size * highlight, y, lane_size, height,
                    highlight_color);
    }

    draw_lines();
    draw_lanes_content();
  }

  void set_highlight(Vector2 mouse_pos) {
    for (int lx = 0; lx < 3; ++lx) {
      Rectangle r;
      r.x = x + lane_size * lx;
      r.y = y;
      r.width = lane_size;
      r.height = height;

      if (CheckCollisionPointRec(mouse_pos, r)) {
        this->highlight = lx;
        return;
      }
    }

    this->highlight = -1;
  }

  bool is_full() {
    int count = 0;

    for (auto &lane : this->lanes) {
      count += lane.size();
    }

    return count == 9;
  }

  int get_total_points() {
    int sum = 0;

    for (auto &lane : this->lanes) {
      sum += this->calculate_lane_points(lane);
    }
    return sum;
  }

private:
  void draw_lines() {
    DrawLine(x + lane_size * 1, y, x + lane_size * 1, y + height,
             lane_sep_color);
    DrawLine(x + lane_size * 2, y, x + lane_size * 2, y + height,
             lane_sep_color);
  }

  void draw_lanes_content() {
    for (int lx = 0; lx < 3; ++lx) {
      std::vector<int> &lane = this->lanes[lx];

      int points = calculate_lane_points(lane);

      int padding = 6;
      int points_font_size = 22;
      int points_x = this->x + lx * this->lane_size + this->lane_size / 2 -
                     points_font_size / 2;
      int points_y = this->y + (this->height * this->invert) -
                     (points_font_size - points_font_size * this->invert) *
                         ((1 - this->invert) * 2 - 1);
      points_y -= padding * ((1 - this->invert) * 2 - 1);

      DrawText(std::to_string(points).c_str(), points_x, points_y,
               points_font_size, primary_color);

      int ly = 0;
      for (int val : lane) {
        int val_x = this->x + lx * this->lane_size + this->lane_size / 3;
        int val_y = this->y + (this->height * this->invert) +
                    (ly + this->invert) * ((1 - this->invert) * 2 - 1) *
                        this->lane_size;

        DrawText(std::to_string(val).c_str(), val_x, val_y, this->lane_size,
                 primary_color);

        ly++;
      }
    }
  }

  int calculate_lane_points(std::vector<int> &lane) {

    std::map<int, int> dice_counts;

    for (int val : lane) {
      if (dice_counts.count(val) != 0) {
        dice_counts[val] += 1;
      } else {
        dice_counts[val] = 1;
      }
    }

    int points = 0;

    std::map<int, int>::iterator it = dice_counts.begin();

    while (it != dice_counts.end()) {

      int val = it->first;
      int count = it->second;
      points += val * count * count;
      it++;
    }

    return points;
  }
};

int roll_dice() { return (rand() % 6) + 1; }

int main(void) {

  srand(time(0));

  InitWindow(window_width, window_height, "boned");
  SetTargetFPS(60);

  Zone zone_top = Zone(window_width / 2 - lane_zone_size / 2, 0, lane_zone_size,
                       lane_zone_size);
  zone_top.invert = true;
  Zone zone_bot =
      Zone(window_width / 2 - lane_zone_size / 2,
           window_height - lane_zone_size, lane_zone_size, lane_zone_size);

  int roll = roll_dice();

  std::vector<Zone *> players = {&zone_bot, &zone_top};
  int turn = 0;

  bool game_end = false;
  bool should_exit = false;

  // TODO: end of game checks
  while (!should_exit && !WindowShouldClose()) {

    if (!game_end) {

      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (players[turn]->insert_roll(roll)) {
          players[1 - turn]->remove_from_lane(roll, players[turn]->highlight);
          players[turn]->highlight = -1;

          if (players[turn]->is_full()) {
            game_end = true;
          }
          turn = 1 - turn;

          roll = roll_dice();
        }
      }
      Vector2 mouse_pos = GetMousePosition();
      players[turn]->set_highlight(mouse_pos);
    }
    BeginDrawing();

    ClearBackground(bg_color);

    for (Zone *zone : players) {
      zone->draw_raylib();
    }

    if (roll > 0) {
      int roll_x = players[turn]->x - players[turn]->lane_size;
      int roll_y =
          players[turn]->y + players[turn]->height - players[turn]->lane_size;

      DrawText(std::to_string(roll).c_str(), roll_x, roll_y,
               players[turn]->lane_size, primary_color);
    }

    if (game_end) {
      int top_points = zone_top.get_total_points();
      int bot_points = zone_bot.get_total_points();

      std::string points_text = "top: " + std::to_string(top_points) +
                                " \nbot: " + std::to_string(bot_points) + " \n";

      std::string winner_text;


      if (top_points < bot_points) {
        winner_text = "\nWinner: bottom.";
      }
      else if (top_points > bot_points) {
        winner_text = "\nWinner: top.";
      }
      else if (top_points == bot_points) {
        winner_text = "Draw.";
      }

      int result =
          GuiMessageBox({(float)window_width / 2 - 100,
                         (float)window_height / 2 - 80, 100 * 2, 80 * 2},
                        winner_text.c_str(), points_text.c_str(), "close");

      if (result == 1) {
        should_exit = true;
      }
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
