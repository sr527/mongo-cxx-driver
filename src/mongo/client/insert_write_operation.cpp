/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mongo/client/insert_write_operation.h"

#include "mongo/client/dbclientinterface.h"
#include "mongo/db/namespace_string.h"


namespace mongo {

    namespace {
        const char kCommandKey[] = "insert";
        const char kBatchKey[] = "documents";
        const char kOrderedKey[] = "ordered";
    } // namespace

    InsertWriteOperation::InsertWriteOperation(const BSONObj doc, int flags)
        : _doc(doc)
        , _flags(flags)
        {}

    Operations InsertWriteOperation::operationType() const {
        return dbInsert;
    }

    void InsertWriteOperation::startRequest(const std::string& ns, BufBuilder* builder) const {
        builder->appendNum(_flags);
        builder->appendStr(ns);
    }

    bool InsertWriteOperation::appendSelfToRequest(int maxSize, BufBuilder* builder) const {
        if (builder->getSize() + _doc.objsize() > maxSize)
            return false;

        _doc.appendSelfToBufBuilder(*builder);
        return true;
    }

    void InsertWriteOperation::startCommand(const std::string& ns, BSONObjBuilder* builder) const {
        builder->append(kCommandKey, nsToCollectionSubstring(ns));
    }

    bool InsertWriteOperation::appendSelfToCommand(BSONArrayBuilder* builder) const {
        builder->append(_doc);
        return true;
    }

    void InsertWriteOperation::endCommand(BSONArrayBuilder* batch, BSONObjBuilder* builder) const {
        builder->append(kBatchKey, batch->arr());
        builder->append(kOrderedKey, bool(!(_flags & InsertOption_ContinueOnError)));
    }

} // namespace mongo
