#include "ft_traceroute.h"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char const *argv[])
{
    t_args *args;

    if (parse_args(&argp, argv, &args))
        return 1;
    /*  if (parse_ping_options(&ping->options, args, argv[0]) || ping_init(ping, argv[0]))
         return 1; */

    t_argr *argr = get_next_arg(args);

    if (!argr)
    {
        printf("%s: destination argument required\n", argv[0]);
        free_args(args);
        return 1;
    }
    return 0;
}