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

#ifndef ATC_KEY_HPP
#define ATC_KEY_HPP

namespace atcupdateservice {
namespace hash {

/* this template is used for generate key for types */
class KeyGenerator {
public:
    static unsigned generate() {
        static unsigned key = 0u;
        return ++key;
    }
};

/* Keys are used to identify types, each only get one key and it's thread safe */
template <class T>
class Key {
public:
    static unsigned getKey() {
        static unsigned key = KeyGenerator::generate();
        return key;
    }
};

}
}

#endif