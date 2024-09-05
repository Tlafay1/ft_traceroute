#include "ft_traceroute.h"

int send_packet(TRACE_R *traceroute)
{
    char buffer[1024];
    bool timed_out = false;

    socklen_t tosize = sizeof(traceroute->to);

    int ttl = traceroute->ttl;
    if (setsockopt(traceroute->udp_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
    {
        perror("Could not set TTL");
        return 1;
    }

    memcpy(buffer, "Hello", 5);
    buffer[5] = '\0';

    if (sendto(traceroute->udp_fd, buffer, ft_strlen(buffer), 0, (struct sockaddr *)&traceroute->to, tosize) < 0)
    {
        perror("Could not send packet");
        return 1;
    }

    struct timeval timeout;
    timeout.tv_sec = traceroute->timeout;
    timeout.tv_usec = 0;
    if (setsockopt(traceroute->icmp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("Could not set socket timeout");
        return 1;
    }

    if (recvfrom(traceroute->icmp_fd, buffer, sizeof buffer - 1, 0, (struct sockaddr *)&traceroute->from, &tosize) < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            timed_out = true;
        }
        else
        {
            perror("Could not receive packet");
            return 1;
        }
    }

    printf("  %d   %s\n", ttl, timed_out ? "*" : inet_ntoa(traceroute->from.sin_addr));

    return 0;
}

/**
 * Sets the destination address for the TRACE_R structure.
 *
 * @param trouceroute The TRACE_R structure to set the destination for.
 * @param host The hostname or IP address of the destination.
 * @return Returns 0 on success, or 1 if an error occurred.
 */
static int set_dest(TRACE_R *traceroute, const char *host)
{
    struct hostent *hostinfo = NULL;

    if ((hostinfo = gethostbyname(host)) == NULL)
    {
        perror("Could not get host by name");
        return 1;
    }

    traceroute->to.sin_addr = *(struct in_addr *)hostinfo->h_addr;
    traceroute->to.sin_family = AF_INET;
    traceroute->to.sin_port = htons(PORT);

    memcpy(&traceroute->to.sin_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
    strncpy(traceroute->hostname, host, HOST_NAME_MAX);

    return 0;
}

int main(__attribute__((unused)) int argc, const char *argv[])
{
    TRACE_R traceroute;
    t_args *args;

    traceroute.progname = argv[0];

    if (parse_args(&argp, argv, &args))
        return 1;

    t_argr *argr = get_next_arg(args);

    if (!argr)
    {
        help_args(&argp, traceroute.progname);
        free_args(args);
        return 1;
    }

    if (set_dest(&traceroute, argr->values[0]))
    {
        perror("Could not set destination");
        free_args(args);
        return 1;
    }

    traceroute.udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (traceroute.udp_fd < 0)
    {
        perror("Could not create socket");
        return 1;
    }

    struct protoent *proto = getprotobyname("icmp");
    if (!proto)
    {
        fprintf(stderr, "%s: unknown protocol icmp.\n", traceroute.progname);
        return -1;
    }
    traceroute.icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (traceroute.icmp_fd < 0)
    {
        if (errno == EPERM || errno == EACCES)
        {
            errno = 0;
            traceroute.icmp_fd = socket(AF_INET, SOCK_DGRAM, proto->p_proto);
            if (traceroute.icmp_fd < 0)
            {
                if (errno == EPERM || errno == EACCES || errno == EPROTONOSUPPORT)
                    printf("%s: Lacking privilege for icmp socket.\n", traceroute.progname);
                else
                    printf("%s: %s\n", traceroute.progname, strerror(errno));

                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    traceroute.port = PORT;
    traceroute.ttl = FIRST_TTL;
    traceroute.max_ttl = MAX_HOPS;
    traceroute.timeout = TIMEOUT;
    traceroute.nqueries = NQUERIES;

    if (setsockopt(traceroute.icmp_fd, IPPROTO_IP, IP_TTL,
                   &traceroute.ttl, sizeof(traceroute.ttl)) < 0)
    {
        perror("Could not set TTL");
        return 1;
    }
    printf("Traceroute to %s (%s), %d hops max, %d byte packets\n",
           traceroute.hostname,
           inet_ntoa(traceroute.to.sin_addr),
           traceroute.max_ttl,
           60);

    while (traceroute.ttl <= traceroute.max_ttl)
    {
        send_packet(&traceroute);
        traceroute.ttl++;
    }

    close(traceroute.udp_fd);
    free_args(args);

    return 0;
}