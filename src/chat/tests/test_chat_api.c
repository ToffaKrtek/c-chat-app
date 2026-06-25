#include <criterion/criterion.h>
#include <assert.h>
#include <criterion/internal/test.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat/chat_module.h"
#include "chat/chat_storage.h"

static chat_storage_t *storage;

void setup(void) {
    storage = chat_storage_new_memory();
}
void teardown(void) {
  chat_storage_free(storage);
}

Test(chat_api, create_chat, .init = setup, .fini = teardown) {
  chat_t *chat = chat_create(storage, "test");
  cr_assert(chat != NULL);
  cr_assert(strcmp(chat->name, "test") == 0);
  cr_assert(chat->id != 0);
  free(chat);
}

Test(chat_api, delete_chat_error, .init = setup, .fini = teardown) {
  bool res = chat_delete(storage, 999);
  cr_assert(res != EXIT_SUCCESS);
}

Test(chat_api, delete_chat, .init = setup, .fini = teardown) {
  chat_t *chat = chat_create(storage, "test");
  bool res = chat_delete(storage, chat->id);
  cr_assert(res == EXIT_SUCCESS);
  free(chat);
}

Test(chat_api, get_chat_error, .init = setup, .fini = teardown) {
  chat_t *chat = chat_get(storage, 999);
  cr_assert(chat == NULL);
  free(chat);
}

Test(chat_api, chat_get_length_zero_on_error, .init = setup, .fini = teardown) {
  uint32_t res = chat_get_length(storage, 999);
  cr_assert(res == 0);
}

Test(chat_api, chat_has_unread_messages, .init = setup, .fini = teardown) {
  chat_t *chat = chat_create(storage, "test");
  bool res = chat_has_unread_messages(storage, chat->id);
  cr_assert(res == false);
  free(chat);
}

