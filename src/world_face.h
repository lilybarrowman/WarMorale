#pragma once

class world;

#include "stage.h"
#include "team.h"
#include "sprite.h"
#include "polygon.h"
#include "matrix_3f.h"
#include "threat.h"
#include "explosion_effect.h"
#include "bitmap_text.h"
#include "running_average.h"

class world : public stage {

public:

  sprite_context s_ctx;
  polygon_context p_ctx;
  point_particle_context pp_ctx;
  bitmap_text_context bt_ctx;
  frame_rate_meter frm;

  long frame_count = 0;
  vector_2f mouse_pos = { 0, 0 };
  bool mouse_down = false;

  team* player_team;
  team* enemy_team;
  legion* player_first_legion;
  legion* enemy_first_legion;


  owning_polygon* tri;


  ordered_parent* teams_layer;
  threat_parent* threat_layer;
  ordered_parent* over_effects_layer; // A container for effects to render over-top the main game elements
  ordered_parent* ui_layer;

  fixed_width_bitmap_text* frame_rate_text;
  variable_width_bitmap_text* log_text;


  world(GLFWwindow* win, static_resources& sr, int_keyed_resources& dr);

  bool update() override;
  
  void key_callback(int key, int scancode, int action, int mods) override;
  void cursor_position_callback(double xpos, double ypos) override;
  void mouse_button_callback(int button, int action, int mods) override;


  sprite* world::static_sprite_orphan(static_texture_id id);
  sprite world::static_sprite(static_texture_id id);


  /*~world() {
    fprintf(stderr, "World Destructor\n");
  }*/

private:
  vector_2f window_to_world(double xpos, double ypos);
};