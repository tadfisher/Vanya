#include <pebble.h>

static const int16_t SCREEN_W = 144;
static const int16_t SCREEN_H = 168;

static const struct GPathInfo HAND_PATH_INFO = {
  .num_points = 4, 
  .points = (GPoint[]) {{-3, -120},   {-3, 120}, {3, 120},   {3, -120}}
};

GPath *hand_path;
Layer *hand_layer;
Window *window;

static void hand_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // TODO change to (total_minutes / 720) * 360 or whatever
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;

  gpath_rotate_to(hand_path, minute_angle);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_draw_filled(ctx, hand_path);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Init layers
  hand_layer = layer_create(bounds);
  layer_set_update_proc(hand_layer, hand_update_proc);
  layer_add_child(window_layer, hand_layer);
}

static void window_unload(Window *window) {
  layer_destroy(hand_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // Init paths
  hand_path = gpath_create(&HAND_PATH_INFO);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  const GPoint center = grect_center_point(&bounds);
  gpath_move_to(hand_path, center);

  // Push the window onto the stack
  const bool animated = true;
  window_stack_push(window, animated);
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();

  gpath_destroy(hand_path);

  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
