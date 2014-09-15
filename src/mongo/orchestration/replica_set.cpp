/**
 * Copyright 2014 MongoDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mongo/orchestration/replica_set.h"

namespace mongo {
namespace orchestration {

    ReplicaSet::ReplicaSet(const string& url)
        : MongoResource(url)
    {}

    Server ReplicaSet::primary() const {
        Document doc = handle_response(get("primary"));
        string primary_uri = doc["uri"].asString();
        return Server(url().substr(0, url().find("/")) + primary_uri);
    }

    vector<Server> ReplicaSet::secondaries() const {
        return plural_subresource<Server>("secondaries");
    }

    vector<Server> ReplicaSet::hidden() const {
        return plural_subresource<Server>("hidden");
    }

    vector<Server> ReplicaSet::arbiters() const {
        return plural_subresource<Server>("arbiters");
    }

} // namespace orchestration
} // namespace mongo
