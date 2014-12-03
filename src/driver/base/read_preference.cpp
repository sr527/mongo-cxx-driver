// Copyright 2014 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "driver/base/read_preference.hpp"
#include "driver/base/private/read_preference.hpp"

#include "driver/util/libbson.hpp"

#include "mongoc.h"

namespace mongo {
namespace driver {

read_preference::read_preference(impl implementation) {
}

read_preference::read_preference(read_mode mode)
    : _impl(
        std::make_unique<impl>(
            mongoc_read_prefs_new(static_cast<mongoc_read_mode_t>(mode))
        )
    )
{}

read_preference::read_preference(read_mode mode, bson::document::view tags)
    : read_preference(mode)
{
    read_preference::tags(tags);
}

void read_preference::mode(read_mode mode) {
    mongoc_read_prefs_set_mode(_impl->read_preference_t, static_cast<mongoc_read_mode_t>(mode));
}

void read_preference::tags(bson::document::view tags) {
    bson::libbson::scoped_bson_t scoped_bson_tags(tags);
    mongoc_read_prefs_set_tags(_impl->read_preference_t, scoped_bson_tags.bson());
}

read_mode read_preference::mode() const {
    return static_cast<read_mode>(mongoc_read_prefs_get_mode(_impl->read_preference_t));
}

optional<bson::document::view> read_preference::tags() const {
    const bson_t* bson_tags = mongoc_read_prefs_get_tags(_impl->read_preference_t);

    if (bson_count_keys(bson_tags))
        return bson::document::view(bson_get_data(bson_tags), bson_tags->len);

    return optional<bson::document::view>{};
}

}  // namespace driver
}  // namespace mongo
