#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "src/ServerPart.hpp"

int main() {
    struct sockaddr_un name;
    int connectionSocket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (connectionSocket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));

    name.sun_family = AF_UNIX;

    struct stat statistic;

    int isExist = stat(server::gSocketName, &statistic);

    if(isExist == 0){
        remove(server::gSocketName);
    }

    strncpy(name.sun_path, server::gSocketName, sizeof(name.sun_path) - 1);

    int ret = bind(connectionSocket, (const struct sockaddr *) &name, sizeof(name));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    server::ServerPart server(connectionSocket);
    server.start();

    return 0;
}
