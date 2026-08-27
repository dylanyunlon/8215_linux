// SPDX-License-Identifier: GPL-2.0-or-later
// Derived from MPD DatabaseSave.cxx / DirectorySave.cxx / SongSave.cxx
//
// 将三个文本序列化文件的功能合并为 SQLite 事务操作:
//   DatabaseSave.cxx   → db_save_internal  → db_save_sqlite
//   DirectorySave.cxx  → directory_save    → INSERT INTO songs (事务批量)
//   SongSave.cxx       → song_save         → 每首歌一条 INSERT
//
// 设计对标 Android:
//   HMediaService 扫描结果 → ContentProvider → SQLite
//   我们: scanner结果 → MPD Directory/Song树 → SQLite
//
// 文件位置: .../mpd_db/db/plugins/simple/DatabaseSaveSqlite.cxx

#include "DatabaseSaveSqlite.hxx"
#include "Directory.hxx"
#include "Song.hxx"
#include "db/DatabaseLock.hxx"
#include "tag/Tag.hxx"
#include "tag/Names.hxx"
#include "tag/Type.hxx"
#include "song/DetachedSong.hxx"
#include "time/ChronoUtil.hxx"

#include "sqlite3.h"

#include <cstring>
#include <stdexcept>
#include <string>

/*============================================================================
 * SQL schema
 *
 * 单表设计: songs 表存储所有歌曲的完整信息
 *   uri       = 目录路径 + "/" + 文件名 (MPD 的 Song::GetURI())
 *   dir_path  = 所在目录路径 (用于 visit_directory 查询)
 *   mtime     = 文件修改时间 (用于增量更新判断)
 *   duration  = 时长毫秒
 *   tag_*     = 各 ID3 标签字段 (artist/album/title/genre/...)
 *
 * 索引: dir_path (按目录查询), uri (去重)
 *==========================================================================*/

static const char *SQL_CREATE =
    "CREATE TABLE IF NOT EXISTS songs ("
    "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  uri       TEXT NOT NULL UNIQUE,"
    "  dir_path  TEXT NOT NULL DEFAULT '',"
    "  filename  TEXT NOT NULL DEFAULT '',"
    "  mtime     INTEGER DEFAULT 0,"
    "  added     INTEGER DEFAULT 0,"
    "  duration_ms INTEGER DEFAULT -1,"
    "  id3_parsed INTEGER DEFAULT 0,"
    "  tag_artist  TEXT DEFAULT '',"
    "  tag_album   TEXT DEFAULT '',"
    "  tag_title   TEXT DEFAULT '',"
    "  tag_track   TEXT DEFAULT '',"
    "  tag_genre   TEXT DEFAULT '',"
    "  tag_date    TEXT DEFAULT '',"
    "  tag_composer TEXT DEFAULT '',"
    "  tag_disc    TEXT DEFAULT '',"
    "  tag_comment TEXT DEFAULT '',"
    "  tag_album_artist TEXT DEFAULT ''"
    ");"
    "CREATE INDEX IF NOT EXISTS idx_songs_dir ON songs(dir_path);"
    "CREATE INDEX IF NOT EXISTS idx_songs_uri ON songs(uri);"
    "CREATE INDEX IF NOT EXISTS idx_songs_title ON songs(tag_title);"
    "CREATE INDEX IF NOT EXISTS idx_songs_artist ON songs(tag_artist);"
    "CREATE INDEX IF NOT EXISTS idx_songs_album ON songs(tag_album);";

static const char *SQL_INSERT =
    "INSERT OR REPLACE INTO songs "
    "(uri, dir_path, filename, mtime, added, duration_ms, id3_parsed, "
    " tag_artist, tag_album, tag_title, tag_track, tag_genre, "
    " tag_date, tag_composer, tag_disc, tag_comment, tag_album_artist) "
    "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

static const char *SQL_SELECT_ALL =
    "SELECT uri, dir_path, filename, mtime, added, duration_ms, id3_parsed, "
    "       tag_artist, tag_album, tag_title, tag_track, tag_genre, "
    "       tag_date, tag_composer, tag_disc, tag_comment, tag_album_artist "
    "FROM songs ORDER BY uri";

/*============================================================================
 * Helper: get tag value safely
 *==========================================================================*/
static const char *
get_tag(const Tag &tag, TagType type) {
    const char *v = tag.GetValue(type);
    return v ? v : "";
}

/*============================================================================
 * Helper: RAII sqlite3 handle
 *==========================================================================*/
class SqliteHandle {
public:
    explicit SqliteHandle(const char *path) {
        int rc = sqlite3_open(path, &db_);
        if (rc != SQLITE_OK) {
            std::string msg = "Cannot open SQLite DB: ";
            msg += sqlite3_errmsg(db_);
            sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error(msg);
        }
        /* WAL 模式: 读写并发 + 崩溃安全 */
        sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
        sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
    }

    ~SqliteHandle() {
        if (db_) sqlite3_close(db_);
    }

    SqliteHandle(const SqliteHandle &) = delete;
    SqliteHandle &operator=(const SqliteHandle &) = delete;

    sqlite3 *get() { return db_; }

    void exec(const char *sql) {
        char *errmsg = nullptr;
        int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            std::string msg = "SQLite exec failed: ";
            msg += errmsg ? errmsg : "unknown";
            sqlite3_free(errmsg);
            throw std::runtime_error(msg);
        }
    }

private:
    sqlite3 *db_ = nullptr;
};

/*============================================================================
 * Save: Directory tree → SQLite
 *
 * 递归遍历 MPD 的 Directory/Song 树，每首歌一条 INSERT。
 * 整个操作在一个事务里，10000 首 ~200ms。
 *==========================================================================*/

static void
save_directory_recursive(sqlite3_stmt *stmt, const Directory &dir,
                         const std::string &parent_path)
{
    /* 当前目录的完整路径 */
    std::string dir_path;
    if (dir.IsRoot()) {
        dir_path = "";
    } else if (parent_path.empty()) {
        dir_path = dir.GetName();
    } else {
        dir_path = parent_path + "/" + std::string(dir.GetName());
    }

    /* 写入该目录下的所有歌曲 */
    for (const auto &song : dir.songs) {
        std::string uri = song.GetURI();

        sqlite3_reset(stmt);

        /* 1: uri */
        sqlite3_bind_text(stmt, 1, uri.c_str(), -1, SQLITE_TRANSIENT);
        /* 2: dir_path */
        sqlite3_bind_text(stmt, 2, dir_path.c_str(), -1, SQLITE_TRANSIENT);
        /* 3: filename */
        sqlite3_bind_text(stmt, 3, song.filename.c_str(), -1, SQLITE_TRANSIENT);
        /* 4: mtime */
        sqlite3_bind_int64(stmt, 4,
            IsNegative(song.mtime) ? 0 :
            (int64_t)std::chrono::system_clock::to_time_t(song.mtime));
        /* 5: added */
        sqlite3_bind_int64(stmt, 5,
            IsNegative(song.added) ? 0 :
            (int64_t)std::chrono::system_clock::to_time_t(song.added));
        /* 6: duration_ms */
        int dur_ms = song.tag.duration.IsNegative() ? -1 : song.tag.duration.ToMS();
        sqlite3_bind_int(stmt, 6, dur_ms);

        /* 7: id3_parsed */
        sqlite3_bind_int(stmt, 7, song.id3_parsed);

        /* 8-17: tags */
        sqlite3_bind_text(stmt, 8,  get_tag(song.tag, TAG_ARTIST), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9,  get_tag(song.tag, TAG_ALBUM), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 10, get_tag(song.tag, TAG_TITLE), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 11, get_tag(song.tag, TAG_TRACK), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, get_tag(song.tag, TAG_GENRE), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, get_tag(song.tag, TAG_DATE), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 14, get_tag(song.tag, TAG_COMPOSER), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 15, get_tag(song.tag, TAG_DISC), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 16, get_tag(song.tag, TAG_COMMENT), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 17, get_tag(song.tag, TAG_ALBUM_ARTIST), -1, SQLITE_TRANSIENT);

        sqlite3_step(stmt);
    }

    /* 递归子目录 */
    for (const auto &child : dir.children) {
        if (child.IsMount()) continue;
        save_directory_recursive(stmt, child, dir_path);
    }
}

void
db_save_sqlite(const char *db_path, const Directory &root)
{
    SqliteHandle db(db_path);

    /* 建表 */
    db.exec(SQL_CREATE);

    /* 事务开始 — 这是性能的关键 */
    db.exec("BEGIN TRANSACTION;");

    /* 清空旧数据（全量替换） */
    db.exec("DELETE FROM songs;");

    /* 准备 INSERT 语句 */
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db.get(), SQL_INSERT, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        db.exec("ROLLBACK;");
        throw std::runtime_error(std::string("Prepare INSERT failed: ") +
                                 sqlite3_errmsg(db.get()));
    }

    /* 递归写入所有歌曲 */
    {
        const ScopeDatabaseLock protect;
        save_directory_recursive(stmt, root, "");
    }

    sqlite3_finalize(stmt);

    /* 事务提交 */
    db.exec("COMMIT;");
}

/*============================================================================
 * Load: SQLite → Directory tree
 *
 * 从 SQLite 读取所有歌曲记录，重建 MPD 的 Directory/Song 树。
 * 利用 Directory::MakeChild 自动创建中间目录节点。
 *==========================================================================*/
void
db_load_sqlite(const char *db_path, Directory &root)
{
    SqliteHandle db(db_path);

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db.get(), SQL_SELECT_ALL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Prepare SELECT failed: ") +
                                 sqlite3_errmsg(db.get()));
    }

    const ScopeDatabaseLock protect;

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        /* 读取字段 */
        const char *uri      = (const char *)sqlite3_column_text(stmt, 0);
        /* dir_path (col 1) 和 filename (col 2) 用于构建目录树 */
        const char *dir_path = (const char *)sqlite3_column_text(stmt, 1);
        const char *filename = (const char *)sqlite3_column_text(stmt, 2);
        int64_t mtime_val    = sqlite3_column_int64(stmt, 3);
        int64_t added_val    = sqlite3_column_int64(stmt, 4);
        int dur_ms           = sqlite3_column_int(stmt, 5);
        int id3_parsed_val   = sqlite3_column_int(stmt, 6);

        if (!uri || !filename) continue;

        /* 在目录树中找到/创建父目录 */
        Directory *dir = &root;
        if (dir_path && dir_path[0]) {
            /* 按 "/" 分割路径，逐级创建 */
            std::string_view path_sv(dir_path);
            size_t start = 0;
            while (start < path_sv.size()) {
                auto end = path_sv.find('/', start);
                if (end == std::string_view::npos)
                    end = path_sv.size();
                auto component = path_sv.substr(start, end - start);
                if (!component.empty())
                    dir = dir->MakeChild(component);
                start = end + 1;
            }
        }

        /* 创建 Song */
        auto song = std::make_unique<Song>(std::string(filename), *dir);

        /* mtime / added */
        if (mtime_val > 0)
            song->mtime = std::chrono::system_clock::from_time_t((time_t)mtime_val);
        if (added_val > 0)
            song->added = std::chrono::system_clock::from_time_t((time_t)added_val);

        /* id3_parsed */
        song->id3_parsed = id3_parsed_val;

        /* 构建 Tag */
        TagBuilder tag_builder;
        if (dur_ms >= 0)
            tag_builder.SetDuration(SignedSongTime::FromMS(dur_ms));

        /* 读取各 tag 列 (shifted +1 due to id3_parsed at col 6) */
        struct { int col; TagType type; } tag_cols[] = {
            {7,  TAG_ARTIST},
            {8,  TAG_ALBUM},
            {9,  TAG_TITLE},
            {10, TAG_TRACK},
            {11, TAG_GENRE},
            {12, TAG_DATE},
            {13, TAG_COMPOSER},
            {14, TAG_DISC},
            {15, TAG_COMMENT},
            {16, TAG_ALBUM_ARTIST},
        };

        for (const auto &tc : tag_cols) {
            const char *val = (const char *)sqlite3_column_text(stmt, tc.col);
            if (val && val[0])
                tag_builder.AddItem(tc.type, val);
        }

        song->tag = tag_builder.Commit();

        /* 加入目录 */
        dir->AddSong(std::move(song));
        count++;
    }

    sqlite3_finalize(stmt);
}
