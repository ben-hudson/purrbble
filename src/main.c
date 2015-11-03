#include <pebble.h>

static Window *window;

static BitmapLayer *bitmap_layer;
static GBitmap *bitmap;

static TextLayer *text_layer;
static Layer *text_background_layer;
static char text[16];

#define TEXT_LAYER_HEIGHT 50
#define TEXT_LAYER_MARGIN 10
#define TEXT_LAYER_CORNER_RADIUS 12

#define MIN(A, B) ((A) < (B) ? (A) : (B))

typedef enum {
  APP_KEY_SIZE,
  APP_KEY_DATA,
} AppKey;

static char *buffer = NULL;
static uint16_t total = 0;
static uint16_t index = 0;
static void inbox_received_callback(DictionaryIterator *it, void *ctx) {

  Tuple *tuple = dict_read_first(it);
  while (tuple) {
    switch (tuple->key) {
      case APP_KEY_SIZE: {
        if (buffer) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, "freeing buffer");
          free(buffer);
          buffer = NULL;
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
            bitmap = gbitmap_create_from_png_data((uint8_t *)buffer, total);
            bitmap_layer_set_bitmap(bitmap_layer, bitmap);
            layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
            // APP_LOG(APP_LOG_LEVEL_DEBUG, "freeing buffer");
            // free(buffer);
            // buffer = NULL;
            // index = 0;
          }
        }
      }
      break;
    }
    tuple = dict_read_next(it);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time(struct tm* time, TimeUnits units) {
  clock_copy_time_string(text, sizeof(text));
  text_layer_set_text(text_layer, text);
  // layer_mark_dirty(text_background_layer);
}

static void request_image(AccelAxisType axis, int32_t direction) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "requesting image...");

  // ResHandle handle = resource_get_handle(RESOURCE_ID_RAW_PNG);
  // size_t res_size = resource_size(handle);

  // buffer = (uint8_t *)malloc(res_size);
  // resource_load(handle, buffer, res_size);

  // gbitmap_destroy(bitmap);
  // bitmap = gbitmap_create_from_png_data(buffer, res_size);
  // if (bitmap == NULL) {
  //   APP_LOG(APP_LOG_LEVEL_DEBUG, "shit");
  // } else {
  //   bitmap_layer_set_bitmap(bitmap_layer, bitmap);
  //   layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
  // }
  DictionaryIterator *it;
  app_message_outbox_begin(&it);
  dict_write_int(it, APP_KEY_DATA, 0, sizeof(int), true);
  app_message_outbox_send();
}

static void text_layer_update(Layer *layer, GContext *ctx) {
  GRect rect = layer_get_bounds(layer);
  rect.origin.x += TEXT_LAYER_MARGIN;
  rect.size.w -= 2*TEXT_LAYER_MARGIN;
  rect.size.h -= TEXT_LAYER_MARGIN;
  graphics_fill_rect(ctx, rect, TEXT_LAYER_CORNER_RADIUS, GCornersAll);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // if (persist_exists((uint32_t)(-1))) {
  //   APP_LOG(APP_LOG_LEVEL_DEBUG, "exists!");
  //   total = persist_read_int((uint32_t)(-1));
  //   buffer = malloc(total);
  //   size_t i, read_index = 0;
  //   for (i = 0; read_index < total; i++) {
  //     persist_read_data(i, &buffer[read_index], MIN(PERSIST_DATA_MAX_LENGTH, total - read_index));
  //     read_index += MIN(PERSIST_DATA_MAX_LENGTH, total - read_index);
  //     APP_LOG(APP_LOG_LEVEL_DEBUG, "writing %d/%d bytes", read_index, total);
  //   }
  //   bitmap = gbitmap_create_from_png_data((uint8_t *)buffer, total);
  // } else {
    bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOADING);
  // }
  bitmap_layer = bitmap_layer_create(window_bounds);
  bitmap_layer_set_bitmap(bitmap_layer, bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));

  GRect text_layer_rect = (GRect) {
    .origin = {0, window_bounds.size.h - TEXT_LAYER_HEIGHT},
    .size = {window_bounds.size.w, TEXT_LAYER_HEIGHT}
  };
  text_background_layer = layer_create((text_layer_rect));
  layer_set_update_proc(text_background_layer, text_layer_update);

  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = text_layer_rect.size });
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  clock_copy_time_string(text, sizeof(text));
  text_layer_set_text(text_layer, text);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_background_layer);
  layer_add_child(text_background_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  layer_destroy(text_background_layer);

  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(bitmap);
  if (buffer) {
  //   size_t i, write_index = 0;
  //   persist_write_int((uint32_t)(-1), total);
  //   for (i = 0; write_index < total; i++) {
  //     persist_write_data(i, &buffer[write_index], MIN(PERSIST_DATA_MAX_LENGTH, total - write_index));
  //     write_index += MIN(PERSIST_DATA_MAX_LENGTH, total - write_index);
  //     APP_LOG(APP_LOG_LEVEL_DEBUG, "writing %d/%d bytes", write_index, total);
  //   }
  //   // write stuff to presist
    free(buffer);
  }
}

static void init(void) {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(1024, 24 + sizeof(int));
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
