#pragma once

namespace GlobalScreenConfig {

    inline bool& getLargeScreenRef() {
        static bool isLargeScreen = false;
        return isLargeScreen;
    }

    inline void setLargeScreen(bool value) {
        getLargeScreenRef() = value;
    }

    inline bool getLargeScreen() {
        return getLargeScreenRef();
    }
}

