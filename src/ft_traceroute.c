#include "ft_traceroute.h"

int send_packet(TRACE_R *traceroute)
{
    char buffer[1024];

    socklen_t tosize = sizeof(traceroute->to);

    int ttl = traceroute->ttl;
    if (setsockopt(traceroute->socket_desc, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
    {
        perror("Could not set TTL");
        return 1;
    }

    memcpy(buffer, "Hello", 5);
    buffer[5] = '\0';

    if (sendto(traceroute->socket_desc, buffer, ft_strlen(buffer), 0, (struct sockaddr *)&traceroute->to, tosize) < 0)
    {
        perror("Could not send packet");
        return 1;
    }

    // struct timeval timeout;
    // timeout.tv_sec = 1; // 5 seconds timeout
    // timeout.tv_usec = 0;
    // if (setsockopt(traceroute->socket_desc, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    // {
    //     perror("Could not set socket timeout");
    //     return 1;
    // }

    int n;
    if ((n = recvfrom(traceroute->socket_desc, buffer, 2, sizeof buffer - 1, (struct sockaddr *)&traceroute->from, &tosize)) < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            perror("Receive timed out");
        }
        else
        {
            perror("Could not receive packet");
        }
        // return 1;
    }

    buffer[n] = '\0';

    printf("Received packet from %s, ttl=%d\n", inet_ntoa(traceroute->from.sin_addr), ttl);

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

    if (parse_args(&argp, argv, &args))
        return 1;

    t_argr *argr = get_next_arg(args);

    if (!argr)
    {
        help_args(&argp, argv[0]);
        free_args(args);
        return 1;
    }

    if (set_dest(&traceroute, argr->values[0]))
    {
        perror("Could not set destination");
        free_args(args);
        return 1;
    }

    traceroute.socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    traceroute.port = PORT;
    traceroute.ttl = 1;
    traceroute.max_ttl = MAX_HOPS;

    if (traceroute.socket_desc < 0)
    {
        perror("Could not create socket");
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

    close(traceroute.socket_desc);
    free_args(args);

    return 0;
}