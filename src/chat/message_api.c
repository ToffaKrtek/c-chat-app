#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_module.h"
#include "chat_storage.h"

chat_message_t* chat_create_message(chat_storage_t* storage, bool is_sender,
                                    uint32_t chat_id, const char* text) {
  chat_message_t* msg = calloc(1, sizeof(chat_message_t));
  if (!msg) return NULL;

  msg->is_sender = is_sender;
  msg->is_read = is_sender;
  msg->chat_id = chat_id;
  if (text) {
    snprintf(msg->text, CHAT_MAX_TEXT_LENGTH, "%s", text);
  }
  msg->id = chat_get_length(storage, chat_id) + 1;

  if (!storage->vtable->create_message(storage, msg)) {
    free(msg);
    return NULL;
  }
  return msg;
}

int chat_delete_message(chat_storage_t* storage, uint32_t chat_id,
                        uint32_t id) {
  return storage->vtable->delete_message(storage, chat_id, id) ? EXIT_SUCCESS
                                                               : EXIT_FAILURE;
}

int chat_read_message(chat_storage_t* storage, uint32_t chat_id, uint32_t id) {
  chat_message_t* msg = storage->vtable->get_message(storage, chat_id, id);
  if (msg == NULL) return EXIT_FAILURE;
  msg->is_read = true;
  int res = storage->vtable->update_message(storage, msg) ? EXIT_SUCCESS
                                                          : EXIT_FAILURE;
  free(msg);
  return res;
}
