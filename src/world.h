#pragma once

#include "world_face.h"
#include "unit.h"
#include "team.h"
#include "units/grunt.h"
#include "space_buckets.h"


inline world::world(GLFWwindow* win, static_resources& sr, int_keyed_resources& dr) : stage(win, sr, dr) {
  std::array<unsigned int, 4> seed = world::get_seed();
  printf("Seeded PRNG with %08x-%08x-%08x-%08x\n", seed[0], seed[1], seed[2], seed[3]);
  std::seed_seq seed_val{ seed[0], seed[1], seed[2], seed[3] };
  gen = generator_type(seed_val);

  s_ctx.init(&static_res.get_shader(static_shader_id::sprite), &static_res.get_vertex_array(static_vertex_array_id::sprite));
  p_ctx.init(&static_res.get_shader(static_shader_id::polygon_fill), &static_res.get_shader(static_shader_id::line));
  pp_ctx.init(&static_res.get_shader(static_shader_id::point_particle));
  bt_ctx.init(&static_res.get_shader(static_shader_id::fixed_width_bitmap_text), &static_res.get_shader(static_shader_id::variable_width_bitmap_text));

  mouse_pos = { -width / 2.0f, height / 2.0f };


  tri = add_orphan(new owning_polygon(&p_ctx, simple_vertex_array::create_circle<3>()));
  tri->fill_color.floats = {0.0, 0.5f, 0.5f, 1.0f};
  tri->edge_color.floats = { 0.0, 0.2f, 0.2f, 1.0f };
  tri->edge_width = 0.3f;//0.1f;
  
  obstacle_layer = add_orphan(new obstacle_parent());
  {
    sprite img = static_sprite(static_texture_id::ceres);
    img.local_trans = matrix_3f::transformation_matrix(256, 256);
    obstacle* ceres = obstacle_layer->add_orphan(new circular_obstacle(110.0f, std::move(img),
      sharing_polygon(&p_ctx, &static_res.get_vertex_array(static_vertex_array_id::dodecagon))));
    ceres->trans.x = -300;
    ceres->trans.y = -200;
  }
  {
    sprite img = static_sprite(static_texture_id::ceres);
    img.local_trans = matrix_3f::transformation_matrix(140, 140);
    obstacle* ceres = obstacle_layer->add_orphan(new circular_obstacle(60.0f, std::move(img),
      sharing_polygon(&p_ctx, &static_res.get_vertex_array(static_vertex_array_id::dodecagon))));
    ceres->trans.x = 400;
    ceres->trans.y = -200;
  }

  {
    sprite img = static_sprite(static_texture_id::ceres);
    img.local_trans = matrix_3f::transformation_matrix(90, 90);
    obstacle* ceres = obstacle_layer->add_orphan(new circular_obstacle(40.0f, std::move(img),
      sharing_polygon(&p_ctx, &static_res.get_vertex_array(static_vertex_array_id::dodecagon))));
    ceres->trans.x = 400;
    ceres->trans.y = 200;
  }
  {
    sprite img = static_sprite(static_texture_id::ceres);
    img.local_trans = matrix_3f::transformation_matrix(45, 45);
    obstacle* ceres = obstacle_layer->add_orphan(new circular_obstacle(20.0f, std::move(img),
      sharing_polygon(&p_ctx, &static_res.get_vertex_array(static_vertex_array_id::dodecagon))));
    ceres->trans.x = 200;
    ceres->trans.y = 200;
  }

  teams_layer = add_orphan(new team_parent());
    
  player_team = teams_layer->add_orphan(new team(color::blue()));
  enemy_team = teams_layer->add_orphan(new team(color::red()));
  enemy_team->establish_hostility(player_team);

  {
    legion& p_first = player_team->create_legion();
    player_first_legion = &p_first;
    p_first.order.pos = { -width / 2.0f, height / 2.0f };
    for (int i = 0; i < 100; i++) {
      grunt* g = player_team->add_orphan(new grunt(*this, *player_team, &p_first));
      g->trans.x = -100.0f + 100.0f * rand_centered_float(get_generator());
      g->trans.y = 300.0f + 100.0f * rand_centered_float(get_generator());
    }
  }

  {
    legion& e_first = enemy_team->create_legion();
    enemy_first_legion = &e_first;
    for (int i = 0; i < 100; i++) {
      grunt* g = enemy_team->add_orphan(new grunt(*this, *enemy_team, &e_first));
      g->trans.x = 100.0f * rand_centered_float(get_generator());
      g->trans.y = 100.0f * rand_centered_float(get_generator());
    }
  }

  {
    threat_layer = add_orphan(new threat_parent());
    sprite fire_sprite = static_sprite(static_texture_id::fire);
    fire_sprite.local_trans = matrix_3f::transformation_matrix(32, 32);
    point_threat* fire = threat_layer->add_orphan(new point_threat(fire_sprite, 10));
    fire->trans.set_position({ 250, 200 });
  }

  over_effects_layer = add_orphan(new ordered_parent());
  ui_layer = add_orphan(new ordered_parent());

  {
    frame_rate_text = ui_layer->add_orphan(new fixed_width_bitmap_text(&bt_ctx, &(static_res.get_mono_font(static_mono_font_id::consolas_12))));
    vector_2f trans = window_to_world(0, 0);
    frame_rate_text->local_trans = matrix_3f::transformation_matrix(1, 1, 0, trans.x, trans.y);
    frame_rate_text->text_color = { 1, 1, 0, 1 };
  }

  {
    log_text = ui_layer->add_orphan(new variable_width_bitmap_text(&bt_ctx, &(static_res.get_prop_font(static_prop_font_id::impact_24)), "Hello World!"));
    vector_2f trans = window_to_world(0, height - log_text->font->char_height());
    log_text->local_trans = matrix_3f::transformation_matrix(1, 1, 0, trans.x, trans.y);
    log_text->text_color = { 0, 1, 1, 1 };
    
  }

  {
    sprite img = static_sprite(static_texture_id::mercury_square);
    img.local_trans = matrix_3f::transformation_matrix(256.0f, 256.0f);
    std::vector<vector_2f> verts{ { 93.0f, 93.0f },{ -93.0f, 93.0f },{ -93.0f, -93.f },{ 93.0f, -93.0f } };
    polygonal_obstacle* mercury = obstacle_layer->add_orphan(new polygonal_obstacle(std::move(verts), std::move(img), &p_ctx));
    mercury->trans.x = 0;
    mercury->trans.y = -200;

    /*for (int n = 0; n < 100; n++) {
      float ang = (n * math_consts::pi() * 2.0f) / 100;
      vector_2f pt = mercury->trans.get_position() + vector_2f::create_polar(ang, 160);
      vector_2f edge_pt = mercury->point_on_edge(pt);

      sprite fire_sprite = static_sprite(static_texture_id::fire);
      fire_sprite.local_trans = matrix_3f::transformation_matrix(32, 32);
      point_threat* fire = threat_layer->add_orphan(new point_threat(fire_sprite, 10));
      fire->trans.set_position(edge_pt);
    }*/
  }
}

inline bool world::update() { 
  frm.count_frame();
  frame_count += 1;
  frame_rate_text->text = string_format("FPS:%3.1f", frm.average_frame_rate());

  float ang = frame_count / 100.0f;
  enemy_first_legion->order.pos = vector_2f::create_polar(ang, 100);
  player_first_legion->order.pos = mouse_pos;
  tri->local_trans = matrix_3f::transformation_matrix(100, 100, ang + math_consts::pi());

  stage::update(); //update children



  // after children update
  s_ctx.update_projection(proj);
  p_ctx.update_projection(proj);
  pp_ctx.update_projection(proj);
  bt_ctx.update_projection(proj);
  return false;
}


inline void world::key_callback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

inline void world::cursor_position_callback(double xpos, double ypos) {
  mouse_pos = window_to_world(xpos, ypos);
}

inline void world::mouse_button_callback(int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      if (!mouse_down) {
        // press started
        //printf("Press\n");

        tri->arr.set_veritices(create_circle_verticies<4>());

        /*std::array<vector_2f, 2> line_arr{{{ -1, 0 }, { 0, 1 }}};
        tri->arr.set_veritices(line_arr);*/
      }
      mouse_down = true;
    } else {
      if (mouse_down) {
        // release started
        //printf("Release\n");
        tri->arr.set_veritices(create_circle_verticies<3>());
      }
      mouse_down = false;
    }
  }
}

inline vector_2f world::window_to_world(double xpos, double ypos) {
  vector_2f fixed = { static_cast<float>(xpos) - width / 2, height / 2 - static_cast<float>(ypos) };
  return fixed;
}

inline std::array<unsigned int, 4> world::get_seed() {
  /*std::random_device rd;
  unsigned int a = rd();
  unsigned int b = rd();
  unsigned int c = rd();
  unsigned int d = rd();
  // C++ has unspecified execution order of arguments thus the temporaries
  return { { a, b, c, d } };*/
  return { { 0xabedbead, 0xcafebabe, 0xdeadbeef, 0xfacefeed } };
}

inline sprite* world::static_sprite_orphan(static_texture_id id) {
  return s_ctx.create_orphan(&(static_res.get_texture(id)));
}

inline sprite world::static_sprite(static_texture_id id) {
  return s_ctx.create(&(static_res.get_texture(id)));
}

inline generator_type& world::get_generator() {
  return gen;
}
