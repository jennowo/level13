#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <string.h>
#include <unistd.h>

int main() {
    // Ask which server, append server name and domain, open connection on port 3456
    char hostname[100] = "";

    printf("Would you like to connect to London or Newark?\n");
    scanf("%s", hostname);
    strcat(hostname, ".cs.sierracollege.edu");

    int fd = create_inet_stream_socket(hostname, "3456", LIBSOCKET_IPv4, 0);
    if (fd < 0)
    {
        printf("Can't create socket.\n");
        return 1;
    }
    printf("Connected to %s.\n", hostname);

    // Convert fd (int) into FILE *, mode read-write
    FILE *s = fdopen(fd, "r+");
    if (!s) {
        printf("File could not be opened.\n");
        return 1;
    }

    // Receive initial greeting
    char buffer[1000];
    if (fgets(buffer, 1000, s) == NULL) {
        printf("Could not receive data.\n");
        fclose(s);
        close(fd);
        return 1;
    }

    // Print the server's initial greeting
    printf("Server says: %s", buffer);
    

    return 0;
}