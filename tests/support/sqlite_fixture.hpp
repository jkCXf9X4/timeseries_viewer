#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>

#include <sqlite3.h>

namespace tsv::test {

namespace fs = std::filesystem;

inline fs::path make_sqlite_fixture(const fs::path& path) {
  std::error_code ec;
  fs::remove(path, ec);
  sqlite3* db = nullptr;
  if (sqlite3_open(path.string().c_str(), &db) != SQLITE_OK) {
    const std::string message = db != nullptr ? sqlite3_errmsg(db) : "failed to open sqlite database";
    if (db != nullptr) {
      sqlite3_close(db);
    }
    throw std::runtime_error(message);
  }

  const char* schema =
    "CREATE TABLE telemetry (timestamp TEXT, speed REAL, altitude REAL);"
    "INSERT INTO telemetry VALUES ('2024-01-01T00:00:00', 10.0, 100.0);"
    "INSERT INTO telemetry VALUES ('2024-01-01T00:00:01', 11.0, 101.0);"
    "INSERT INTO telemetry VALUES ('2024-01-01T00:00:02', 12.0, 102.0);"
    "CREATE TABLE metadata (id INTEGER, label TEXT);"
    "INSERT INTO metadata VALUES (1, 'alpha');"
    "INSERT INTO metadata VALUES (2, 'beta');";

  char* error = nullptr;
  if (sqlite3_exec(db, schema, nullptr, nullptr, &error) != SQLITE_OK) {
    const std::string message = error != nullptr ? error : "unknown sqlite error";
    sqlite3_free(error);
    sqlite3_close(db);
    throw std::runtime_error(message);
  }
  sqlite3_close(db);
  return path;
}

} // namespace tsv::test
