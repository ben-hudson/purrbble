#include "png.h"
#include <pebble.h>

static Window *window;
static BitmapLayer *bitmap_layer;
static TextLayer *text_layer;
static GBitmap *bitmap;
static char text[16];

typedef enum {
  APP_KEY_REQUEST,
  APP_KEY_RECEIVE,
} AppKey;

static void received_cat(DictionaryIterator *it, void *ctx) {
  Tuple *tuple = dict_read_first(it);
  char *buffer = malloc(tuple->length);
  memcpy(buffer, tuple->value->data, tuple->length);
  gbitmap_destroy(bitmap);
  bitmap = gbitmap_create_with_png_data((uint8_t *)buffer, tuple->length);
  layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
}

static void request_cat(AccelAxisType axis, int32_t direction) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, APP_KEY_REQUEST, 0, sizeof(int), true);
  app_message_outbox_send();
}

static void update_time(struct tm* time, TimeUnits units) {
  clock_copy_time_string(text, sizeof(text));
  text_layer_set_text(text_layer, text);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOADING);
  bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(bitmap_layer, bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));

  text_layer = text_layer_create((GRect) { .origin = {0, bounds.size.h - 40}, .size = {bounds.size.w, 40}});
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  update_time(NULL, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, update_time);

  request_cat(ACCEL_AXIS_X, 0);
  accel_tap_service_subscribe(request_cat);
}

static void window_unload(Window *window) {
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  text_layer_destroy(text_layer);
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(bitmap);
}

static void init(void) {
  app_message_register_inbox_received(received_cat);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

  window = window_create();
  window_set_fullscreen(window, true);
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  app_message_deregister_callbacks();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
