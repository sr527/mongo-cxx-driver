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

#include "driver/model/update_many.hpp"

namespace mongo {
namespace driver {
namespace model {

update_many::update_many(bson::document::view criteria, bson::document::view update)
    : _criteria(std::move(criteria)), _update(std::move(update)) {}

update_many& update_many::upsert(bool upsert) {
    _upsert = upsert;
    return *this;
}

optional<bool> update_many::upsert() const { return _upsert; }

bson::document::view update_many::criteria() const { return _criteria; }

bson::document::view update_many::update() const { return _update; }

}  // namespace model
}  // namespace driver
}  // namespace mongo
