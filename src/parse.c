#include "ft_traceroute.h"

void parse_traceroute_options(TRACE_R *traceroute, t_args *args)
{
    t_argr *argr;

    while ((argr = get_next_option(args)))
    {
        switch (argr->option->sflag)
        {
        case 'f':
            parse_first_hop(traceroute, argr->values[0]);
            break;
        case 'm':
            parse_max_hop(traceroute, argr->values[0]);
            break;
        case 'p':
            parse_port(traceroute, argr->values[0]);
            break;
        case 'q':
            parse_tries(traceroute, argr->values[0]);
            break;
        case 'w':
            parse_wait(traceroute, argr->values[0]);
            break;
        default:
            printf("%s: this definitely should not have happened...\n", traceroute->progname);
            exit(EXIT_FAILURE);
        }
    }
}

void parse_first_hop(TRACE_R *traceroute, char *arg)
{
    char *p;
    traceroute->first_ttl = strtol(arg, &p, 10);
    if (*p || traceroute->first_ttl < FIRST_TTL || traceroute->first_ttl > MAX_HOPS)
    {
        fprintf(stderr, "%s: Impossible distance `%s'\n", traceroute->progname, arg);
        exit(EXIT_FAILURE);
    }
}

void parse_max_hop(TRACE_R *traceroute, char *arg)
{
    char *p;
    traceroute->max_ttl = strtol(arg, &p, 10);
    if (*p || traceroute->max_ttl < FIRST_TTL || traceroute->max_ttl > MAX_HOPS)
    {
        fprintf(stderr, "%s: invalid hops value `%s'\n", traceroute->progname, arg);
        exit(EXIT_FAILURE);
    }
}

void parse_port(TRACE_R *traceroute, char *arg)
{
    char *p;
    traceroute->port = strtol(arg, &p, 10);
    if (*p || traceroute->port < 1 || traceroute->port > USHRT_MAX)
    {
        fprintf(stderr, "%s: invalid port number `%s'\n", traceroute->progname, arg);
        exit(EXIT_FAILURE);
    }
}

void parse_tries(TRACE_R *traceroute, char *arg)
{
    char *p;
    traceroute->nqueries = strtol(arg, &p, 10);
    if (*p || traceroute->nqueries < 1 || traceroute->nqueries > 10)
    {
        fprintf(stderr, "%s: number of tries should be between 1 and 10\n", traceroute->progname);
        exit(EXIT_FAILURE);
    }
}

void parse_wait(TRACE_R *traceroute, char *arg)
{
    char *p;
    traceroute->timeout = strtol(arg, &p, 10);
    if (*p || traceroute->timeout < 0 || traceroute->timeout > 60)
    {
        fprintf(stderr, "%s: ridiculous waiting time `%s'\n", traceroute->progname, arg);
        exit(EXIT_FAILURE);
    }
}