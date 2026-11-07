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

## Available Options
| Option        | Description                                                                    |
|---------------|--------------------------------------------------------------------------------|
| -? -h --help  | display usage information                                                      |
| -v            | enable verbose output, including packet diagnostics and ICMP error information |
| --ttl=value   | set the IPv4 Time To Live (TTL) value                                          |
| -q --quiet    | suppresses the individual “reply” lines, quiet mode                            |

---

## Technical Implementation

### Raw sockets
The program uses an IPv4 raw socket with the ICMP protocol:
```bash
socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```
This allows ft_ping to construct and send ICMP packets directly and receive ICMP messages from the network stack.

### ICMP Echo Request construction
Each outgoing packet contains:

* ICMP Type (ICMP_ECHO)

* ICMP Code

* Identifier

* Sequence number

* Checksum

* Payload containing a timestamp and additional data

The timestamp is used to calculate the round-trip time when a reply is received.

### Packet reception and parsing
Incoming packets are received using recvfrom().

The implementation parses the IPv4 header and locates the ICMP header using the IPv4 Internet Header Length (IHL).

It also performs basic validation to protect against truncated or malformed packets.

### Reply validation
Received ICMP packets are validated before being considered responses to the current request.

The validation process includes:

* Checking the ICMP type

* Matching the packet identifier

* Matching the sequence number

* Verifying the ICMP checksum

* Identifying ICMP error messages

For ICMP error messages, the program extracts the original IP and ICMP headers embedded inside the error packet. This makes it possible to determine whether the error belongs to one of the requests sent by ft_ping.

### ICMP error handling
ICMP errors are not treated as ordinary Echo Replies.

The program identifies and reports errors such as Destination Unreachable and Time Exceeded.

In verbose mode, it also displays information extracted from the original packet embedded in the ICMP error response.

### Round Trip Time and statistics
The program records timestamps to calculate the round-trip time of successful Echo Replies.

It maintains statistics including:

* Packets transmitted

* Packets received

* ICMP errors

* Packet loss percentage

* Minimum RTT

* Average RTT

* Maximum RTT

Statistics are displayed when the program is interrupted.

### Verbose mode
The -v option provides additional information about received packets and ICMP errors.

For ICMP error packets, the diagnostic output includes:

* Original IPv4 header dump

* IPv4 version and header length

* Type of service

* Total packet length

* IP identification

* Fragmentation flags and offset

* TTL and protocol

* Header checksum

* Source and destination addresses

* Original ICMP type, code, identifier and sequence number

The output is designed to follow the formatting of the inetutils-2.0 reference implementation.

---

## Learning Objectives
This project provided an opportunity to explore several networking concepts and low-level programming techniques:

* IPv4 networking fundamentals

* ICMP protocol and packet structure

* Raw socket programming in C

* Network byte-order and byte-order conversions

* Internet checksum calculation

* IPv4 and ICMP packet parsing

* DNS resolution

* ICMP error messages and embedded packet inspection

* Socket timeouts using select()

* RTT calculations and packet statistics

* Signal handling and graceful termination

* Debugging network traffic with tcpdump and strace

---

## Reference
* inetutils-2.0

* RFC 792 – Internet Control Message Protocol

* RFC 791 – Internet Protocol

* Linux man pages

---

#### Developed as part of the 42 curriculum.

#### This project is intended for educational purposes and is not a replacement for the system's original ping utility.

---





























