# AWTK Music Player Application

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    AWTK UI (music_ui.c)                      │
│  ┌──────────┬──────────┬──────────┬──────────┬─────────────┐ │
│  │ Title    │ Progress │ Controls │ Mode     │ Playlist    │ │
│  │ Artist   │ Slider   │ Prev/    │ Seq/Rep/ │ ListView    │ │
│  │ Album    │ Time     │ Play/Next│ Shuffle  │             │ │
│  └──────────┴──────────┴──────────┴──────────┴─────────────┘ │
│         ↕ music_app_xxx() function calls                     │
├─────────────────────────────────────────────────────────────┤
│              Application Controller (music_app.c)            │
│  ┌────────────┐  ┌────────────┐  ┌──────────────────────┐   │
│  │USB Monitor │  │   Scanner  │  │      Player           │   │
│  │(uevent NL) │→ │(dir+ID3v2) │→ │(libatcmediaplayer)   │   │
│  │usb_monitor │  │music_scanner│  │  music_player        │   │
│  └────────────┘  └────────────┘  └──────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                    Linux Kernel                               │
│  uevent netlink    /proc/mounts    ALSA audio                │
└─────────────────────────────────────────────────────────────┘
```

## File Locations

### New Files (AWTK app - this directory)

| File | Location | Description |
|------|----------|-------------|
| `main.c` | `awtk_app/src/main.c` | AWTK entry point |
| `music_app.h/c` | `awtk_app/src/music_app.[hc]` | Application controller |
| `music_ui.h/c` | `awtk_app/src/music_ui.[hc]` | AWTK widget layer |
| `usb_monitor.h/c` | `awtk_app/src/usb_monitor.[hc]` | USB hotplug monitor |
| `Makefile` | `awtk_app/Makefile` | GNU make build |
| `SConscript` | `awtk_app/SConscript` | SCons build (awtk-linux-fb) |

### Modified Files (existing musicplayer library)

| File | Location | Changes |
|------|----------|---------|
| `music_scanner.c` | `musicplayer/music_scanner.c` | 10 fixes (see diff) |
| `music_player.cpp` | `musicplayer/music_player.cpp` | 4 fixes (see diff) |

### Unmodified Reference Files

| File | Description |
|------|-------------|
| `music_scanner.h` | Scanner API header (no changes needed) |
| `music_player.h` | Player API header (no changes needed) |

## Build Instructions

### Cross-compile for AC8215 ARM target
```bash
cd source/packages/application/musicplayer/awtk_app
make
```

### Host build for development
```bash
cd source/packages/application/musicplayer/awtk_app
make HOST_BUILD=1
```

### SCons build (within awtk-linux-fb)
```bash
cd source/packages/cluster/awtk/awtk-linux-fb
scons APP=../../../packages/application/musicplayer/awtk_app
```

## Android Reference Mapping

| Android Class | C Module | Key Mapping |
|---------------|----------|-------------|
| `LocalService.java` | `music_app.c` | Service lifecycle, storage events |
| `MediaService.java` | `music_app.c` | Playback control, key events |
| `FileStorageState.java` | `usb_monitor.c` | Mount state queries |
| `MediaFilePathScan.java` | `music_scanner.c` | Directory scanning + filtering |
| `MusicProvider.java` | `music_scanner.h` (MusicList) | Data model |
| `MusicInfoLayout.java` | `music_ui.c` | Play info display |
| `MusicListLayout.java` | `music_ui.c` | Playlist display |
| `Preferences.java` | `music_app.c` (save/restore) | State persistence |
