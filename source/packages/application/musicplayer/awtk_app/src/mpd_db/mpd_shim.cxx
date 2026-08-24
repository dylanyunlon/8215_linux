/*
 * mpd_shim.cxx — Minimal implementations for MPD subsystems we don't fully port.
 * Provides: Log, IcuCollate, FileInfo, Path ops, ConfigBlock, Charset, OpenReadOnly, tag_pool
 */

#include "config.h"
#include "Log.hxx"
#include "LogLevel.hxx"
#include "fs/AllocatedPath.hxx"
#include "fs/Path.hxx"
#include "fs/Traits.hxx"
#include "fs/FileInfo.hxx"
#include "fs/Charset.hxx"
#include "config/Block.hxx"
#include "lib/icu/Collate.hxx"
#include "tag/Pool.hxx"
#include "tag/Item.hxx"
#include "tag/Mask.hxx"
#include "util/Domain.hxx"
#include "io/FileDescriptor.hxx"
/* AutoGunzipFileLineReader: without ENABLE_ZLIB, it's typedef'd to FileLineReader */
#include "io/BufferedReader.hxx"
#include "io/FileReader.hxx"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <string>
#include <mutex>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* ===== Log ===== */

void Log(LogLevel level, const Domain &domain, std::string_view msg) noexcept
{
    if (level >= LogLevel::INFO) {
        fprintf(stderr, "[mpd_db:%s] %.*s\n",
                domain.GetName(),
                (int)msg.size(), msg.data());
    }
}

void LogVFmt(LogLevel level, const Domain &domain,
             fmt::string_view format_str, fmt::format_args args) noexcept
{
    auto msg = fmt::vformat(format_str, args);
    Log(level, domain, msg);
}

/* LogError is inline in Log.hxx */

/* ===== IcuCollate ===== */

int IcuCollate(std::string_view a, std::string_view b) noexcept
{
    /* Simple fallback: case-insensitive compare */
    auto len = std::min(a.size(), b.size());
    for (size_t i = 0; i < len; i++) {
        int ca = tolower((unsigned char)a[i]);
        int cb = tolower((unsigned char)b[i]);
        if (ca != cb) return ca - cb;
    }
    return (int)a.size() - (int)b.size();
}

/* ===== FileInfo ===== */

FileInfo::FileInfo(Path path, bool follow)
{
    struct stat st;
    int ret = follow ? stat(path.c_str(), &st) : lstat(path.c_str(), &st);
    if (ret < 0)
        throw std::system_error(errno, std::system_category(),
                                std::string("Failed to stat ") + path.c_str());
    this->st = st;
}

/* ===== Path operations ===== */

AllocatedPath Path::GetDirectoryName() const noexcept
{
    const char *s = c_str();
    const char *slash = strrchr(s, '/');
    if (!slash) return AllocatedPath::FromFS(".");
    std::string dir(s, slash - s);
    return AllocatedPath::FromFS(dir.c_str());
}

AllocatedPath operator/(Path a, Path b) noexcept
{
    std::string result = a.c_str();
    result += '/';
    result += b.c_str();
    return AllocatedPath::FromFS(result.c_str());
}

AllocatedPath operator+(Path a, std::string_view b) noexcept
{
    std::string result = a.c_str();
    result.append(b);
    return AllocatedPath::FromFS(result.c_str());
}

/* ===== Charset (embedded = UTF-8 passthrough) ===== */

const char *GetFSCharset() noexcept
{
    return "UTF-8";
}

std::string PathToUTF8(std::string_view path)
{
    return std::string(path);
}

PathTraitsFS::string PathFromUTF8(PathTraitsUTF8::string_view path)
{
    return PathTraitsFS::string(path);
}

/* Path::ToUTF8 / ToUTF8Throw — provided by fs/Path.cxx */
/* tag_pool — provided by tag/Pool.cxx */
/* SongFilter — provided by song/Filter.cxx */

/* ===== ConfigBlock (we bypass config, construct SimpleDatabase directly) ===== */

const char *ConfigBlock::GetBlockValue(const char *name, const char *default_value) const noexcept
{
    (void)name;
    return default_value;
}

bool ConfigBlock::GetBlockValue(const char *name, bool default_value) const
{
    (void)name;
    return default_value;
}

AllocatedPath ConfigBlock::GetPath(const char *name, const char *default_value) const
{
    (void)name;
    if (default_value)
        return AllocatedPath::FromFS(default_value);
    return AllocatedPath(nullptr);
}

/* ===== OpenReadOnly ===== */
#include "io/Open.hxx"
#include "io/UniqueFileDescriptor.hxx"

UniqueFileDescriptor OpenReadOnly(const char *path, int flags)
{
    int fd = open(path, O_RDONLY | flags);
    if (fd < 0)
        throw std::system_error(errno, std::system_category(),
                                std::string("Failed to open ") + path);
    return UniqueFileDescriptor(AdoptTag{}, fd);
}

UniqueFileDescriptor OpenWriteOnly(const char *path, int flags)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | flags, 0666);
    if (fd < 0)
        throw std::system_error(errno, std::system_category(),
                                std::string("Failed to open for writing ") + path);
    return UniqueFileDescriptor(AdoptTag{}, fd);
}

/* Tag Pool: provided by tag/Pool.cxx */

/* AutoGunzipFileLineReader: without ENABLE_ZLIB it's typedef'd to FileLineReader (inline) */

/* ===== PlaylistDatabase stubs ===== */

#include "PlaylistDatabase.hxx"
#include "db/PlaylistVector.hxx"
#include "io/BufferedOutputStream.hxx"
#include "io/LineReader.hxx"

void playlist_vector_save(BufferedOutputStream &, const PlaylistVector &)
{
    /* No playlist support in embedded player */
}

void playlist_metadata_load(LineReader &, PlaylistVector &, const char *)
{
    /* No playlist support */
}

/* SongFilter: provided by song/Filter.cxx */

/* Song::Export: provided by db/plugins/simple/Song.cxx */

/* ===== pcm/AudioFormat ToString ===== */
#include "pcm/AudioFormat.hxx"
#include "util/StringBuffer.hxx"

StringBuffer<24> ToString(AudioFormat) noexcept
{
    StringBuffer<24> buf;
    strcpy(buf.data(), "unknown");
    return buf;
}

/* StripLeft is inline in util/StringStrip.hxx - no impl needed */

/* ===== Additional missing symbols ===== */

#include "tag/FixString.hxx"
#include "util/AllocatedArray.hxx"

AllocatedArray<char> FixTagString(std::string_view p) noexcept
{
    /* no-op: return null array = no fix needed */
    return nullptr;
}

/* FileInfo from FileDescriptor */
FileInfo::FileInfo(FileDescriptor fd)
{
    struct stat st;
    if (fstat(fd.Get(), &st) < 0)
        throw std::system_error(errno, std::system_category(), "fstat failed");
    this->st = st;
}

/* StripLeft(const char*) */
const char *StripLeft(const char *p) noexcept
{
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* ParseISO8601 stub */
#include <chrono>
std::chrono::system_clock::time_point ParseISO8601(const char *) {
    return std::chrono::system_clock::time_point::min();
}

/* Log(exception_ptr) overload */
void Log(LogLevel level, const std::exception_ptr &ep) noexcept {
    try { if (ep) std::rethrow_exception(ep); }
    catch (const std::exception &e) { fprintf(stderr, "[mpd_db] %s\n", e.what()); }
    catch (...) {}
}

/* ===== tag_name_parse(const char*) ===== */
#include "tag/ParseName.hxx"
TagType tag_name_parse(const char *name) noexcept {
    return tag_name_parse(std::string_view(name));
}

/* ===== URI utils ===== */
bool uri_safe_local(std::string_view) noexcept { return true; }
bool uri_is_child_or_same(const char *parent, const char *child) noexcept {
    size_t len = strlen(parent);
    return strncmp(parent, child, len) == 0 && (child[len] == 0 || child[len] == '/');
}

/* ===== IcuCompare stub ===== */
#include "lib/icu/Compare.hxx"
IcuCompare::IcuCompare(std::string_view s, bool fc, bool sd) noexcept
    : needle(AllocatedString(s)), fold_case(fc), strip_diacritics(sd) {}
bool IcuCompare::operator==(const char *other) const noexcept {
    return needle != nullptr && strcasecmp(needle.c_str(), other) == 0;
}
bool IcuCompare::IsIn(const char *haystack) const noexcept {
    return needle != nullptr && strcasestr(haystack, needle.c_str()) != nullptr;
}
bool IcuCompare::StartsWith(const char *other) const noexcept {
    return needle != nullptr && strncasecmp(other, needle.c_str(), strlen(needle.c_str())) == 0;
}

/* ===== EscapeFilterString ===== */
std::string EscapeFilterString(std::string_view s) noexcept {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '\'' || c == '"') result += '\\';
        result += c;
    }
    return result;
}

/* ===== OptimizeSongFilter ===== */
#include "song/AndSongFilter.hxx"
void OptimizeSongFilter(AndSongFilter &) noexcept {}

/* ===== SongFilter vtable stubs for filter types we don't use ===== */
#include "song/AudioFormatSongFilter.hxx"
#include "song/ModifiedSinceSongFilter.hxx"
#include "song/AddedSinceSongFilter.hxx"
#include "song/PrioritySongFilter.hxx"

bool AudioFormatSongFilter::Match(const LightSong &) const noexcept { return true; }
std::string AudioFormatSongFilter::ToExpression() const noexcept { return ""; }

bool ModifiedSinceSongFilter::Match(const LightSong &) const noexcept { return true; }
std::string ModifiedSinceSongFilter::ToExpression() const noexcept { return ""; }
bool AddedSinceSongFilter::Match(const LightSong &) const noexcept { return true; }
std::string AddedSinceSongFilter::ToExpression() const noexcept { return ""; }

bool PrioritySongFilter::Match(const LightSong &) const noexcept { return true; }
std::string PrioritySongFilter::ToExpression() const noexcept { return ""; }
