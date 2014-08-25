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

#include <cassert>

#include "orchestration.h"

#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/reader.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

namespace mongo {
namespace orchestration {

    namespace Status {
        const int OK = 200;
        const int NoContent = 204;
        const int Fail = 400;
        const int NotFound = 404;
    }

    const char Resource::_content_type[] = "text/json";

    Resource::Resource(string _url) : _url(_url) {}

    RestClient::response Resource::get(string relative_path) const {
        return RestClient::get(make_url(relative_path));
    }

    RestClient::response Resource::put(string relative_path, string payload) {
        return RestClient::put(make_url(relative_path), _content_type, payload);
    }

    RestClient::response Resource::post(string relative_path, string payload) {
        return RestClient::post(make_url(relative_path), _content_type, payload);
    }

    RestClient::response Resource::del(string relative_path) {
        return RestClient::del(make_url(relative_path));
    }

    string Resource::make_url(string relative_path) const {
        return relative_path.empty() ? _url : _url + "/" + relative_path;
    }

    auto_ptr<Document> Resource::handle_response(RestClient::response response) const {
        auto_ptr<Document> doc_ptr(new Document);
        if (response.code == Status::OK) {
            doc_ptr->Parse(response.body.c_str());
        } else if (response.code != Status::NoContent) {
            throw response.body.c_str();
        }
        return doc_ptr;
    }

    API::API(string url) : Resource(url) {}

    vector<Host> API::hosts() const {
        return get_plural_resource<Host>("hosts");
    }

    vector<ReplicaSet> API::replica_sets() const {
        return get_plural_resource<ReplicaSet>("rs");
    }

    vector<Cluster> API::clusters() const {
        return get_plural_resource<Cluster>("sh");
    }

    Host API::host(const string& id) const {
        return Host(_url + "/hosts/" + id);
    }

    ReplicaSet API::replica_set(const string& id) const {
        return ReplicaSet(_url + "/rs/" + id);
    }

    string API::createMongod(const string& id) {
        Document doc;
        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        writer.StartObject();

        // Host ID
        if (!id.empty()) {
            writer.String("id");
            writer.String(id.c_str());
        }

        // Process Name
        writer.String("name");
        writer.String("mongod");

        // Process Parameters
        writer.String("procParams");
        writer.StartObject();
        writer.String("setParameter");
        writer.String("enableTestCommands=1");
        writer.EndObject();

        writer.EndObject();

        RestClient::response result = post("hosts", sb.GetString());
        Document result_doc;
        result_doc.Parse(result.body.c_str());
        return result_doc["id"].GetString();
    }

    Host::Host(const string& url) : Resource(url) {}

    void Host::start() {
        put("start");
    }

    void Host::stop() {
        put("stop");
    }

    void Host::restart() {
        put("restart");
    }

    void Host::destroy() {
        del();
    }

    string Host::uri() const {
        Document doc;
        doc.Parse(status().body.c_str());
        assert(doc.IsObject());
        assert(doc.HasMember("uri"));
        return doc["uri"].GetString();
    }

    ReplicaSet::ReplicaSet(const string& url) : Resource(url) {}

    RestClient::response Host::status() const {
        return get();
    }

    Cluster::Cluster(const string& url) : Resource(url) {}

} // namespace orchestration
} // namespace mongo