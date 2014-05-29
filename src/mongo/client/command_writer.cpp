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

#include "mongo/client/command_writer.h"

#include "mongo/client/dbclientinterface.h"
#include "mongo/client/write_result.h"
#include "mongo/db/namespace_string.h"

namespace mongo {

    const int kOverhead = 8 * 1024;
    const char kOrderedKey[] = "ordered";

    CommandWriter::CommandWriter(DBClientBase* client) : _client(client) {
    }

    void CommandWriter::write(
        const StringData& ns,
        const std::vector<WriteOperation*>& write_operations,
        bool ordered,
        const WriteConcern* wc,
        WriteResult*
    ) {
        std::vector<WriteOperation*>::const_iterator batch_begin = write_operations.begin();
        const std::vector<WriteOperation*>::const_iterator end = write_operations.end();

        while (batch_begin != end) {

            boost::scoped_ptr<BSONObjBuilder> command(new BSONObjBuilder);
            boost::scoped_ptr<BSONArrayBuilder> batch(new BSONArrayBuilder);
            std::vector<WriteOperation*>::const_iterator batch_iter = batch_begin;

            // We must be able to fit the first item of the batch. Otherwise, the calling code
            // passed an over size write operation in violation of our contract.
            invariant(_fits(batch.get(), *batch_iter));

            // Begin the command for this batch.
            (*batch_iter)->startCommand(ns.toString(), command.get());

            while (true) {

                // Always safe to append here: either we just entered the loop, or all the
                // checks below passed.
                (*batch_iter)->appendSelfToCommand(batch.get());

                // Peek at the next operation.
                const std::vector<WriteOperation*>::const_iterator next = boost::next(batch_iter);

                // If we are out of operations, issue what we have.
                if (next == end)
                    break;

                // If the next operation is of a different type, issue what we have.
                if ((*next)->operationType() != (*batch_iter)->operationType())
                    break;

                // If adding the next op would put us over the limit of ops in a batch, issue
                // what we have.
                if (std::distance(batch_begin, next) >= _client->getMaxWriteBatchSize())
                    break;

                // If we can't put the next item into the current batch, issue what we have.
                if (!_fits(batch.get(), *next))
                    break;

                // OK to proceed to next op.
                batch_iter = next;
            }

            // End the command for this batch.
            _endCommand(batch.get(), *batch_iter, ordered, command.get());

            // Issue the complete command.
            results->push_back(_send(command.get(), wc, ns));

            // The next batch begins with the op after the last one in the just issued batch.
            batch_begin = ++batch_iter;
        }
    }

    bool CommandWriter::_fits(BSONArrayBuilder* builder, WriteOperation* op) {
        int opSize = op->incrementalSize();
        int maxSize = _client->getMaxBsonObjectSize();

        // This update is too large to ever be sent as a command, assert
        uassert(0, "update command exceeds maxBsonObjectSize", opSize <= maxSize);

        return (builder->len() + opSize + kOverhead) <= maxSize;
    }

    void CommandWriter::_endCommand(
        BSONArrayBuilder* batch,
        WriteOperation* op,
        bool ordered,
        BSONObjBuilder* command
    ) {
        command->append(op->batchName(), batch->arr());
        command->append(kOrderedKey, ordered);
    }

    BSONObj CommandWriter::_send(BSONObjBuilder* command, const WriteConcern* wc, const StringData& ns) {
        command->append("writeConcern", wc->obj());

        BSONObj result;
        bool commandWorked = _client->runCommand(nsToDatabase(ns), command->obj(), result);

        if (!commandWorked || result.hasField("writeErrors")) {
            throw OperationException(result);
        }

        return result;
    }

} // namespace mongo
