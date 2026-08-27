/*
 * mpd_db_bridge.cxx — Bridge between C musicplayer and MPD's C++ SimpleDatabase
 *
 * This file implements the extern "C" API defined in mpd_db_bridge.h,
 * wrapping MPD's SimpleDatabase/Directory/Song/Tag classes.
 */

#include "mpd_db_bridge.h"

#include "config.h"
#include "db/plugins/simple/SimpleDatabasePlugin.hxx"
#include "db/plugins/simple/Directory.hxx"
#include "db/plugins/simple/Song.hxx"
#include "db/DatabaseLock.hxx"
#include "db/Selection.hxx"
#include "tag/Tag.hxx"
#include "tag/Builder.hxx"
#include "tag/Type.hxx"
#include "song/DetachedSong.hxx"
#include "fs/AllocatedPath.hxx"

#include <cstring>
#include <memory>
#include <string>

/* Internal state */
struct mpd_db {
    std::unique_ptr<SimpleDatabase> db;
    std::string path_str;
};

/* Map our C enum to MPD's TagType */
static TagType bridge_tag_to_mpd(mpd_tag_type_t t)
{
    /* They share the same order, but let's be explicit for safety */
    static_assert((int)MPD_TAG_ARTIST == (int)TAG_ARTIST, "tag enum mismatch");
    static_assert((int)MPD_TAG_TITLE == (int)TAG_TITLE, "tag enum mismatch");
    static_assert((int)MPD_TAG_ALBUM == (int)TAG_ALBUM, "tag enum mismatch");
    static_assert((int)MPD_TAG_GENRE == (int)TAG_GENRE, "tag enum mismatch");
    if ((int)t >= (int)TAG_NUM_OF_ITEM_TYPES)
        return TAG_NUM_OF_ITEM_TYPES;
    return (TagType)(int)t;
}

extern "C" {

mpd_db_t *mpd_db_open(const char *db_path)
{
    try {
        auto *h = new mpd_db();
        h->path_str = db_path;

        auto path = AllocatedPath::FromUTF8(db_path);
        h->db = std::make_unique<SimpleDatabase>(
            std::move(path),
            false,  /* compress */
            true    /* hide_playlist_targets */
        );

        h->db->Open();
        return h;
    } catch (...) {
        return nullptr;
    }
}

void mpd_db_close(mpd_db_t *db)
{
    if (!db) return;
    try {
        db->db->Close();
    } catch (...) {}
    delete db;
}

int mpd_db_save(mpd_db_t *db)
{
    if (!db) return -1;
    try {
        db->db->Save();
        return 0;
    } catch (...) {
        return -1;
    }
}

int mpd_db_add_song(mpd_db_t *db, const mpd_song_info_t *info)
{
    if (!db || !info || !info->uri) return -1;

    try {
        const ScopeDatabaseLock protect;

        Directory &root = db->db->GetRoot();

        /* Parse the URI to get directory and filename */
        std::string_view uri_sv(info->uri);
        std::string dir_path;
        std::string filename;

        auto slash = uri_sv.rfind('/');
        if (slash != std::string_view::npos) {
            dir_path = std::string(uri_sv.substr(0, slash));
            filename = std::string(uri_sv.substr(slash + 1));
        } else {
            filename = std::string(uri_sv);
        }

        /* Navigate/create directory path */
        Directory *dir = &root;
        if (!dir_path.empty()) {
            size_t start = 0;
            while (start < dir_path.size()) {
                auto end = dir_path.find('/', start);
                if (end == std::string::npos)
                    end = dir_path.size();
                auto component = std::string_view(dir_path).substr(start, end - start);
                if (!component.empty())
                    dir = dir->MakeChild(component);
                start = end + 1;
            }
        }

        /* Check if song already exists with same mtime (incremental update) */
        Song *existing = dir->FindSong(filename);
        if (existing) {
            auto existing_mtime = std::chrono::system_clock::to_time_t(existing->mtime);
            if (info->mtime > 0 && existing_mtime == info->mtime) {
                /* Same mtime, skip update */
                return 0;
            }
            /* Remove old version */
            dir->RemoveSong(existing);
        }

        /* Build the tag */
        TagBuilder tag_builder;

        if (info->duration_ms >= 0)
            tag_builder.SetDuration(SignedSongTime::FromMS(info->duration_ms));

        for (int i = 0; i < MPD_TAG_COUNT; i++) {
            if (info->tags[i] && info->tags[i][0]) {
                TagType tt = bridge_tag_to_mpd((mpd_tag_type_t)i);
                if (tt < TAG_NUM_OF_ITEM_TYPES)
                    tag_builder.AddItem(tt, info->tags[i]);
            }
        }

        /* Create the song */
        auto song = std::make_unique<Song>(std::string(filename), *dir);
        song->tag = tag_builder.Commit();
        song->id3_parsed = info->id3_parsed;

        if (info->mtime > 0)
            song->mtime = std::chrono::system_clock::from_time_t(info->mtime);

        dir->AddSong(std::move(song));
        return 0;
    } catch (...) {
        return -1;
    }
}

int mpd_db_remove_song(mpd_db_t *db, const char *uri)
{
    if (!db || !uri) return -1;

    try {
        const ScopeDatabaseLock protect;
        Directory &root = db->db->GetRoot();

        std::string_view uri_sv(uri);
        auto slash = uri_sv.rfind('/');

        Directory *dir = &root;
        std::string_view filename;

        if (slash != std::string_view::npos) {
            auto r = root.LookupDirectory(uri_sv.substr(0, slash));
            dir = r.directory;
            filename = uri_sv.substr(slash + 1);
        } else {
            filename = uri_sv;
        }

        Song *song = dir->FindSong(filename);
        if (!song) return -1;

        dir->RemoveSong(song);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* Helper struct for visiting */
struct visit_context {
    mpd_song_visitor_fn visitor;
    void *user_data;
    int count;
    mpd_song_info_t info;
    std::string uri_buf;
    /* Tag value buffers (GetValue returns pointer to internal data) */
};

static void visit_directory_songs(const Directory &dir,
                                  struct visit_context *ctx)
{
    for (const auto &song : dir.songs) {
        /* Build the full URI */
        ctx->uri_buf = song.GetURI();

        memset(&ctx->info, 0, sizeof(ctx->info));
        ctx->info.uri = ctx->uri_buf.c_str();
        ctx->info.mtime = std::chrono::system_clock::to_time_t(song.mtime);
        ctx->info.id3_parsed = song.id3_parsed;

        if (!song.tag.duration.IsNegative())
            ctx->info.duration_ms = song.tag.duration.ToMS();
        else
            ctx->info.duration_ms = -1;

        /* Extract tags */
        for (int i = 0; i < MPD_TAG_COUNT; i++) {
            TagType tt = bridge_tag_to_mpd((mpd_tag_type_t)i);
            if (tt < TAG_NUM_OF_ITEM_TYPES)
                ctx->info.tags[i] = song.tag.GetValue(tt);
        }

        ctx->count++;
        if (ctx->visitor(&ctx->info, ctx->user_data) != 0)
            return;
    }

    /* Recurse into subdirectories */
    for (const auto &child : dir.children) {
        visit_directory_songs(child, ctx);
    }
}

int mpd_db_visit_all(mpd_db_t *db, mpd_song_visitor_fn visitor, void *user_data)
{
    if (!db || !visitor) return -1;

    try {
        const ScopeDatabaseLock protect;
        Directory &root = db->db->GetRoot();

        struct visit_context ctx;
        ctx.visitor = visitor;
        ctx.user_data = user_data;
        ctx.count = 0;

        visit_directory_songs(root, &ctx);
        return ctx.count;
    } catch (...) {
        return -1;
    }
}

int mpd_db_visit_directory(mpd_db_t *db, const char *dir_uri,
                           mpd_song_visitor_fn visitor, void *user_data)
{
    if (!db || !visitor) return -1;

    try {
        const ScopeDatabaseLock protect;
        Directory &root = db->db->GetRoot();

        Directory *target = &root;
        if (dir_uri && dir_uri[0]) {
            auto r = root.LookupDirectory(dir_uri);
            target = r.directory;
        }

        struct visit_context ctx;
        ctx.visitor = visitor;
        ctx.user_data = user_data;
        ctx.count = 0;

        visit_directory_songs(*target, &ctx);
        return ctx.count;
    } catch (...) {
        return -1;
    }
}

/* Page-based visit: skip + limit traversal of the song tree */
struct page_visit_context {
    mpd_song_visitor_fn visitor;
    void *user_data;
    int skip;       /* remaining songs to skip */
    int remaining;  /* remaining songs to visit */
    int visited;    /* songs visited so far */
    mpd_song_info_t info;
    std::string uri_buf;
};

static bool visit_page_directory(const Directory &dir, struct page_visit_context *ctx)
{
    for (const auto &song : dir.songs) {
        if (ctx->remaining <= 0) return false; /* done */

        if (ctx->skip > 0) {
            ctx->skip--;
            continue;
        }

        ctx->uri_buf = song.GetURI();
        memset(&ctx->info, 0, sizeof(ctx->info));
        ctx->info.uri = ctx->uri_buf.c_str();
        ctx->info.mtime = std::chrono::system_clock::to_time_t(song.mtime);
        ctx->info.id3_parsed = song.id3_parsed;

        if (!song.tag.duration.IsNegative())
            ctx->info.duration_ms = song.tag.duration.ToMS();
        else
            ctx->info.duration_ms = -1;

        for (int i = 0; i < MPD_TAG_COUNT; i++) {
            TagType tt = bridge_tag_to_mpd((mpd_tag_type_t)i);
            if (tt < TAG_NUM_OF_ITEM_TYPES)
                ctx->info.tags[i] = song.tag.GetValue(tt);
        }

        ctx->visited++;
        ctx->remaining--;
        if (ctx->visitor(&ctx->info, ctx->user_data) != 0)
            return false;
    }

    for (const auto &child : dir.children) {
        if (ctx->remaining <= 0) return false;
        if (!visit_page_directory(child, ctx))
            return false;
    }
    return true;
}

int mpd_db_visit_page(mpd_db_t *db, int page, int page_size,
                      mpd_song_visitor_fn visitor, void *user_data)
{
    if (!db || !visitor || page < 0 || page_size <= 0) return -1;

    try {
        const ScopeDatabaseLock protect;
        Directory &root = db->db->GetRoot();

        struct page_visit_context ctx;
        ctx.visitor = visitor;
        ctx.user_data = user_data;
        ctx.skip = page * page_size;
        ctx.remaining = page_size;
        ctx.visited = 0;

        visit_page_directory(root, &ctx);
        return ctx.visited;
    } catch (...) {
        return -1;
    }
}

int mpd_db_song_count(mpd_db_t *db)
{
    if (!db) return 0;

    struct visit_context ctx;
    ctx.count = 0;

    try {
        const ScopeDatabaseLock protect;
        Directory &root = db->db->GetRoot();

        /* Just count, don't call a visitor */
        std::function<void(const Directory &)> count_fn;
        count_fn = [&](const Directory &dir) {
            for ([[maybe_unused]] const auto &s : dir.songs)
                ctx.count++;
            for (const auto &child : dir.children)
                count_fn(child);
        };
        count_fn(root);
    } catch (...) {}

    return ctx.count;
}

time_t mpd_db_get_mtime(mpd_db_t *db)
{
    if (!db) return 0;
    auto tp = db->db->GetUpdateStamp();
    return std::chrono::system_clock::to_time_t(tp);
}

void mpd_db_clear(mpd_db_t *db)
{
    if (!db) return;
    try {
        const ScopeDatabaseLock protect;
        /* Delete and recreate root */
        db->db->Close();
        db->db->Open();
    } catch (...) {}
}

} /* extern "C" */
