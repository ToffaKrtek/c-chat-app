#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_module.h"
#include "chat_storage.h"

typedef struct {
  sqlite3* db;
} sqlite_backend_t;

// HELPERS
static char* duplicate_string(const char* str) {
  if (!str) return NULL;
  size_t len = strlen(str);
  char* dst = malloc(len + 1);
  if (dst) {
    memcpy(dst, str, len + 1);
  }
  return dst;
}
// HELPERS
//
// CORE
static bool sqlite_open(chat_storage_t* self, const char* url) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  int rc = sqlite3_open(url, &backend->db);
  if (rc != SQLITE_OK) {
    sqlite_close(backend->db);
    backend->db = NULL;
    return false;
  }
  const char* create_chat_sql =
      "CREATE TABLE IF NOT EXISTS chats ("
      "id INTEGER PRIMARY KEY,"
      "name TEXT"
      ");";
  const char* create_message_sql =
      "CREATE TABLE IF NOT EXISTS messages ("
      "id INTEGER PRIMARY KEY,"
      "chat_id INTEGER,"
      "is_sender INTEGER,"
      "is_read INTEGER,"
      "content TEXT,"
      ");";
  char* errmsg = NULL;
  rc = sqlite3_exec(backend->db, create_chat_sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    sqlite3_free(errmsg);
    sqlite_close(backend->db);
    backend->db = NULL;
    return false;
  }
  rc = sqlite3_exec(backend->db, create_message_sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    sqlite3_free(errmsg);
    sqlite_close(backend->db);
    backend->db = NULL;
    return false;
  }
  return true;
}

static void sqlite_close(chat_storage_t* self) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  if (backend) {
    if (backend->db) {
      sqlite3_close(backend->db);
    }
    free(backend);
    self->backend_data = NULL;
  }
}
// CORE
// MESSAGE
static bool sqlite_create_message(chat_storage_t* self,
                                  const chat_message_t* message) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql =
      "INSERT INTO messages (id, chat_id, is_sender, is_read, content) VALUES "
      "(?, ?, ?, ?, ?);";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_int(stmt, 1, message->id);
  sqlite3_bind_int(stmt, 2, message->chat_id);
  sqlite3_bind_int(stmt, 3, message->is_sender ? 1 : 0);
  sqlite3_bind_int(stmt, 4, message->is_read ? 1 : 0);
  sqlite3_bind_text(stmt, 5, message->content, -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

static chat_message_t* sqlite_get_message(chat_storage_t* self,
                                          uint32_t chat_id, uint32_t id) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "SELECT * FROM messages WHERE chat_id = ? AND id = ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return NULL;
  sqlite3_bind_int(stmt, 1, chat_id);
  sqlite3_bind_int(stmt, 2, id);
  chat_message_t* message = NULL;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    message = malloc(sizeof(chat_message_t));
    if (message) {
      message->id = sqlite3_column_int(stmt, 0);
      message->chat_id = sqlite3_column_int(stmt, 1);
      message->is_sender = sqlite3_column_int(stmt, 2) == 1 ? true : false;
      message->is_read = sqlite3_column_int(stmt, 3) == 1 ? true : false;
      const unsigned char* content = sqlite3_column_text(stmt, 4);
      message->content = duplicate_string((const char*)content);
    }
  }
  sqlite3_finalize(stmt);
  return message;
}

static bool sqlite_get_messages(chat_storage_t* self, uint32_t chat_id,
                                chat_message_t* out, uint32_t* count) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql =
      "SELECT * FROM messages WHERE chat_id = ? ORDER BY id LIMIT ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return NULL;
  sqlite3_bind_int(stmt, 1, chat_id);
  sqlite3_bind_int(stmt, 2, *count);
  uint32_t idx = 0;
  while (idx < *count && sqlite3_step(stmt) == SQLITE_ROW) {
    out[idx].id = sqlite3_column_int(stmt, 0);
    out[idx].chat_id = sqlite3_column_int(stmt, 1);
    out[idx].is_sender = sqlite3_column_int(stmt, 2) == 1 ? true : false;
    out[idx].is_read = sqlite3_column_int(stmt, 3) == 1 ? true : false;
    const unsigned char* content = sqlite3_column_text(stmt, 4);
    out[idx].content = duplicate_string((const char*)content);
    idx++;
  }
  *count = idx;
  sqlite3_finalize(stmt);
  return true;
}

static bool sqlite_update_message(chat_storage_t* self, const chat_message_t* message) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "UPDATE messages SET content = ?, is_read = ? WHERE chat_id = ? AND id = ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_text(stmt, 1, message->content, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, message->is_read ? 1 : 0);
  sqlite3_bind_int(stmt, 3, message->chat_id);
  sqlite3_bind_int(stmt, 4, message->id);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

static bool sqlite_delete_message(chat_storage_t* self, uint32_t chat_id,
                                  uint32_t id) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "DELETE FROM messages WHERE chat_id = ? AND id = ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_int(stmt, 1, chat_id);
  sqlite3_bind_int(stmt, 2, id);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}
// MESSAGE
// CHAT
static uint32_t sqlite_chat_count(chat_storage_t* self) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "SELECT COUNT(*) FROM chats;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return 0;
  uint32_t count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return count;
}

static bool sqlite_create_chat(chat_storage_t* self, const chat_t* chat) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "INSERT INTO chats (id, name) VALUES (?, ?);";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_int(stmt, 1, chat->id);
  sqlite3_bind_text(stmt, 2, chat->name, -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

static chat_t* sqlite_get_chat(chat_storage_t* self, uint32_t id) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "SELECT * FROM chats WHERE id = ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return NULL;
  sqlite3_bind_int(stmt, 1, id);
  chat_t* chat = NULL;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    chat = malloc(sizeof(chat_t));
    if (chat) {
      chat->id = sqlite3_column_int(stmt, 0);
      const unsigned char* name = sqlite3_column_text(stmt, 1);
      chat->name = duplicate_string((const char*)name);
      
      // TODO:: count messages
      // TODO:: messages
    }
  }
  sqlite3_finalize(stmt);
  return chat;
}

static bool sqlite_get_chats(chat_storage_t* self, chat_t* out,
                             uint32_t* count) {
  if (!out || !count) return false;
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;
  const char* sql = "SELECT * FROM chats LIMIT ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_int(stmt, 1, *count);
  uint32_t idx = 0;
  while (idx < *count && sqlite3_step(stmt) == SQLITE_ROW) {
    out[idx].id = sqlite3_column_int(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    out[idx].name = duplicate_string((const char*)name);
    // TODO messages
    idx++;
  }
  *count = idx;
  sqlite3_finalize(stmt);
  return true;
}

static bool sqlite_delete_chat(chat_storage_t* self, uint32_t id) {
  sqlite_backend_t* backend = (sqlite_backend_t*)self->backend_data;

  const char* del_msgs = "DELETE FROM messages WHERE chat_id = ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, del_msgs, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_int(stmt, 1, id);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (rc != SQLITE_DONE) return false;

  const char* sql = "DELETE FROM chats WHERE id = ?;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(backend->db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) return false;
  sqlite3_bind_int(stmt, 1, id);
  rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}
// CHAT
static const storage_v_table_t sqlite_vtable = {
  .open = sqlite_open,
  .close = sqlite_close,
  //CHAT
  .create_chat = sqlite_create_chat,
  .chat_count = sqlite_chat_count,
  .delete_chat = sqlite_delete_chat,
  .get_chat = sqlite_get_chat,
  .get_chats = sqlite_get_chats,

  //MESSAGE
  .create_message = sqlite_create_message,
  .delete_message = sqlite_delete_message,
  .get_message = sqlite_get_message,
  .update_message = sqlite_update_message,
  // ...
};

chat_storage_t* chat_storage_new_sqlite(void) {
  chat_storage_t* storage = malloc(sizeof(chat_storage_t));
  if (!storage) return NULL;
  sqlite_backend_t* backend = calloc(1, sizeof(sqlite_backend_t));
  if (!backend) {
    free(storage);
    return NULL;
  }
  storage->vtable = &sqlite_vtable;
  storage->backend_data = backend;
  return storage;
}

void chat_storage_free(chat_storage_t* storage) {
  if (!storage) return;
  if (storage->vtable && storage->vtable->close)
    storage->vtable->close(storage);
  free(storage);
}
