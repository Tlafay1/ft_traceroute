#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include <limits.h>

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
    {'p', "port", "port", "Set the port to use", ONE_ARG},
    {'m', "max-ttl", "max-ttl", "Set the max ttl to use", ONE_ARG},
    {'f', "first-ttl", "first-ttl", "Set the first ttl to use", ONE_ARG},
    {0, NULL, NULL, NULL, NO_ARG}};

static t_argp argp __attribute__((unused)) = {
    .options = options,
    .args_doc = "host [packetlen] [options]",
    .doc = "Arguments:\n"
           "+     host          The host to traceroute to\n"
           "      packetlen     The full packet length(default is the length of an IP\n"
           "                    header plus 40). Can be ignored or increased to a minimal\n"
           "                    allowed value\n"
           "Options :"};

#define PORT 33434

#define MAX_HOPS 30

#define FIRST_TTL 1

/*
 * @brief Structure for the traceroute command
 */
typedef struct traceroute_args TRACE_R;
struct traceroute_args
{
    struct sockaddr_in to;
    struct sockaddr_in from;
    char hostname[HOST_NAME_MAX];
    int socket_desc;
    // int packetlen;
    int ttl;
    int max_ttl;
    // int nqueries;
    // int nprobes;
    // int waittime;
    int port;
};

#endif