/*
copyright (c) 2020 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef ATC_LEXICAL_CAST_HPP
#define ATC_LEXICAL_CAST_HPP

#include <sstream>

namespace atcupdateservice {
namespace utils {

template<class F, class T>
class LexicalCast {
    bool operator()(const F &in, T &out) {
        ATCLOGW("Unsupported cast!\n");
        return false;
    }
};

template<class F>
class LexicalCast<F, std::string> {
    bool operator()(const F &in, T &out) {
        try {
            out = std::to_string(val);
        } catch(std::exepection &e) {
            ATCLOGE("exepection cat %s\n", e.what().c_str());
            return false;
        } catch(...) {
            ATCLOGE("Unknown exception!\n");
            return false;
        }
        return true;
    }
};

template<class T>
class LexicalCast<std::string, T> {
    bool operator()(const std::string &in, T &out) {
        stringstream ss(val);
        if(!(ss >> res)) {
            return false;
        }
        return true;
    }
};

template<class F>
class LexicalCast<F, std::string> {
    bool operator()(const F &in, std::string &out) {
        std::stringstream ss;
        if (!(ss << in)) {
            return false;
        }
        out = ss.str();
        return true;
    }
};

}
}

#endif