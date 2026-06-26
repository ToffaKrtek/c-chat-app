#ifndef CHAT_H
#define CHAT_H

#include <stdbool.h>
#include <stdint.h>

#define CHAT_MAX_MESSAGES 20
#define CHAT_MAX_CHATS 10
#define CHAT_MAX_NAME_LENGTH 128
#define CHAT_MAX_TEXT_LENGTH 1024

typedef struct chat_storage_t chat_storage_t;
chat_storage_t* chat_storage_new_sqlite(const char* filename);
chat_storage_t* chat_storage_new_memory(void);
void chat_storage_free(chat_storage_t* storage);

//MESSAGE
typedef struct {
  bool is_sender;
  bool is_read;
  uint32_t id;
  uint32_t chat_id;
  char text[CHAT_MAX_TEXT_LENGTH];
} chat_message_t;

chat_message_t* chat_create_message(chat_storage_t* storage, bool is_sender,
                                    uint32_t chat_id, const char* text);
int chat_delete_message(chat_storage_t* storage, uint32_t chat_id, uint32_t id);
int chat_read_message(chat_storage_t* storage, uint32_t chat_id, uint32_t id);
int chat_update_message(chat_storage_t* storage, const chat_message_t* message);
//MESSAGE

//CHAT
typedef struct {
  uint32_t id;
  uint32_t message_count;
  char name[CHAT_MAX_NAME_LENGTH];
  chat_message_t* messages[CHAT_MAX_MESSAGES];
} chat_t;

chat_t* chat_create(chat_storage_t* storage, const char* name);
int chat_delete(chat_storage_t* storage, uint32_t id);
chat_t* chat_get(chat_storage_t* storage, uint32_t id);
int chat_get_all(chat_storage_t* storage, chat_t* out, uint32_t* count);
bool chat_has_unread_messages(chat_storage_t* storage, uint32_t id);
uint32_t chat_get_length(chat_storage_t* storage, uint32_t id);
//CHAT

#endif  // !CHAT_H
