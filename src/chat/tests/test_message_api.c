#include <assert.h>
#include <criterion/criterion.h>
#include <criterion/internal/test.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat/chat_module.h"
#include "chat/chat_storage.h"

static chat_storage_t* storage;
static void setup(void) { storage = chat_storage_new_memory(); }
static void teardown(void) { chat_storage_free(storage); }

Test(message_api, create_message, .init = setup, .fini = teardown) {
  chat_t* chat = chat_create(storage, "test");
  chat_message_t* message =
      chat_create_message(storage, true, chat->id, "test");
  cr_assert(message != NULL);
  cr_assert(strcmp(message->text, "test") == 0);
  cr_assert(message->id != 0);
}

Test(message_api, delete_message_no_chat_error, .init = setup,
     .fini = teardown) {
  int res = chat_delete_message(storage, 999, 999);
  cr_assert(res != EXIT_SUCCESS);
}

Test(message_api, delete_message_no_msg_error, .init = setup,
     .fini = teardown) {
  chat_t* chat = chat_create(storage, "test");
  int res = chat_delete_message(storage, chat->id, 999);
  cr_assert(res != EXIT_SUCCESS);
}

Test(message_api, delete_message, .init = setup, .fini = teardown) {
  chat_t* chat = chat_create(storage, "test");
  chat_message_t* message =
      chat_create_message(storage, true, chat->id, "test");
  int res = chat_delete_message(storage, chat->id, message->id);
  cr_assert(res == EXIT_SUCCESS);
  uint32_t res_len = chat_get_length(storage, chat->id);
  cr_assert(res_len == 0);
}

Test(message_api, update_message_error, .init = setup, .fini = teardown) {
  chat_message_t* message = calloc(1, sizeof(chat_message_t));
  message->id = 999;
  int res = chat_update_message(storage, message);
  cr_assert(res != EXIT_SUCCESS);
  free(message);
}

Test(message_api, update_message, .init = setup, .fini = teardown) {
  chat_t* chat = chat_create(storage, "test");
  chat_message_t* message =
      chat_create_message(storage, false, chat->id, "test");
  message->is_read = true;
  int res = chat_update_message(storage, message);
  cr_assert(res == EXIT_SUCCESS);
}

Test(message_api, read_message_error, .init = setup, .fini = teardown) {
  int res = chat_read_message(storage, 999, 999);
  cr_assert(res != EXIT_SUCCESS);
}
Test(message_api, read_message, .init = setup, .fini = teardown) {
  chat_t* chat = chat_create(storage, "test");
  cr_assert(chat != NULL);
  chat_message_t* message =
      chat_create_message(storage, false, chat->id, "test");
  cr_assert(message != NULL);
  int res = chat_read_message(storage, chat->id, message->id);
  cr_assert(res == EXIT_SUCCESS);
  int has_unread = chat_has_unread_messages(storage, chat->id);
  cr_assert(has_unread == false);
}
