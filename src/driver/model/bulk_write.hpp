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

#pragma once

#include "driver/config/prelude.hpp"

#include <vector>

#include "driver/options/bulk_write.hpp"
#include "driver/base/write_concern.hpp"
#include "driver/model/write.hpp"
#include "driver/util/optional.hpp"

namespace mongo {
namespace driver {
class collection;

namespace model {

class LIBMONGOCXX_EXPORT bulk_write {

    friend class mongo::driver::collection;

    class impl;

   public:
    bulk_write(const options::bulk_write& options);

    bulk_write& append(write operation);

    bulk_write(bulk_write&&);
    bulk_write& operator=(bulk_write&&);
    ~bulk_write();

   private:
    std::unique_ptr<impl> _impl;
};

}  // namespace model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
