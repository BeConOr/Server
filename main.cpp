#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "src/ServerPart.hpp"

int main() {
    struct sockaddr_un name;
    int connectionSocket = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    if (connectionSocket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));

    name.sun_family = AF_UNIX;
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
