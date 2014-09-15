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

#include "mongo/orchestration/server.h"

namespace mongo {
namespace orchestration {

    Server::Server(const string& url, Json::Value parameters) : Resource(url) {
    }

    void Server::start() {
        action("start");
    }

    void Server::stop() {
        action("stop");
    }

    void Server::restart() {
        action("restart");
    }

    void Server::destroy() {
        del();
    }

    RestClient::response Server::status() const {
        return get();
    }

    string Server::uri() const {
        return handle_response(status())["uri"].asString();
    }

} // namespace orchestration
} // namespace mongo
