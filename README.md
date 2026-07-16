# Redis Server (C++)

A lightweight Redis-inspired in-memory key-value database built from scratch in modern C++. The project implements the Redis Serialization Protocol (RESP), supports multiple native data structures, concurrent client connections, key expiration, and persistence while remaining compatible with the standard `redis-cli`.

The objective of this project was not to recreate Redis completely, but to understand the systems programming concepts involved in building an in-memory database from scratch, including socket programming, protocol implementation, synchronization, persistence, and concurrent server design.

---

## Features

### Networking

* TCP server built using POSIX sockets
* Compatible with the standard `redis-cli`
* Supports multiple concurrent client connections using worker threads
* Graceful shutdown through signal handling (`Ctrl + C`)

---

### RESP Protocol

Implements the Redis Serialization Protocol (RESP2) parser from scratch.

Supports both:

* RESP formatted requests
* Inline commands

For example,

```text
*2
$4
PING
$5
Hello
```

is parsed internally as

```text
PING Hello
```

---

### Supported Data Types

#### Strings

* SET
* GET

#### Lists

* LPUSH
* RPUSH
* LPOP
* RPOP
* LGET
* LLEN
* LINDEX
* LSET
* LREM

#### Hashes

* HSET
* HGET
* HDEL
* HEXISTS
* HGETALL
* HKEYS
* HVALS
* HLEN
* HMSET

---

### Database Commands

* KEYS
* TYPE
* DEL
* FLUSHALL
* RENAME

---

### Key Expiration

Supports lazy expiration using TTL metadata.

```text
SET session abc123
EXPIRE session 60
```

Expired keys are removed transparently before command execution.

---

### Persistence

Implements a lightweight text-based persistence mechanism.

The database is:

* Restored automatically during startup
* Periodically written to disk every five minutes
* Persisted again during graceful shutdown

The dump file stores three record types:

```text
K key value
L key value1 value2 value3 ...
H key field:value field:value ...
```

---

### Concurrency

Each accepted client connection is processed in a dedicated worker thread while the main server thread continues accepting new incoming connections.

Shared database access is synchronized using mutexes to ensure thread safety.

---

# Project Structure

```text
.
├── include
│   ├── RedisCommandHandler.h
│   ├── RedisDatabase.h
│   └── RedisServer.h
│
├── src
│   ├── RedisCommandHandler.cpp
│   ├── RedisDatabase.cpp
│   ├── RedisServer.cpp
│   └── main.cpp
│
├── Makefile
├── README.md
└── dump.my_rdb
```

---

# Building

Compile the project using:

```bash
make
```

---

# Running

Start the server on the default Redis port (`6379`):

```bash
./bin/my_redis_server
```

Or specify a custom port:

```bash
./bin/my_redis_server 6380
```

Connect using the standard Redis CLI:

```bash
redis-cli -p 6379
```

---

# Example Session

```text
127.0.0.1:6379> SET name Alice
OK

127.0.0.1:6379> GET name
"Alice"

127.0.0.1:6379> LPUSH numbers 1 2 3
(integer) 3

127.0.0.1:6379> LGET numbers
1) "3"
2) "2"
3) "1"

127.0.0.1:6379> HSET user name Alice
(integer) 1

127.0.0.1:6379> HGET user name
"Alice"

127.0.0.1:6379> EXPIRE name 60
OK
```

---

# Architecture

The project is organized into three major components.

## RedisServer

Responsible for:

* Socket creation and initialization
* Accepting incoming client connections
* Managing worker threads
* Graceful shutdown and persistence lifecycle

---

## RedisCommandHandler

Responsible for:

* Parsing RESP requests
* Dispatching commands
* Formatting RESP responses

Acts as the bridge between client requests and database operations.

---

## RedisDatabase

Implements the core in-memory database.

Responsible for:

* String storage
* List storage
* Hash storage
* Key expiration
* Persistence
* Thread-safe access using mutexes

Implemented as a singleton to provide a shared database instance across all worker threads.

---

# Concepts Explored

* POSIX Socket Programming
* TCP Networking
* Concurrent Server Design
* Multithreading
* Mutex Synchronization
* Redis Serialization Protocol (RESP)
* In-Memory Database Design
* Lazy Expiration
* File-Based Persistence
* Signal Handling