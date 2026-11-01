# ft_ping
this project is about re-coding the ping command...

The goal of ft_ping is to understand how the ping utility works internally by implementing ICMP Echo Requests and processing ICMP responses directly through raw sockets, without relying on the system's ping command.

The reference implementation for this project is inetutils-2.0.

---

## Overview
ft_ping is an IPv4 network diagnostic utility that sends ICMP Echo Request packets to a specified IP address or hostname and listens for ICMP responses.

The project focuses on low-level network programming, socket management, packet construction, packet parsing, DNS resolution and ICMP error handling.

---

## Features
* Send ICMP Echo Request packets using IPv4 raw sockets.

* Receive and process ICMP Echo Reply packets.

* Support IPv4 addresses and hostnames.

* Resolve hostnames before sending packets.

* Handle Fully Qualified Domain Names (FQDN).

* Calculate round-trip time (RTT).

* Maintain packet transmission, reception, and error statistics.

* Validate ICMP checksums.

* Handle ICMP error messages, including.

* Support verbose output (-v) for packet diagnostics.

* Support TTL configuration.

* Display packet information and statistics when interrupted.

---

## Requirements
* Linux

* GCC or another compatible C compiler

* Make

* Root privileges or the appropriate capabilities to create raw sockets

Raw sockets generally require elevated privileges.

---

## Installation
Clone the repository and compile the project
```bash
git clone git@github.com:42-student/ft_ping.git
cd ft_ping
make
```

---

## Usage
Run the program with an IPv4 address or hostname
```bash
//IPv4
sudo ./ft_ping 8.8.8.8

//hostname
sudo ./ft_ping google.com
```

---

## Available options
| Option        | Description                                                                    |
|---------------|--------------------------------------------------------------------------------|
| -? -h --help  | display usage information                                                      |
| -v            | enable verbose output, including packet diagnostics and ICMP error information |
| --ttl=value   | set the IPv4 Time To Live (TTL) value                                          |
| -q --quiet    | suppresses the individual “reply” lines, quiet mode                            |

---
