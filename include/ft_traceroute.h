#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include <limits.h>
#include <stdbool.h>
#include <sys/time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_systm.h>

#include "libft.h"
#include "argparse.h"

static t_argo options[] = {
    {'f', "first-hop", "first-hop", "set initial hop distance, i.e., time-to-live", ONE_ARG},
    {'m', "max-hop", "max-hop", "set maximal hop count (default: 64)", ONE_ARG},
    {'p', "port", "port", "use destination PORT port (default: 33434)", ONE_ARG},
    {'q', "tries", "queries", "send NUM probe packets per hop (default: 3)", ONE_ARG},
    {'w', "wait", "wait", "wait NUM seconds for response (default: 3)", ONE_ARG},
    {0, NULL, NULL, NULL, NO_ARG}};

static t_argp argp __attribute__((unused)) = {
    .options = options,
    .args_doc = "[OPTION...] HOST",
    .doc = "Print the route packets trace to network host.\n"};

#define MAXIPLEN 60
#define MAXICMPLEN 76

#define CAPTURE_LEN (MAXIPLEN + MAXICMPLEN)

#define FIRST_TTL 1

#define MAX_HOPS 64

#define PORT 33434

#define NQUERIES 3

#define TIMEOUT 3

/*
 * @brief Structure for the traceroute command
 */
typedef struct traceroute_args TRACE_R;
struct traceroute_args
{
    struct sockaddr_in to;
    struct sockaddr_in from;
    char hostname[HOST_NAME_MAX];
    struct timeval sent;
    struct timeval received;
    const char *progname;
    int udp_fd;
    int icmp_fd;
    int ttl;
    int max_ttl;
    int timeout;
    int nqueries;
    int port;
};

#endif