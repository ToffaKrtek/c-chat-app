#include "chat_module.h"
#include "chat_storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    chat_t *chats[10];
    uint32_t chat_count;
} memory_backend_t;

static bool memory_open(chat_storage_t *self, const char *url) { /* игнорируем url */ return true; }
static void memory_close(chat_storage_t *self) { free(self->backend_data); }
static bool memory_create_chat(chat_storage_t *self, const chat_t *chat) { 
  memory_backend_t *backend = (memory_backend_t*)self->backend_data;
  if(backend->chat_count >= 10) return false;
  backend->chats[backend->chat_count] = chat;
  backend->chat_count++;
  return true;
}
static uint32_t memory_chat_count(chat_storage_t *self) { 
  memory_backend_t *backend = (memory_backend_t*)self->backend_data;
  return backend->chat_count; 
}

static bool memory_delete_chat(chat_storage_t *self, uint32_t id) {
  memory_backend_t *backend = (memory_backend_t*)self->backend_data;
  for(uint32_t i = 0; i < backend->chat_count; i++) {
    if(backend->chats[i] == NULL) continue;
    if(backend->chats[i]->id == id) {
      backend->chats[i] = NULL;
      return true;
    }
  }
  return false;
}

static chat_t *memory_get_chat(chat_storage_t *self, uint32_t id) {
  memory_backend_t *backend = (memory_backend_t*)self->backend_data;
  for(uint32_t i = 0; i < backend->chat_count; i++) {
    if(backend->chats[i] == NULL) continue;
    if(backend->chats[i]->id == id) {
      return backend->chats[i];
    }
  }
  return NULL;
}

static const storage_v_table_t memory_vtable = {
    .open = memory_open,
    .close = memory_close,
    .create_chat = memory_create_chat,
    .chat_count = memory_chat_count,
    .delete_chat = memory_delete_chat,
    .get_chat = memory_get_chat,
    // ...
};

chat_storage_t* chat_storage_new_memory(void) {
    chat_storage_t *storage = malloc(sizeof(chat_storage_t));
    if (!storage) return NULL;
    storage->vtable = &memory_vtable;
    memory_backend_t *backend = calloc(1, sizeof(memory_backend_t));
    if (!backend) { free(storage); return NULL; }
    storage->backend_data = backend;
    return storage;
}

void chat_storage_free(chat_storage_t *storage) {
  if (!storage) return;
  if(storage->vtable && storage->vtable->close) storage->vtable->close(storage);
  free(storage);
}
