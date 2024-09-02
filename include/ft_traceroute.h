#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "libft.h"
#include "argparse.h"

static t_argo options[] = {
    {0, NULL, NULL, NULL, NO_ARG}};

static t_argp argp __attribute__((unused)) = {
    .options = options,
    .args_doc = "[options] <destination>",
    .doc = ""};

#endif