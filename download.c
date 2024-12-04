#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <string.h>
#include <unistd.h>

int main() {
    // Ask which server, append server name and domain, open connection on port 3456
    char hostname[100] = "";
    int bufferSize = 1000;

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

    
    // Send LIST for file listing, then print formatted file list
    char buffer[bufferSize];

    fprintf(s, "LIST\n");
    fflush(s);

    printf("\n%-30s%-30s", "Filename", "Size");            // Header
    printf("\n---------------------------------------\n");

    while (fgets(buffer, sizeof(buffer), s) != NULL) {   
        if (strcmp(buffer, ".\n") == 0) {                   // Check for end-of-data marker
            break;
        }

        int size;
        char name[100];
        int getSizeName = sscanf(buffer, "%d %s", &size, name);

        // Make sure size and name was found before printing
        if (getSizeName == 2) {
            printf("\n%-30s", name); 
            printf("%-30d", size); 
            // TODO Print size in KB, GB, etc.
        }
    }



    // Get filename from user
    char fileName[100];
    printf("\nWhich file do you want to get? ");
    scanf("%s", fileName);

    // Send GET command with the filename
    fprintf(s, "GET %s\n", fileName);
    fflush(s);

    // Check for errors in server response
    int fileSize = 60; // Size of file

    if (fgets(buffer, sizeof(buffer), s) == NULL) {             // Check if fgets was success
        printf("Data was not received.\n");
        fclose(s);
        close(fd);
        return 1;
    }
    printf("Server Response: %s", buffer);                      // TEST raw server response
    if (strncmp(buffer, "-ERR", 4) == 0) {                      // Chck for "-ERR"
        printf("Error retrieving file: %s", buffer);
        fclose(s);
        close(fd);
        return 1;
    }
    /* if (sscanf(buffer, "+OK %d", &fileSize) != 1) {             // Get file size
        printf("Server Response: %s", buffer);                  // TEST raw server response
        printf("Invalid response from server: %s", buffer);
        fclose(s);
        close(fd);
        return 1;
    } */

    /* // Get file size
    fprintf(s, "SIZE %s\n", fileName);
    fflush(s);
    char fileSize[100];
    char sizeBuffer[100];
    fgets(sizeBuffer, 100, s);
    printf("%s", sizeBuffer);
    sscanf(sizeBuffer, "+OK %s", fileSize); 

    printf("File size %s", fileSize); */

    // Save contents of server file to local file
    FILE *destination = fopen(fileName, "wb");
    if (!destination) {
        printf("Could not open a file for writing.\n");
        fclose(s);
        close(fd);
        return 1;
    }

    int transferred = 0;                                        // Total amount transferred so far

    while (transferred < bufferSize) {
        int remaining = fileSize - transferred;
        int bytesWanted;

        // min(remaining, bufferSize)
        if (remaining < bufferSize) {
            bytesWanted = remaining;
        } else {
            bytesWanted = bufferSize;
        }

        // Read bytes_wanted from server file into the buffer
        int bytesReceived = fread(buffer, 1, bytesWanted, s);  
        if (bytesReceived <= 0) {
            printf("Error receiving data or unexpected end.\n");
            break;          // TODO idk about this one
        }

        // write bytes_received into dest file from the buffer
        fwrite(buffer, 1, bytesReceived, destination);
        transferred = transferred + bytesReceived;             // Update the amount transferred 

    }
    
    printf("\nThe file %s has been downloaded.\n", fileName); 
    
    // Close connection
    fprintf(s, "QUIT\n");
    fflush(s);
    fclose(s);

    return 0;
}