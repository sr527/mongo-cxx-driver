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

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "driver/libmongoc.hpp"

#include "driver/base/client.hpp"
#include "driver/base/database.hpp"

using namespace mongo::driver;

TEST_CASE("A database", "[database][base]") {
    const std::string database_name("database");
    auto client_new = libmongoc::client_new_from_uri.create_instance();
    client_new->interpose([](const mongoc_uri_t*) {
        return nullptr;
    }).forever();
    auto get_database = libmongoc::client_get_database.create_instance();
    get_database->interpose([&](mongoc_client_t* client, const char* c_name) {
        return nullptr;
    });
;
    auto client_set_preference = libmongoc::client_set_read_prefs.create_instance();
    client_set_preference->interpose([](mongoc_client_t*,
                                        const mongoc_read_prefs_t*) {
        return;
    }).forever();
    auto client_set_concern = libmongoc::client_set_write_concern.create_instance();
    client_set_concern->interpose([](mongoc_client_t*,
                                     const mongoc_write_concern_t*) {
        return;
    }).forever();
    auto database_set_preference = libmongoc::database_set_read_prefs.create_instance();
    database_set_preference->interpose([](mongoc_database_t*,
                                        const mongoc_read_prefs_t*) {
        return;
    }).forever();
    auto database_set_concern = libmongoc::database_set_write_concern.create_instance();
    database_set_concern->interpose([](mongoc_database_t*,
                                     const mongoc_write_concern_t*) {
        return;
    }).forever();
    auto database_destroy = libmongoc::database_destroy.create_instance();
    database_destroy->interpose([](mongoc_database_t* client) {
        return;
    }).forever();
    client mongo_client;

    SECTION("is created by a client") {
        bool called = false;
        get_database->interpose([&](mongoc_client_t* client, const char* c_name) {
            called = true;
            REQUIRE(database_name == c_name);
            return nullptr;
        });

        database obtained_database = mongo_client[database_name];
        REQUIRE(called);
        REQUIRE(obtained_database.name() == database_name);
    }
    SECTION("cleans up its underlying mongoc database on destruction") {
        bool destroy_called = false;
        database_destroy->interpose([&](mongoc_database_t* client) {
            destroy_called = true;
            return nullptr;
        });

        {
            database database = mongo_client["database"];
            REQUIRE(!destroy_called);
        }

        REQUIRE(destroy_called);
    }

    SECTION("supports move operations") {
        bool destroy_called = false;
        database_destroy->interpose([&](mongoc_database_t* client) {
            destroy_called = true;
            return nullptr;
        });

        {
            client mongo_client;
            database a = mongo_client[database_name];

            database b{std::move(a)};
            REQUIRE(!destroy_called);

            database c = std::move(b);
            REQUIRE(!destroy_called);
        }
        REQUIRE(destroy_called);
    }

    SECTION("has a read preferences which may be set and obtained") {
        bool destroy_called = false;
        database_destroy->interpose([&](mongoc_database_t* client) {
            destroy_called = true;
            return nullptr;
        });

        database mongo_database(mongo_client["database"]);
        read_preference preference{read_mode::k_secondary_preferred};

        bool called = false;
        database_set_preference->interpose([&](mongoc_database_t* db,
                            const mongoc_read_prefs_t* read_prefs) {
                called = true;
                REQUIRE(mongoc_read_prefs_get_mode(read_prefs) ==
                        static_cast<mongoc_read_mode_t>(read_mode::k_secondary_preferred));
        });
        mongo_database.read_preference(preference);
        REQUIRE(called);

        REQUIRE(preference.mode() == mongo_database.read_preference().mode());
    }

    SECTION("has a write concern which may be set and obtained") {
        bool destroy_called = false;
        database_destroy->interpose([&](mongoc_database_t* client) {
            destroy_called = true;
            return nullptr;
        });

        database mongo_database(mongo_client[database_name]);
        write_concern concern{};
        concern.confirm_from(majority);

        bool called = false;
        database_set_concern->interpose([&](mongoc_database_t* db,
                             const mongoc_write_concern_t* concern) {
                called = true;
                REQUIRE(mongoc_write_concern_get_wmajority(concern));
        });
        mongo_database.write_concern(concern);
        REQUIRE(called);

        REQUIRE(concern.confirm_from().majority() ==
                mongo_database.write_concern().confirm_from().majority());
    }


    SECTION("may create a collection") {
        auto database_get_collection = libmongoc::database_get_collection.create_instance();
        database_get_collection->interpose([](mongoc_database_t* client, const char* name) {
            return nullptr;
        }).forever();
        auto collection_set_preference = libmongoc::collection_set_read_prefs.create_instance();
        collection_set_preference->interpose([](mongoc_collection_t*,
                                            const mongoc_read_prefs_t*) {
            return;
        }).forever();
        auto collection_set_concern = libmongoc::collection_set_write_concern.create_instance();
        collection_set_concern->interpose([](mongoc_collection_t*,
                                         const mongoc_write_concern_t*) {
            return;
        }).forever();
        auto collection_destroy = libmongoc::collection_destroy.create_instance();
        collection_destroy->interpose([](mongoc_collection_t* client) {
            return;
        });

        const std::string collection_name("collection");
        database database = mongo_client[database_name];
        collection obtained_collection = database[collection_name];
        REQUIRE(obtained_collection.name() == collection_name);
    }
}
