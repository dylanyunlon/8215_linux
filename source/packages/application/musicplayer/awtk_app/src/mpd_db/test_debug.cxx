#include "config.h"
#include "db/plugins/simple/SimpleDatabasePlugin.hxx"
#include "db/plugins/simple/Directory.hxx"
#include "db/plugins/simple/Song.hxx"
#include "db/DatabaseLock.hxx"
#include "tag/Tag.hxx"
#include "tag/Builder.hxx"
#include "fs/AllocatedPath.hxx"
#include <cstdio>
#include <memory>

int main() {
    auto path = AllocatedPath::FromUTF8("/tmp/test_debug.db");
    printf("Path: %s\n", path.c_str());

    SimpleDatabase db(std::move(path), false, true);
    printf("Created SimpleDatabase\n");

    db.Open();
    printf("Opened\n");

    {
        const ScopeDatabaseLock protect;
        Directory &root = db.GetRoot();
        printf("Root: '%s', IsRoot=%d\n", root.GetPath(), root.IsRoot());

        // Add a song directly to root
        TagBuilder tb;
        tb.AddItem(TAG_TITLE, "Test Song");
        auto song = std::make_unique<Song>(std::string("test.mp3"), root);
        song->tag = tb.Commit();
        root.AddSong(std::move(song));
        printf("Added song to root\n");

        // Count
        int count = 0;
        for ([[maybe_unused]] const auto &s : root.songs) count++;
        printf("Root songs: %d\n", count);

        // Find it back
        const Song *found = root.FindSong("test.mp3");
        printf("FindSong: %p\n", (void*)found);
        if (found) {
            printf("  filename: %s\n", found->filename.c_str());
            const char *title = found->tag.GetValue(TAG_TITLE);
            printf("  title: %s\n", title ? title : "(null)");
        }

        // Create subdirectory and add song
        Directory *sub = root.MakeChild("USB1");
        Directory *sub2 = sub->MakeChild("Rock");
        printf("Created USB1/Rock\n");

        auto song2 = std::make_unique<Song>(std::string("track01.mp3"), *sub2);
        TagBuilder tb2;
        tb2.AddItem(TAG_TITLE, "Rock Song");
        song2->tag = tb2.Commit();
        sub2->AddSong(std::move(song2));
        printf("Added song to USB1/Rock\n");

        // Count all songs recursively
        int total = 0;
        std::function<void(const Directory&)> counter;
        counter = [&](const Directory &d) {
            for ([[maybe_unused]] const auto &s : d.songs) total++;
            for (const auto &c : d.children) counter(c);
        };
        counter(root);
        printf("Total songs: %d\n", total);
    }

    // Save
    db.Save();
    printf("Saved\n");

    db.Close();
    printf("Closed\n");

    // Reopen and verify
    auto path2 = AllocatedPath::FromUTF8("/tmp/test_debug.db");
    SimpleDatabase db2(std::move(path2), false, true);
    db2.Open();
    printf("Reopened\n");

    {
        const ScopeDatabaseLock protect;
        Directory &root = db2.GetRoot();
        int total = 0;
        std::function<void(const Directory&)> counter;
        counter = [&](const Directory &d) {
            for ([[maybe_unused]] const auto &s : d.songs) total++;
            for (const auto &c : d.children) counter(c);
        };
        counter(root);
        printf("After reload, total songs: %d\n", total);
    }

    db2.Close();
    unlink("/tmp/test_debug.db");
    printf("ALL OK\n");
    return 0;
}
