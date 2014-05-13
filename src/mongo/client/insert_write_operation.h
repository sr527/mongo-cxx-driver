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

#pragma once

#include "mongo/bson/bsonobj.h"
#include "mongo/client/write_operation.h"
#include "mongo/util/net/message.h"

namespace mongo {

    class InsertWriteOperation : public WriteOperation {
    public:
        InsertWriteOperation(const BSONObj doc, int flags);

        virtual Operations operationType() const;

        virtual void startRequest(const std::string& ns, BufBuilder* builder) const;
        virtual bool appendSelfToRequest(int maxSize, BufBuilder* builder) const;

        virtual void startCommand(const std::string& ns, BSONObjBuilder* command) const;
        virtual bool appendSelfToCommand(BSONArrayBuilder* batch) const;
        virtual void endCommand(BSONArrayBuilder* batch, BSONObjBuilder* command) const;

    private:
        const BSONObj _doc;
        int _flags;
    };

} // namespace mongo