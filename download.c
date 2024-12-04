#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

void ListFiles(FILE *s)  {
    int bufferSize = 1000;
    char buffer[bufferSize];

    fprintf(s, "LIST\n");
    fflush(s);

    printf("\n%-30s%-30s", "Filename", "Size");             // Header
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
}

// Additional feature: Progress bar displays '-' for how far the download has progressed. Ex: [-------  ]
void ProgressBar(int fileSize, int transferred) {
    const int width = 20;                              // Bar width
    // Find how many bars to display
    float progress = (float)transferred / fileSize;  
    int pos = width * progress;  

    printf("\n["); 
    for (int i = 0; i < width; ++i) {
        if (i < pos) printf("-");
        else printf(" ");
    }
    printf("] %.2f%%\n\n", (float)(progress * 100));         // Percentage
}

void DownloadFile(FILE* s, int fd) {
    int bufferSize = 1000;
    char buffer[bufferSize];

    // Get filename from user
    char fileName[100];
    printf("Which file do you want to download? ");
    scanf("%s", fileName);

    // Additional Feature: If the file already exists in the user's directory, ask if they want to overwrite it
    if (access(fileName, F_OK) == 0) {
        char choice;
        printf("The file '%s' already exists. Input 'o' to overwrite: ", fileName);
        scanf(" %c", &choice);
        if (tolower(choice) != 'o') {
            printf("'%s' will not be downloaded.\n", fileName);
            return;
        }
    }

    // Get SIZE of file
    fprintf(s, "SIZE %s\n", fileName);
    fflush(s);

    if (fgets(buffer, sizeof(buffer), s) == NULL) {
        printf("Failed to find file size.\n");
        return;
    }

    int fileSize = 0;
    if (sscanf(buffer, "+OK %d", &fileSize) != 1 || fileSize < 0) {
        printf("Invalid file name.\n");
        return;
    }
    printf("File size: %d bytes\n", fileSize);

    // Send GET command for file
    fprintf(s, "GET %s\n", fileName);
    fflush(s);

    // Check for errors in server response
    if (fgets(buffer, sizeof(buffer), s) == NULL) {             // Check if fgets was success
        printf("Data was not received.\n");
        return;
    }
    if (strncmp(buffer, "-ERR", 4) == 0) {                      // Check for "-ERR"
        printf("Error retrieving file: %s", buffer);
        return;
    }

    // Save contents of server file to local file
    FILE *destination = fopen(fileName, "w");
    if (!destination) {
        printf("Could not open a file for writing.\n");
        return;
    }

    int transferred = 0;                                        // Total amount transferred so far
    while (transferred < fileSize) {
        int remaining = fileSize - transferred;
        int bytesWanted;

        // min(remaining, bufferSize)
        if (remaining < bufferSize) {
            bytesWanted = remaining;
        } else {
            bytesWanted = bufferSize;
        }

        // Read bytes wanted from server file into the buffer, check for errors?
        int bytesReceived = fread(buffer, 1, bytesWanted, s);  
        if (bytesReceived <= 0) {
            printf("Error reading bytes from server file into buffer.\n");
            break;        
        }

        // Write bytes received into dest file from the buffer
        fwrite(buffer, 1, bytesReceived, destination);
        transferred = transferred + bytesReceived;             // Update the amount transferred 

        // Additional Feature: Show progress bar
        ProgressBar(fileSize, transferred);

    }
    
    printf("The file %s has been downloaded.\n", fileName);  
}

void Quit(FILE *s, int fd) {
    fprintf(s, "QUIT\n");
    fflush(s);
    fclose(s);
    close(fd);
}

void PrintMenu() {
        printf("\n\nMenu:\n\t(L) List Files\n\t(D) Download\n\t(Q) Quit\n");
        printf("\nWhat would you like to do? ");
}

int main() {
    // Ask which server, append server name and domain, open connection on port 3456
    char hostname[100] = "";
    int bufferSize = 1000;
    char buffer[bufferSize];

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

    // Loop to show menu and prompt for choice until Q is selected
    char choice;
    do{
        PrintMenu();
        scanf(" %c", &choice);

        // Clear input buffer
        while (getchar() != '\n'); 

        switch (tolower(choice)) {
            case 'l':
                ListFiles(s);
                break;
            case 'd':
                DownloadFile(s, fd);
                break;
            case 'q':
                Quit(s, fd);
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while(tolower(choice) != 'q');

    return 0;
}