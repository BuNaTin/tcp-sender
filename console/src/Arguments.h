#include <Application/Args.h>

#define ARG_LONG_HELP "help"
#define ARG_HELP "h"

#define ARG_CONFIG_FILE "config"
#define ARG_CONFIG_FILE_SHORT "c"

#define ARG_PORT Args::Name::PORT
#define ARG_PORT_SHORT Args::Name::PORT_SHORT

#define ARG_ADDRESS Args::Name::ADDRESS
#define ARG_ADDRESS_SHORT Args::Name::ADDRESS_SHORT

void buildArgs(Args &args) {
    args.add({ARG_LONG_HELP, ARG_HELP}, "Print help message");

    args.add({ARG_CONFIG_FILE, ARG_CONFIG_FILE_SHORT},
             "Application configuration file",
             Param::not_empty());

    args.add({ARG_PORT, ARG_PORT_SHORT},
             "Destination port",
             Param::not_empty());
             
    args.add({ARG_ADDRESS, ARG_ADDRESS_SHORT},
             "Destination address",
             Param::not_empty());
}
