#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_module.h"
#include "chat_storage.h"

chat_message_t *chat_create_message(chat_storage_t *storage, bool is_sender, uint32_t chat_id,
                                    const char *text) {
  chat_message_t *msg = calloc(1, sizeof(chat_message_t));
  if (!msg) return NULL;

  msg->is_sender = is_sender;
  msg->is_read = false;
  msg->chat_id = chat_id;
  if (text) {
    snprintf(msg->text, CHAT_MAX_TEXT_LENGTH, "%s", text);
  }
  return msg;
}

bool add_message_to_chat(chat_message_t *storage, chat_t *chat, chat_message_t *msg) {
  // TODO:: append or push --- direction
  if (chat->message_count >= CHAT_MAX_MESSAGES) {
    return false;
  }
  chat->messages[chat->message_count] = *msg;
  chat->message_count++;
  return true;
}

int chat_delete_message(chat_storage_t *storage, uint32_t chat_id, uint32_t id) { return EXIT_SUCCESS; }

int chat_read_message(chat_storage_t *storage, uint32_t id) { return EXIT_SUCCESS; }

chat_t *chat_create(chat_storage_t *storage, const char *name) {
  chat_t *chat = calloc(1, sizeof(chat_t));
  if (!chat) return NULL;

  chat->id = storage->vtable->chat_count(storage) + 1;

  if (name) {
    snprintf(chat->name, CHAT_MAX_NAME_LENGTH, "%s", name);
  }
  if(!storage->vtable->create_chat(storage, chat)){
    free(chat);
    return NULL;
  }
  return chat;
}
int chat_delete(chat_storage_t *storage, uint32_t id) { \
  bool res = storage->vtable->delete_chat(storage, id);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

chat_t *chat_get(chat_storage_t *storage, uint32_t id) { 
  return storage->vtable->get_chat(storage, id); 
}

uint32_t chat_get_length(chat_storage_t *storage, uint32_t id) { return 0; }

bool chat_has_unread_messages(chat_storage_t *storage, uint32_t id) { return false; }
