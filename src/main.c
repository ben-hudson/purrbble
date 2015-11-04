#include <pebble.h>

static Window *window;

static BitmapLayer *bitmap_layer;
static GBitmap *bitmap;

static Layer *text_layer;
static PropertyAnimation *text_animation;
static GCornerMask text_corners = GCornerNone;
static char text[16];

#define TEXT_HEIGHT 50
#define TEXT_MARGIN 10
#define TEXT_CORNER_RADIUS 12
#define TEXT_OVERFLOW_MODE GTextOverflowModeTrailingEllipsis
#define TEXT_ALIGNMENT GTextAlignmentCenter

static bool initialized = false;
static char *buffer = NULL;
static uint16_t total = 0;
static uint16_t index = 0;

typedef enum {
  APP_KEY_SIZE,
  APP_KEY_DATA,
} AppKey;

static void inbox_received_callback(DictionaryIterator *it, void *ctx) {
  Tuple *tuple = dict_read_first(it);
  while(tuple) {
    switch(tuple->key) {
    case APP_KEY_SIZE: {
      total = tuple->value->uint16;
      buffer = malloc(total);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "allocating buffer of %d bytes", total);
      index = 0;
    }
    break;
    case APP_KEY_DATA: {
      if (buffer) {
        memcpy(buffer + index, tuple->value->data, tuple->length);
        index += tuple->length;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "receiving bytes %d/%d", index, total);

        if (index >= total) {
          gbitmap_destroy(bitmap);
          bitmap = gbitmap_create_from_png_data((uint8_t *)buffer, total);
          bitmap_layer_set_bitmap(bitmap_layer, bitmap);
          layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));

          APP_LOG(APP_LOG_LEVEL_DEBUG, "freeing buffer");
          free(buffer);
          buffer = NULL;

          if (!initialized) {
            initialized = true;

            text_corners = GCornersAll;
            GRect from = layer_get_frame(text_layer);
            GRect to = GRect(TEXT_MARGIN, from.size.h - TEXT_HEIGHT, from.size.w - 2 * TEXT_MARGIN,
                             TEXT_HEIGHT - TEXT_MARGIN);
            text_animation = property_animation_create_layer_frame(text_layer, &from, &to);
            animation_schedule((Animation *)text_animation);
          }
        }
      }
    }
    break;
    }
    tuple = dict_read_next(it);
  }
}

static void minute_tick_handler(struct tm *time, TimeUnits units) {
  clock_copy_time_string(text, sizeof(text));
  layer_mark_dirty(text_layer);
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "requesting image...");
  DictionaryIterator *it;
  app_message_outbox_begin(&it);
  dict_write_int(it, APP_KEY_DATA, 0, sizeof(int), true);
  app_message_outbox_send();
}

static void text_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect frame = layer_get_frame(layer);
  frame.origin = GPoint(0, 0);
  graphics_fill_rect(ctx, frame, TEXT_CORNER_RADIUS, text_corners);

  GSize text_size = graphics_text_layout_get_content_size(text, fonts_get_system_font(FONT_KEY_GOTHIC_28), frame,
                                                          TEXT_OVERFLOW_MODE, TEXT_ALIGNMENT);
  GRect text_frame = GRect(0, (frame.size.h - text_size.h - 10) / 2, frame.size.w, frame.size.h);
  graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_28), text_frame, TEXT_OVERFLOW_MODE,
                     TEXT_ALIGNMENT, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  bitmap_layer = bitmap_layer_create(window_bounds);
  bitmap_layer_set_bitmap(bitmap_layer, bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));

  text_layer = layer_create(window_bounds);
  clock_copy_time_string(text, sizeof(text));
  layer_set_update_proc(text_layer, text_layer_update_proc);
  layer_add_child(window_layer, text_layer);
}

static void window_unload(Window *window) {
  layer_destroy(text_layer);
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(bitmap);
}

static void init(void) {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(1024, 28);
  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick_handler);
  accel_tap_service_subscribe(accel_tap_handler);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers){
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
