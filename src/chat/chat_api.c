#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_module.h"
#include "chat_storage.h"

chat_t* chat_create(chat_storage_t* storage, const char* name) {
  chat_t* chat = calloc(1, sizeof(chat_t));
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
int chat_delete(chat_storage_t* storage, uint32_t id) {
  bool res = storage->vtable->delete_chat(storage, id);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

chat_t* chat_get(chat_storage_t* storage, uint32_t id) {
  return storage->vtable->get_chat(storage, id);
}

uint32_t chat_get_length(chat_storage_t* storage, uint32_t id) {
  chat_t* chat = chat_get(storage, id);
  if (!chat) return 0;
  return chat->message_count;
}

bool chat_has_unread_messages(chat_storage_t* storage, uint32_t id) {
  chat_t* chat = chat_get(storage, id);
  if (!chat) return false;
  for (uint32_t i = 0; i < chat->message_count; i++) {
    if (!chat->messages[i]->is_read) {
      return true;
    }
  }
  return false;
}
