# Webserv

A lightweight HTTP server written in C++98 for 42 Warsaw School.

## Overview

This project is an implementation of a simple HTTP server that follows the HTTP/1.1 protocol standard. The server is built entirely in C++98 without using any external libraries and is designed to handle client requests, route them to the appropriate handlers, and generate proper HTTP responses.

## Features

- Non-blocking I/O with `poll()`
- Configurable virtual servers
- HTTP/1.1 protocol support
- Method handling: GET, POST, DELETE
- Chunked transfer encoding support
- CGI script execution
- Custom error pages
- Directory listing (autoindex)
- Client request routing
- File uploads
- Keep-alive connections
- Multiple server configurations

## Configuration

The server is configured via a configuration file that defines:
- Port and host
- Server names
- Default error pages
- Client body size limits
- Routes with specific configurations:
  - Allowed methods
  - Redirection
  - Directory listings
  - CGI execution
  - Upload directories

## Usage

```bash
# Compile the project
make

# Run the server with a configuration file
./webserv [config_file]

# Default configuration
./webserv example_files/webserv.conf
```

## Technical Implementation
- Socket Programming: Uses POSIX socket API for network communication
- Non-blocking I/O: Uses poll() for event-driven architecture
- Request Parsing: Parses HTTP requests with proper header and body handling
- Chunked Encoding: Supports chunked transfer encoding in request bodies
- Response Generation: Constructs valid HTTP responses based on request and configuration
- CGI Support: Executes and communicates with CGI scripts via environment variables and pipes
