# LVGL Bluetooth Module

This module is the active Bluetooth implementation for `lvgl_home`.

Legacy Qt Bluetooth files under the top-level application directories are kept only as migration references and must not be added to the `lvgl_home` build.

## Legacy Mapping

- Application lifecycle and profile ownership -> `core/` and `platform/`
- GAP callback handling -> `profile/bt_gap.c`
- HFP callback handling -> `profile/bt_hfp.c`
- PBAP callback handling -> `profile/bt_pbap.c`
- AVRCP callback handling -> `profile/bt_avrcp.c`
- A2DP callback handling -> `profile/bt_a2dp.c`
- HID callback handling -> not yet implemented
- File sync and utility behavior -> `core/` helpers as needed

## SDK Boundary

`platform/` is the only place that may call the vendor Bluetooth SDK.
`bt_platform_stub.c` is a pure C placeholder so the LVGL app can build and the UI can be exercised without the vendor SDK.
`bt_platform_sdk.cpp` wraps the NOR Bluetooth C++ SDK behind the same C platform API.

Enable the SDK backend with CMake options:

```sh
cmake -S lvgl_home -B build-lvgl-home \
  -DLVGL_BT_USE_SDK=ON \
  -DLVGL_BT_SDK_INCLUDE_DIR=/path/to/bluetooth/include \
  -DLVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR=/path/to/universal_utils/include \
  -DLVGL_BT_LINK_SDK_LIBS=ON
```
