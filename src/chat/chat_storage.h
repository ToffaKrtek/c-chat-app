#ifndef CHAT_STORAGE_H
#define CHAT_STORAGE_H
#include <stdbool.h>
#include <stdint.h>

#include "chat_module.h"

typedef struct chat_storage_t chat_storage_t;
typedef struct {
  bool (*open)(chat_storage_t* self, const char* url);
  void (*close)(chat_storage_t* self);

  // CHAT
  chat_t* (*get_chat)(chat_storage_t* self, uint32_t id);
  bool (*get_chats)(chat_storage_t* self, chat_t* out, uint32_t* count);
  bool (*create_chat)(chat_storage_t* self, const chat_t* chat);
  bool (*update_chat)(chat_storage_t* self, chat_t chat);
  bool (*delete_chat)(chat_storage_t* self, uint32_t id);
  uint32_t (*chat_count)(chat_storage_t* self);

  // MESSAGE
  chat_message_t* (*get_message)(chat_storage_t* self, uint32_t chat_id,
                                 uint32_t id);
  bool (*get_messages)(chat_storage_t* self, uint32_t chat_id,
                       chat_message_t* out, uint32_t* count);
  bool (*create_message)(chat_storage_t* self, const chat_message_t* message);
  bool (*update_message)(chat_storage_t* self, const chat_message_t* message);
  bool (*delete_message)(chat_storage_t* self, uint32_t chat_id, uint32_t id);
} storage_v_table_t;

struct chat_storage_t {
  const storage_v_table_t* vtable;
  void* backend_data;
};

chat_storage_t* chat_storage_new_sqlite(const char* filename);
chat_storage_t* chat_storage_new_memory(void);
void chat_storage_free(chat_storage_t* storage);
#endif  // !CHAT_STORAGE_H
