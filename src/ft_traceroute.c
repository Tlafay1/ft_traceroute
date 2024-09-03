#include "ft_traceroute.h"

int send_packet(TRACE_R *traceroute)
{
    char message[2000];

    socklen_t server_struct_length = sizeof(struct sockaddr_in);

    memset(message, '\0', sizeof(message));

    if (sendto(traceroute->socket_desc, message, sizeof(message), 0, (struct sockaddr *)&traceroute->dest, server_struct_length) < 0)
    {
        perror("Could not send packet");
        return 1;
    }

    if (recvfrom(traceroute->socket_desc, message, sizeof(message), 0, (struct sockaddr *)&traceroute->dest, &server_struct_length) < 0)
    {
        perror("Could not receive packet");
        return 1;
    }

    printf("Received packet from %s:%d\nData: %s\n\n",
           inet_ntoa(traceroute->dest.sin_addr), ntohs(traceroute->dest.sin_port), message);

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
    struct addrinfo hints;
    struct addrinfo *res;
    struct sockaddr_in *ipv4;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, NULL, &hints, &res) != 0)
        return 1;

    ipv4 = (struct sockaddr_in *)res->ai_addr;
    traceroute->dest = *ipv4;

    ft_strlcpy(traceroute->hostname, host, HOST_NAME_MAX);
    printf("Destination: %s\n", traceroute->hostname);

    freeaddrinfo(res);

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

    set_dest(&traceroute, argr->values[0]);

    traceroute.socket_desc = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

    if (traceroute.socket_desc < 0)
    {
        perror("Could not create socket");
        return 1;
    }

    send_packet(&traceroute);

    return 0;
}