#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_module.h"
#include "chat_storage.h"

chat_message_t *chat_create_message(chat_storage_t *storage, bool is_sender,
                                    uint32_t chat_id, const char *text) {
  chat_message_t *msg = calloc(1, sizeof(chat_message_t));
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

int chat_delete_message(chat_storage_t *storage, uint32_t chat_id,
                        uint32_t id) {
  return storage->vtable->delete_message(storage, chat_id, id) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int chat_read_message(chat_storage_t *storage, uint32_t id) {
  return EXIT_SUCCESS;
}

chat_t *chat_create(chat_storage_t *storage, const char *name) {
  chat_t *chat = calloc(1, sizeof(chat_t));
  if (!chat) return NULL;

  chat->id = storage->vtable->chat_count(storage) + 1;

  if (name) {
    snprintf(chat->name, CHAT_MAX_NAME_LENGTH, "%s", name);
  }
  if (!storage->vtable->create_chat(storage, chat)) {
    free(chat);
    return NULL;
  }
  return chat;
}
int chat_delete(chat_storage_t *storage, uint32_t id) {
  bool res = storage->vtable->delete_chat(storage, id);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

chat_t *chat_get(chat_storage_t *storage, uint32_t id) {
  return storage->vtable->get_chat(storage, id);
}

uint32_t chat_get_length(chat_storage_t *storage, uint32_t id) { 
  chat_t *chat = chat_get(storage, id);
  if (!chat) return 0;
  return chat->message_count;
}

bool chat_has_unread_messages(chat_storage_t *storage, uint32_t id) {
  chat_t *chat = chat_get(storage, id);
  if (!chat) return false;
  for (uint32_t i = 0; i < chat->message_count; i++) {
    if (!chat->messages[i].is_read) {
      return true;
    }
  }
  return false;
}
