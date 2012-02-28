/*
 * ConnectionManager.h
 *
 *  Created on: Feb 24, 2012
 *      Author: corneliu
 */

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include <Connection.h>
#include <list>
#include <memory>

#include <ReadBuffer.h>
#include <WriteBuffer.h>
#include <RequestProcessor.h>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sys = boost::system;

inline ReadBuffer<> createReadBuffer() {
  return ReadBuffer<>();
}

inline WriteBuffer<> createWriteBuffer() {
  return WriteBuffer<>();
}

// manages a pool of connection objects
class ConnectionManager {
public:
  typedef RequestProcessor<ReadBuffer<>, WriteBuffer<>> DefaultRequestProcessor;
  typedef Connection<DefaultRequestProcessor> DefaultConnection;
  typedef std::shared_ptr<DefaultConnection> ConnectionPtr;

  // max connections specifies the total number of connection objects
  // which are allowed to be created
  ConnectionManager(asio::io_service& loop, size_t max_connections = 2)
    : _loop(loop), _max_connections(max_connections), _next_id(0)
  {
    // fill the free list with null connection pointers initially
    for (size_t i = 0; i < max_connections; ++i)
      _freelist.push_back(nullptr);
  }

  // free connection objects which have been released
  // and return them to the free list
  bool free() {
    // store initial active connection size
    size_t before = _connections.size();

    // predicate for removing connections from
    // the active list if they are not in use
    auto pred = [&](const ConnectionPtr& elem) {
        if (elem->in_use())
          return false;
        _freelist.push_back(elem);
        return true;
    };
    _connections.remove_if(pred);

    // if the size has changed, an active connection
    // has been freed and returned to the pool
    return (before != _connections.size());
  }

  // allocates a new or reinitialized connection object
  // if we've reached the maximum allocated connections,
  // attempt to free released connections in the active list
  ConnectionPtr allocate() {
    // if we have connections in the free list
    if (_freelist.size() > 0) {
      // if the connection object is uninitialized, allocate it
      // otherwise reinitialize with the incremented id value
      auto connPtr = _freelist.front();
      if (!connPtr)
        connPtr = ConnectionPtr(new DefaultConnection(_loop, ++_next_id));
      else
        connPtr->reinitialize(++_next_id);

      // remove front of free list, add to active list, and return
      _freelist.pop_front();
      _connections.emplace_back(connPtr);
      return connPtr;
    }

    // attempt to free a connection and return it, otherwise return null
    if (free())
      return allocate();

    return nullptr;
  }

private:
  asio::io_service& _loop;
  std::list<ConnectionPtr> _connections;
  std::list<ConnectionPtr> _freelist;
  size_t _max_connections;
  size_t _next_id;
};


#endif /* CONNECTIONMANAGER_H_ */
