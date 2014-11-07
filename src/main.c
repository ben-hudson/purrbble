#include <pebble.h>
#include "png.h"

static Window *window;
static BitmapLayer *bitmap_layer;
static TextLayer *text_layer;
static GBitmap *bitmap;
static char text[8];

typedef enum {
  APP_KEY_SIZE,
  APP_KEY_DATA,
} AppKey;

static void received_image(DictionaryIterator *it, void *ctx) {
  static char *buffer = NULL;
  static uint16_t total = 0;
  static uint16_t index = 0;

  Tuple *tuple = dict_read_first(it);
  while (tuple) {
    switch (tuple->key) {
      case APP_KEY_SIZE: {
        if (buffer) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, "freeing buffer");
          free(buffer);
        }
        total = tuple->value->uint16;
        buffer = malloc(total);
        index = 0;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "allocating buffer of %d bytes", total);
      }
      break;
      case APP_KEY_DATA: {
        if (buffer) {
          memcpy(buffer + index, tuple->value->data, tuple->length);
          index += tuple->length;
          APP_LOG(APP_LOG_LEVEL_DEBUG, "receiving bytes %d/%d", index, total);

          if (index >= total) {
            gbitmap_destroy(bitmap);
            bitmap = gbitmap_create_with_png_data((uint8_t *)buffer, total);
            layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
            buffer = NULL;
            index = 0;
          }
        }
      }
      break;
    }
    tuple = dict_read_next(it);
  }
}

static void update_time(struct tm* time, TimeUnits units) {
  clock_copy_time_string(text, sizeof(text));
  text_layer_set_text(text_layer, text);
}

static void request_image(AccelAxisType axis, int32_t direction) {
  DictionaryIterator *it;
  app_message_outbox_begin(&it);
  dict_write_int(it, APP_KEY_DATA, 0, sizeof(int), true);
  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOADING);
  bitmap_layer = bitmap_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h - 40 } });
  bitmap_layer_set_bitmap(bitmap_layer, bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));

  clock_copy_time_string(text, sizeof(text));
  text_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h - 40 }, .size = { bounds.size.w, 40 } });
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(text_layer, text);
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(bitmap);
}

static void init(void) {
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_received(received_image);
  tick_timer_service_subscribe(MINUTE_UNIT, update_time);
  accel_tap_service_subscribe(request_image);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
