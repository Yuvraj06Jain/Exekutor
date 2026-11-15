/* discovery.h
 *
 * UDP discovery (spec Section 2C), written by C.
 *
 * THE PROBLEM: workers and clients shouldn't need to be told where the
 * scheduler is. So the scheduler listens on a UDP port (default 9999), and a
 * worker or client that wants to find it broadcasts "where are you?" to the
 * whole local network. The scheduler answers with its TCP port, and the
 * answer's sender address tells the asker the scheduler's IP.
 *
 * WHO CALLS WHAT
 *   scheduler  -> discovery_responder_start()   (once, at startup)
 *   worker     -> discovery_find_scheduler()    (at startup and after losing the connection)
 *   client     -> discovery_find_scheduler()    (same)
 *
 * THE PACKETS (both 12 bytes)
 *   question:  "DCC_FIND" + 4 zero bytes
 *   answer:    "DCC_HERE" + the scheduler's TCP port (4 bytes, network byte order)
 * The 8-letter marker lets each side ignore any unrelated packet that happens
 * to land on the port.
 */
#ifndef COMMON_DISCOVERY_H
#define COMMON_DISCOVERY_H

#include <stddef.h>

/* Sets the tag used in the log lines discovery_find_scheduler() prints,
 * e.g. "WORKER" or "CLIENT". Without this call the tag is "DISCOVERY".
 * Call it once at startup, before searching. */
void discovery_set_log_tag(char *tag);

/* Finds the scheduler. Used by the worker and the client.
 *
 *   udp_port      discovery port to broadcast on (-discovery-port; 0 or less means 9999)
 *   ip_out        buffer that receives the scheduler's IP as text, e.g. "192.168.1.20"
 *   ip_len        size of ip_out; use at least INET_ADDRSTRLEN (16) or no answer can be stored
 *   tcp_port_out  receives the scheduler's TCP port
 *
 * Makes DISCOVERY_RETRIES attempts, waiting DISCOVERY_TIMEOUT seconds each.
 * Returns 0 if the scheduler answered (ip_out and *tcp_port_out are filled in).
 * Returns -1 if nobody answered. Then the CALLER must use the address from
 * the command line, and print and log that it did so
 * (e.g. "No discovery reply, using fallback 127.0.0.1:8080"), or print an
 * error and exit if no address was given. */
int discovery_find_scheduler(int udp_port, char *ip_out, size_t ip_len, int *tcp_port_out);

/* Starts answering discovery questions. Used by the scheduler.
 *
 *   udp_port  UDP port to listen on (-discovery-port; 0 or less means 9999)
 *   tcp_port  the scheduler's TCP port; this is what gets sent back to askers
 *
 * The answering runs in a background thread, so this returns straight away
 * and never blocks the scheduler. Returns 0 if it started, or -1 if the socket
 * couldn't be set up (e.g. the port couldn't be bound). */
int discovery_responder_start(int udp_port, int tcp_port);

#endif
