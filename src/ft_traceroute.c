#include "ft_traceroute.h"

int send_packet(TRACE_R *traceroute)
{
    char message[] = "Superman";

    socklen_t tosize = sizeof(traceroute->to);

    int ttl = traceroute->ttl;
    if (setsockopt(traceroute->udp_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
    {
        perror("Could not set TTL");
        return EXIT_FAILURE;
    }

    if (sendto(traceroute->udp_fd, message, ft_strlen(message), 0, (struct sockaddr *)&traceroute->to, tosize) < 0)
    {
        perror("Could not send packet");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int recv_packet(TRACE_R *traceroute, bool *done)
{
    char buffer[CAPTURE_LEN];
    socklen_t fromsize = sizeof(traceroute->from);
    ssize_t received;
    uint hlen;
    // struct timeval now, sent, *tp;
    struct icmphdr *icp;

    received = recvfrom(traceroute->icmp_fd, buffer, sizeof buffer - 1, 0, (struct sockaddr *)&traceroute->from, &fromsize);

    if (received < 0)
    {
        perror("Could not receive packet");
        return EXIT_FAILURE;
    }

    struct ip *ip_packet = (struct ip *)buffer;
    hlen = ip_packet->ip_hl << 2;

    icp = (struct icmphdr *)(buffer + hlen);

    *done = icp->type == ICMP_DEST_UNREACH || icp->type == ICMP_ECHOREPLY;
    return EXIT_SUCCESS;
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
        return EXIT_FAILURE;
    }

    traceroute->to.sin_addr = *(struct in_addr *)hostinfo->h_addr;
    traceroute->to.sin_family = AF_INET;
    traceroute->to.sin_port = htons(PORT);

    memcpy(&traceroute->to.sin_addr, hostinfo->h_addr_list[0], hostinfo->h_length);
    strncpy(traceroute->hostname, host, HOST_NAME_MAX);

    return EXIT_SUCCESS;
}

int main(__attribute__((unused)) int argc, const char *argv[])
{
    TRACE_R traceroute;
    t_args *args;

    traceroute.progname = argv[0];

    if (parse_args(&argp, argv, &args))
        return EXIT_FAILURE;

    t_argr *argr = get_next_arg(args);

    if (!argr)
    {
        fprintf(stderr, "%s: missing host operand\nTry '%s --help' for more information\n",
                traceroute.progname, traceroute.progname);
        free_args(args);
        return EXIT_FAILURE;
    }

    if (set_dest(&traceroute, argr->values[0]))
    {
        perror("Could not set destination");
        free_args(args);
        return EXIT_FAILURE;
    }

    traceroute.udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (traceroute.udp_fd < 0)
    {
        perror("Could not create socket");
        return EXIT_FAILURE;
    }

    struct protoent *proto = getprotobyname("icmp");
    if (!proto)
    {
        fprintf(stderr, "%s: unknown protocol icmp.\n", traceroute.progname);
        return EXIT_FAILURE;
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

                return EXIT_FAILURE;
            }
        }
        else
        {
            return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }
    printf("Traceroute to %s (%s), %d hops max\n",
           traceroute.hostname,
           inet_ntoa(traceroute.to.sin_addr),
           traceroute.max_ttl);

    bool done = false;
    struct timeval timeout;
    fd_set readset;
    while (traceroute.ttl <= traceroute.max_ttl && !done)
    {
        in_addr_t previous_addr = 0;
        printf("%2d  ", traceroute.ttl);
        for (int tries = 0; tries < traceroute.nqueries; tries++)
        {
            FD_ZERO(&readset);
            FD_SET(traceroute.icmp_fd, &readset);

            memset(&timeout, 0, sizeof(timeout));
            timeout.tv_sec = traceroute.timeout;
            timeout.tv_usec = 0;

            send_packet(&traceroute);

            int ret = select(traceroute.icmp_fd + 1, &readset, NULL, NULL, &timeout);
            if (ret < 0)
            {
                perror("Could not select");
                return EXIT_FAILURE;
            }
            else if (ret == 0)
            {
                printf(" * ");
                fflush(stdout);
            }
            else if (FD_ISSET(traceroute.icmp_fd, &readset))
            {
                recv_packet(&traceroute, &done);
                if (previous_addr != traceroute.from.sin_addr.s_addr)
                    printf(" %s ", inet_ntoa(traceroute.from.sin_addr));
                previous_addr = traceroute.from.sin_addr.s_addr;
                fflush(stdout);
            }
        }
        printf("\n");
        traceroute.ttl++;
    }

    close(traceroute.udp_fd);
    free_args(args);

    return EXIT_SUCCESS;
}