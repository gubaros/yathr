Here is the **full English version** of your README, keeping structure, formatting, and technical accuracy intact:

---

# YATHR - Yet Another Tiny HTTP Router

A high-performance HTTP redirect server written in C with a modular architecture. The server uses platform-specific event notification mechanisms (epoll on Linux, kqueue on BSD/macOS) to efficiently handle thousands of concurrent connections.

## Table of Contents

* [Description](#description)
* [Architecture](#architecture)
* [Purpose](#purpose)
* [Setup](#setup)

  * [Dependencies](#dependencies)
  * [Installation](#installation)
* [Usage](#usage)

  * [Building the Server](#building-the-server)
  * [Running the Server](#running-the-server)
  * [Testing the Server](#testing-the-server)
  * [Stress Testing](#stress-testing)
* [License](#license)

## Description

YATHR is a lightweight, event-driven HTTP redirect server built for performance and maintainability. The codebase is organized into modular components, each with a clear and specific responsibility:

### Core Modules

* **`server.c`** – Main entry point and event loop orchestration
* **`http.c/h`** – HTTP request handling and response generation
* **`routing.c/h`** – URL redirect mapping and lookup
* **`platform.c/h`** – Platform-specific event handling (epoll/kqueue)
* **`utils/socket.c/h`** – Socket creation, configuration, and management
* **`utils/config.c/h`** – Configuration file parsing
* **`utils/logs.c/h`** – Logging infrastructure
* **`plugins/`** – Plugin system for pre/post-routing hooks

## Architecture

The server follows a modular design that cleanly separates responsibilities:

```
┌─────────────┐
│  server.c   │  ← Entry point, event loop
└──────┬──────┘
       │
   ┌───┴────────────────────┐
   │                        │
┌──▼──────┐          ┌──────▼───────┐
│platform │          │    http.c    │
│ (epoll/ │◄─────────┤  (handlers)  │
│ kqueue) │          └──────┬───────┘
└─────────┘                 │
                      ┌─────▼──────┐
                      │ routing.c  │
                      │(redirects) │
                      └────────────┘
```

### Benefits

* **Separation of Concerns** – Each module has a single, well-defined responsibility
* **Testability** – Components can be unit-tested independently
* **Maintainability** – Easy to locate and modify specific functionality
* **Reusability** – Modules can be reused across other projects
* **Scalability** – Easy to extend (e.g., HTTPS, CDB support, authentication)

## Purpose

This project provides a simple and efficient way to handle a large number of HTTP redirects. The key motivations include:

* **Simplicity**: Demonstrates how to build a basic but functional HTTP server capable of URL redirection using minimal dependencies and clear code.
* **Understanding HTTP**: Running and modifying the server helps users understand HTTP requests, responses, and redirection logic.
* **Performance**: The `tinycdb` library enables efficient handling of large URL datasets. CDB (Constant Database) offers fast lookups with a low memory footprint—ideal for scalable, high-performance applications.

## Approach to the Logic

### Event Loop Logic, Connection Handling, and Use of kqueue

The HTTP server is designed to efficiently manage a large number of concurrent connections. To achieve this, it relies on **kqueue**, an event notification interface available on BSD and macOS systems. kqueue makes it possible to monitor multiple file descriptors for events such as reads, writes, and errors in a scalable and efficient way.

### Main Loop Logic

The server creates a master socket that listens on a specified port (8080 by default). This socket is set to **non-blocking** mode to allow asynchronous I/O operations. A kqueue instance is created, and the master socket is registered for event monitoring.

The main loop performs the following tasks:

1. **Wait for Events**:
   The server calls `kevent` to wait for events on registered file descriptors. This call blocks until one or more events occur.

2. **Process Events**:
   When `kevent` returns events, the server processes them individually:

   * **New Connection**: If the event corresponds to the master socket, a new client connection is accepted. The client socket is set to non-blocking mode and added to kqueue for read monitoring.
   * **Data Available**: If the event corresponds to a client socket, the server reads the available data. If the client disconnects, the socket is closed; otherwise, the request is parsed and the appropriate response is generated.

### Connection Handling

Efficient connection handling is crucial for server performance. Using kqueue allows the server to manage thousands of concurrent connections without blocking, unlike thread-per-connection or process-per-connection models.

* **Non-Blocking I/O**: Both the master and client sockets use non-blocking mode, ensuring I/O operations never block the main loop.
* **Connection Reuse**: After finishing the communication with a client, the server closes the client socket and removes it from kqueue to free system resources.

### Use of kqueue

kqueue is ideal for applications that must handle many simultaneous connections due to its efficiency and scalability. Unlike `select` and `poll`, whose performance degrades with large descriptor sets, kqueue uses a notification-based model that remains efficient at scale.

**Advantages of kqueue:**

* **Scalability**: Efficiently handles thousands of descriptors.
* **Efficiency**: Reduces CPU usage and increases overall performance.
* **Flexibility**: Supports a wide range of events and filters.

In summary, combining kqueue with non-blocking I/O enables the server to handle high traffic volumes with low latency and high efficiency—essential traits for modern web services and high-performance infrastructure.

## Setup

### Dependencies

* `tinycdb` library

### Installation

On macOS:

```sh
brew install zlog
```

On Linux, install zlog from your package manager or build from source.

## Usage

### Building the Server

Compile the server using `make`:

```sh
make
```

This will produce the `http_server` executable.

### Configuration

Set the server's port in `config.txt`:

```
SERVER_PORT=8080
```

### Running the Server

Start the HTTP redirect server:

```sh
./http_server
```

### Testing the Server

Test redirections with curl:

```sh
curl -I http://localhost:8080/google
```

Expected output:

```
HTTP/1.1 302 Found
Location: https://www.google.com
Content-Length: 0
```

Or open the URL in a browser to verify the redirect.

### Stress Testing

The server is optimized for high performance. You can stress test using `wrk`:

```sh
# Install wrk
brew install wrk

# Light test - 100 concurrent connections
wrk -t4 -c100 -d30s --latency http://localhost:8080/google

# Medium test - 1000 concurrent connections
wrk -t8 -c1000 -d30s --latency http://localhost:8080/google

# Heavy test - 5000 concurrent connections
wrk -t10 -c5000 -d30s --latency http://localhost:8080/google
```

Benchmark results indicate that the server can handle **50,000+ requests per second** with **sub-5ms latency**.

### Epoll and Kqueue for Portability

#### Background

To support multiple operating systems, the server uses:

* **epoll on Linux** – a scalable I/O event notification mechanism
* **kqueue on BSD/macOS** – a similarly efficient event system for those platforms

#### Implementation Details

##### Epoll (Linux)

Key functions:

* `epoll_create1()` – creates an epoll instance
* `epoll_ctl()` – registers or modifies monitored descriptors
* `epoll_wait()` – waits for I/O events

##### Kqueue (BSD/macOS)

Key functions:

* `kqueue()` – creates an event queue
* `kevent()` – registers events and retrieves event notifications

#### Portability Layer

The server abstracts event handling behind platform-specific modules. At build time or runtime (depending on configuration), the correct implementation (epoll or kqueue) is selected.

## License

This project is licensed under the GNU General Public License v3.0. See the LICENSE file for full details.

---
