#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_module.h"
#include "chat_storage.h"

typedef struct {
  chat_t* chats[10];
  uint32_t chat_count;
} memory_backend_t;

// HELPERS
int32_t find_chat_idx(chat_storage_t* self, uint32_t chat_id) {
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  for (uint32_t i = 0; i < backend->chat_count; i++) {
    if (backend->chats[i] == NULL) continue;
    if (backend->chats[i]->id == chat_id) {
      return i;
    }
  }
  return -1;
}
int32_t find_msg_idx(chat_storage_t* self, int32_t chat_idx, uint32_t msg_idx) {
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  if (chat_idx == -1) return -1;
  if (backend->chat_count <= (uint32_t)chat_idx) return -1;
  if (backend->chats[chat_idx] == NULL) return -1;
  chat_t* chat = backend->chats[chat_idx];
  for (uint32_t i = 0; i < chat->message_count; i++) {
    if (chat->messages[i]->id == msg_idx) return i;
  }
  return -1;
}
// HELPERS
//
// CORE
static bool memory_open(chat_storage_t* self,
                        const char* url) { 
  return true;
}
static void memory_close(chat_storage_t* self) { free(self->backend_data); }
// CORE
//
//  MESSAGE
static bool memory_create_message(chat_storage_t* self,
                                  const chat_message_t* message) {
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  int32_t chat_idx = find_chat_idx(self, message->chat_id);
  if (chat_idx == -1) return false;

  chat_t* chat = backend->chats[chat_idx];
  // TODO:: delete first, push to end
  if (chat->message_count >= CHAT_MAX_MESSAGES) return false;
  chat->messages[chat->message_count] = message;
  chat->message_count++;
  return true;
}

static bool memory_delete_message(chat_storage_t* self, uint32_t chat_id,
                                  uint32_t id) {
  int32_t chat_idx = find_chat_idx(self, chat_id);
  if (chat_idx == -1) return false;
  int32_t msg_idx = find_msg_idx(self, chat_idx, id);
  if (msg_idx == -1) return false;

  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  chat_t* chat = backend->chats[chat_idx];
  chat->messages[msg_idx] = chat->messages[chat->message_count - 1];
  chat->message_count--;
  return true;
}

static bool memory_update_message(chat_storage_t* self,
                                  const chat_message_t* message) {
  int32_t chat_idx = find_chat_idx(self, message->chat_id);
  if (chat_idx == -1) return false;
  int32_t msg_idx = find_msg_idx(self, chat_idx, message->id);
  if (msg_idx == -1) return false;
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  chat_t* chat = backend->chats[chat_idx];
  chat->messages[msg_idx] = message;
  return true;
}

static chat_message_t* memory_get_message(chat_storage_t* self,
                                          uint32_t chat_id, uint32_t id) {
  int32_t chat_idx = find_chat_idx(self, chat_id);
  if (chat_idx == -1) return NULL;
  int32_t msg_idx = find_msg_idx(self, chat_idx, id);
  if (msg_idx == -1) return NULL;
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  return backend->chats[chat_idx]->messages[msg_idx];
}
// MESSAGE
//
// CHAT
static bool memory_create_chat(chat_storage_t* self, const chat_t* chat) {
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  if (backend->chat_count >= 10) return false;
  backend->chats[backend->chat_count] = chat;
  backend->chat_count++;
  return true;
}
static uint32_t memory_chat_count(chat_storage_t* self) {
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  return backend->chat_count;
}

static bool memory_delete_chat(chat_storage_t* self, uint32_t id) {
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  for (uint32_t i = 0; i < backend->chat_count; i++) {
    if (backend->chats[i] == NULL) continue;
    if (backend->chats[i]->id == id) {
      backend->chats[i] = NULL;
      return true;
    }
  }
  return false;
}

static chat_t* memory_get_chat(chat_storage_t* self, uint32_t id) {
  int32_t idx = find_chat_idx(self, id);
  if (idx == -1) return NULL;
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  return backend->chats[idx];
}

static bool memory_get_chats(chat_storage_t* self, chat_t* out, uint32_t* count) {
  if (out == NULL || count == NULL) return false;
  memory_backend_t* backend = (memory_backend_t*)self->backend_data;
  uint32_t available = backend->chat_count;
  uint32_t to_copy = available > *count ? *count : available;
  for (uint32_t i = 0; i < to_copy; i++) {
    out[i] = *(backend->chats[i]);
  }
  *count = to_copy;
  return true;
}
// CHAT

static const storage_v_table_t memory_vtable = {
    .open = memory_open,
    .close = memory_close,
    // CHAT
    .create_chat = memory_create_chat,
    .chat_count = memory_chat_count,
    .delete_chat = memory_delete_chat,
    .get_chat = memory_get_chat,
    .get_chats = memory_get_chats,

    // MESSAGE
    .create_message = memory_create_message,
    .delete_message = memory_delete_message,
    .get_message = memory_get_message,
    .update_message = memory_update_message,
    // ...
};

chat_storage_t* chat_storage_new_memory(void) {
  chat_storage_t* storage = malloc(sizeof(chat_storage_t));
  if (!storage) return NULL;
  storage->vtable = &memory_vtable;
  memory_backend_t* backend = calloc(1, sizeof(memory_backend_t));
  if (!backend) {
    free(storage);
    return NULL;
  }
  storage->backend_data = backend;
  return storage;
}

void chat_storage_free(chat_storage_t* storage) {
  if (!storage) return;
  if (storage->vtable && storage->vtable->close)
    storage->vtable->close(storage);
  free(storage);
}
