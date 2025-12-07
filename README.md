# YATHR - Yet Another Tiny HTTP Router

A high-performance HTTP redirect server written in C with a modular architecture. The server uses platform-specific event notification mechanisms (epoll on Linux, kqueue on BSD/macOS) to efficiently handle thousands of concurrent connections.

## Table of Contents

- [Description](#description)
- [Architecture](#architecture)
- [Purpose](#purpose)
- [Setup](#setup)
  - [Dependencies](#dependencies)
  - [Installation](#installation)
- [Usage](#usage)
  - [Building the Server](#building-the-server)
  - [Running the Server](#running-the-server)
  - [Testing the Server](#testing-the-server)
  - [Stress Testing](#stress-testing)
- [License](#license)

## Description

YATHR is a lightweight, event-driven HTTP redirect server built for performance and maintainability. The codebase is organized into modular components, each with a specific responsibility:

### Core Modules

- **`server.c`** - Main entry point and event loop orchestration
- **`http.c/h`** - HTTP request handling and response generation
- **`routing.c/h`** - URL redirect mapping and lookup
- **`platform.c/h`** - Platform-specific event handling (epoll/kqueue)
- **`utils/socket.c/h`** - Socket creation, configuration, and management
- **`utils/config.c/h`** - Configuration file parsing
- **`utils/logs.c/h`** - Logging infrastructure
- **`plugins/`** - Plugin system for pre/post-routing hooks

## Architecture

The server follows a modular design that separates concerns:

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

✅ **Separation of Concerns** - Each module has a single, well-defined responsibility
✅ **Testability** - Components can be unit tested independently
✅ **Maintainability** - Easy to locate and modify specific functionality
✅ **Reusability** - Modules can be used in other projects
✅ **Scalability** - Easy to extend (e.g., add HTTPS, CDB support, authentication)

## Purpose

The purpose of this project is to provide a simple and efficient way to handle a large number of HTTP URL redirects. The key motivations behind this project include:

- **Simplicity**: The project aims to demonstrate how to build a basic yet functional HTTP server that performs URL redirections using minimal dependencies and straightforward code.
- **Understanding HTTP**: By building and running this server, users can gain a deeper understanding of how HTTP requests and responses work, and how to implement redirection logic.
- **Performance**: The use of the `tinycdb` library for managing URL mappings allows the server to handle large datasets efficiently. CDB (Constant Database) provides fast lookups with a minimal memory footprint, making it ideal for applications requiring high performance and scalability.

## Aproximación a la lógica

Lógica del Bucle, Manejo de Conexiones y Uso de kqueue

El servidor HTTP está diseñado para manejar un alto número de conexiones concurrentes de manera eficiente. Para lograr esto, se utiliza kqueue, una interfaz de notificación de eventos proporcionada por los sistemas operativos BSD y macOS. kqueue permite monitorear múltiples descriptores de archivo para diversas condiciones de eventos, como lectura, escritura y errores, de una manera escalable y eficiente.

Lógica del Bucle Principal

El servidor crea un socket maestro que escucha en un puerto especificado (8080 por defecto). Este socket se configura como no bloqueante para permitir operaciones de E/S asíncronas. Se crea una instancia de kqueue y se añade el socket maestro a la lista de eventos a monitorear.

El bucle principal del servidor realiza las siguientes operaciones:

    Esperar Eventos: El servidor llama a kevent para esperar eventos en los descriptores de archivo registrados. Esta llamada se bloquea hasta que haya eventos disponibles.

    Procesar Eventos: Cuando kevent devuelve eventos, el servidor los procesa uno por uno:
        Nueva Conexión: Si el evento se asocia con el socket maestro, se acepta una nueva conexión. El nuevo socket de cliente también se configura como no bloqueante y se añade a kqueue para monitorear eventos de lectura.
        Datos Disponibles para Leer: Si el evento se asocia con un socket de cliente, se leen los datos disponibles. Si el cliente se ha desconectado, el socket se cierra. Si se reciben datos, se procesan para determinar la solicitud del cliente y se genera la respuesta apropiada.

Manejo de Conexiones

El manejo eficiente de conexiones es crucial para el rendimiento del servidor. El uso de kqueue permite al servidor manejar miles de conexiones concurrentes sin bloquear, lo que sería ineficiente y escalable con otras técnicas como el uso de hilos o procesos para cada conexión.

    No Bloqueante: Todos los sockets (tanto el maestro como los de clientes) se configuran como no bloqueantes. Esto asegura que las operaciones de E/S no bloqueen el bucle principal del servidor.
    Reutilización de Conexiones: Al finalizar la lectura de datos de un cliente, el socket se cierra y se elimina de kqueue, liberando recursos del sistema.

Uso de kqueue

kqueue es una opción ideal para aplicaciones que requieren manejar un gran número de conexiones simultáneas debido a su eficiencia y escalabilidad. A diferencia de select y poll, que pueden volverse ineficientes con un gran número de descriptores, kqueue maneja eventos de manera eficiente mediante un modelo basado en notificaciones.

Ventajas de kqueue:

    Escalabilidad: Maneja eficientemente miles de descriptores de archivo.
    Eficiencia: Reduce el consumo de CPU y mejora el rendimiento general del servidor.
    Flexibilidad: Soporta una amplia gama de eventos y condiciones de notificación.

En resumen, la combinación de kqueue y E/S no bloqueante permite al servidor HTTP manejar un alto volumen de tráfico con baja latencia y alta eficiencia, lo que es fundamental para aplicaciones web modernas y servicios que requieren un rendimiento robusto y escalable.
`

## Setup

### Dependencies

- `tinycdb` library

### Installation

On macOS, install the required dependency:

```sh
brew install zlog
```

On Linux, install zlog from source or your package manager.

## Usage

### Building the Server

Compile the server using make:

```sh
make
```

This will produce the `http_server` executable.

### Configuration

Edit `config.txt` to set the server port:

```
SERVER_PORT=8080
```

### Running the Server

Start the HTTP redirect server:

```sh
./http_server
```

### Testing the Server

Test redirections using curl:

```sh
curl -I http://localhost:8080/google
```

Expected response:

```
HTTP/1.1 302 Found
Location: https://www.google.com
Content-Length: 0
```

Or use a web browser to navigate to `http://localhost:8080/google` and verify the redirect.

### Stress Testing

The server is designed for high performance. You can stress test it using `wrk`:

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

Benchmark results show the server can handle 50,000+ requests per second with sub-5ms latency.

### Epoll and Kqueue for Portability

Background

The HTTP server is designed to handle a high number of concurrent connections efficiently. To achieve this, it uses different event notification interfaces based on the operating system:

Linux: Uses epoll, which is a scalable I/O event notification mechanism.
BSD/macOS: Uses kqueue, which provides similar functionality for these platforms.

Implementation details

Epoll (Linux)
In Linux, epoll is used to monitor multiple file descriptors to see if I/O is possible on any of them. It's designed to be more efficient than traditional polling methods like select and poll.

Key functions:

epoll_create1(): Creates an epoll instance.
epoll_ctl(): Controls the file descriptors that will be monitored by an epoll instance.
epoll_wait(): Waits for events on the file descriptors monitored by the epoll instance.
Kqueue (BSD/macOS)
For BSD and macOS, kqueue provides an efficient way to monitor multiple file descriptors.

Key Functions:

kqueue(): Creates a new kernel event queue.
kevent(): Registers events with the queue and waits for events.
Portability
The server is designed to be portable between Linux and BSD/macOS by abstracting the event handling logic into separate functions. Depending on the operating system, the appropriate mechanism (epoll or kqueue) is used.

## License

This project is licensed under the terms of the GNU General Public License v3.0. See the LICENSE file for details.

