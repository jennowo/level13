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

    // TEST Receive and print initial greeting
    char buffer[1000];
    if (fgets(buffer, 1000, s) == NULL) {
        printf("Could not receive data.\n");
        fclose(s);
        close(fd);
        return 1;
    }
    printf("\nInitial response: %s", buffer);

    // TEST Send HELO command, receive and print response
    fprintf(s, "HELO\n"); // Remember newline
    fflush(s);
    if (fgets(buffer, 1000, s) == NULL) {
        printf("Could not receive data.\n");
        fclose(s);
        close(fd);
        return 1;
    }
    printf("HELO response: %s", buffer);
    
    // Send LIST for file listing, print file list
    fprintf(s, "LIST\n");
    fflush(s);

    printf("\nFile listing:\n");
    while (fgets(buffer, 1000, s) != NULL) {
        if (strcmp(buffer, ".\n") == 0) { // Check for end-of-data marker
            break;
        }
        printf("%s", buffer); // Print each line
    }


    // Close connection
    fprintf(s, "QUIT\n");
    fflush(s);
    fclose(s);

    return 0;
}