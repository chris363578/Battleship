#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Global variables
char buffer[256];
char line[256];
bool finish = false;
const int L = 9;
const int W = 9;

// Node definition
struct Node {
    bool hit;
    bool ship;
};

// Function definitions
void server(int);
void *thread(void *vargp);
void fireShot(struct Node[L][W], int*, int coord[6], int*, int);

// Error function
void error(char *msg) {
    perror(msg);
    exit(1);
}

// Main
int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    
    // Set up server by getting ip and port addresses.
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
     fprintf(stderr,"ERROR, no port provided\n");
     exit(1);
    }
    // Set socket file descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    
    // Listen call
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    
    // Infinite server loop
    loop:
        while(1) {
            // Wait for accept call and create new thread.
            clientlen = sizeof(struct sockaddr_storage);
            connfdp = malloc(sizeof(int)); 
            *connfdp = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            pthread_create(&tid, NULL, thread, connfdp);
        }
    // In case loop gets broken.
    goto loop;
    return 0;
}


// Server funcionality
void server(int connfd) {
    int choice = 1;
    char c;
    int hitsTaken = 0;
    int myX, myY = 0;
    int use = 0;
    int count = 0;
    int coord[6];
    
    struct Node mGrid[L][W];
    struct Node eGrid[L][W];
    
    // Generate number for map preset
    srandom(time(NULL));
    choice = random() % 4 + 1;
    
    // Open file
    FILE* file;
    file = fopen("text.txt", "r");
    if (file == NULL) {
        printf("Unable to open file\n");
        return;
    }
    
    
    // Initialize everything to false
    for(int i = 0; i < L; i++) {
        for(int j = 0; j < W; j++) {
            mGrid[i][j].hit = false;
            mGrid[i][j].ship = false;
            
            eGrid[i][j].hit = false;
            eGrid[i][j].ship = false;
        }
    }
    
    // Read correct map
    while ((c = fgetc(file)) != EOF) {
        if(c == (choice + '0')) {
            for(int i = 0; i < 9; i++) {
                for(int j = 0; j < 10; j++) {
                    c = fgetc(file);
                    if(c == '#') {
                        mGrid[i][j - 1].ship = true;
                    }
                }
            }
        }
    }
    
    
    while(!finish) {
        // Read first input
        bzero(buffer,256);
        int n = read(connfd, buffer, 256);
        // Help protect from closing due to sudden connection loss
        if (n == 0 || n < 0) {
            finish = true;
        }
        
        else {
            strcpy(line, buffer);

            // Respond if tomahawk shot is recieved. It will check for all 5 shots.
            if(!strcmp(line, "hawk")) {
                bzero(buffer, 256);
                read(connfd,buffer,256);
                myX = atoi(buffer);

                // Read y
                bzero(buffer, 256);
                read(connfd,buffer,256);
                myY = atoi(buffer);

                // Send hit or miss for shot 1
                if(mGrid[myX + 1][myY].ship && !mGrid[myX + 1][myY].hit && (myX + 1) < 9) {
                    mGrid[myX][myY].hit = true;
                    hitsTaken++;

                    if(hitsTaken == 9) {
                        bzero(buffer, 256);
                        strcpy(buffer, "dead");
                        write(connfd,buffer,256);
                        finish = true;
                    }

                    else {
                        bzero(buffer, 256);
                        strcpy(buffer, "hit");
                        write(connfd,buffer,256);
                    }
                }

                else {
                    bzero(buffer, 256);
                    strcpy(buffer, "miss");
                    write(connfd,buffer,256);
                }


                // Send hit or miss for shot 2
                if(mGrid[myX - 1][myY].ship && !mGrid[myX - 1][myY].hit && (myX - 1) > -1) {
                    mGrid[myX][myY].hit = true;
                    hitsTaken++;

                    if(hitsTaken == 9) {
                        bzero(buffer, 256);
                        strcpy(buffer, "dead");
                        write(connfd,buffer,256);
                        finish = true;
                    }

                    else {
                        bzero(buffer, 256);
                        strcpy(buffer, "hit");
                        write(connfd,buffer,256);
                    }
                }

                else {
                    bzero(buffer, 256);
                    strcpy(buffer, "miss");
                    write(connfd,buffer,256);
                }

                // Send hit or miss for shot 3
                if(mGrid[myX][myY + 1].ship && !mGrid[myX][myY + 1].hit && (myY + 1) < 9) {
                    mGrid[myX][myY].hit = true;
                    hitsTaken++;

                    if(hitsTaken == 9) {
                        bzero(buffer, 256);
                        strcpy(buffer, "dead");
                        write(connfd,buffer,256);
                        finish = true;
                    }

                    else {
                        bzero(buffer, 256);
                        strcpy(buffer, "hit");
                        write(connfd,buffer,256);
                    }
                }

                else {
                    bzero(buffer, 256);
                    strcpy(buffer, "miss");
                    write(connfd,buffer,256);
                }

                // Send hit or miss  for shot 4
                if(mGrid[myX][myY - 1].ship && !mGrid[myX][myY - 1].hit && (myY - 1) > -1) {
                    mGrid[myX][myY].hit = true;
                    hitsTaken++;

                    if(hitsTaken == 9) {
                        bzero(buffer, 256);
                        strcpy(buffer, "dead");
                        write(connfd,buffer,256);
                        finish = true;
                    }

                    else {
                        bzero(buffer, 256);
                        strcpy(buffer, "hit");
                        write(connfd,buffer,256);
                    }
                }

                else {
                    bzero(buffer, 256);
                    strcpy(buffer, "miss");
                    write(connfd,buffer,256);
                }


                // Send hit or miss for shot 5
                if(mGrid[myX][myY].ship && mGrid[myX][myY].hit) {
                    mGrid[myX][myY].hit = true;
                    hitsTaken++;

                    if(hitsTaken == 9) {
                        bzero(buffer, 256);
                        strcpy(buffer, "dead");
                        write(connfd,buffer,256);
                        finish = true;
                    }

                    else {
                        bzero(buffer, 256);
                        strcpy(buffer, "hit");
                        write(connfd,buffer,256);
                    }
                }

                else {
                    bzero(buffer, 256);
                    strcpy(buffer, "miss");
                    write(connfd,buffer,256);
                }
            }


            // Respond to basic shot incoming.
            else if(!strcmp(line, "base")) {
                // Read x
                bzero(buffer, 256);
                read(connfd,buffer,256);
                myX = atoi(buffer);

                // Read y
                bzero(buffer, 256);
                read(connfd,buffer,256);
                myY = atoi(buffer);

                // Send hit or miss
                if(mGrid[myX][myY].ship && !mGrid[myX][myY].hit) {
                    mGrid[myX][myY].hit = true;
                    hitsTaken++;

                    if(hitsTaken == 9) {
                        bzero(buffer, 256);
                        strcpy(buffer, "dead");
                        write(connfd,buffer,256);
                        finish = true;
                    }

                    else {
                        bzero(buffer, 256);
                        strcpy(buffer, "hit");
                        write(connfd,buffer,256);
                    }
                }

                else {
                    bzero(buffer, 256);
                    strcpy(buffer, "miss");
                    write(connfd,buffer,256);
                }
            }
            
            // Send shot to client.
            fireShot(eGrid, &use, coord, &count, connfd);
        }
    }
}


// Selects a shot and sends to client.
void fireShot(struct Node grid[L][W], int* use, int coord[6], int* count, int connfd) {
    int x, y;
    srandom(time(NULL));
    
    // Fire random shot
    if((*use) == 0) {
        // Get an x and y
        x = random() % 8 + 0;
        y = random() % 8 + 0;
        
        while(grid[x][y].hit) {
            srand(time(NULL));
            x = random() % 8 + 0;
            y = random() % 8 + 0;
        }
        
        // Send x
        bzero(buffer, 256);
        sprintf(buffer, "%d", x);
        write(connfd,buffer,256);
        
        // Send y
        bzero(buffer, 256);
        sprintf(buffer, "%d", y);
        write(connfd,buffer,256);
        
        // Read result
        bzero(buffer, 256);
        read(connfd,buffer,256);
        strcpy(line, buffer);
        
        if(!strcmp(line, "dead")) {
            finish = true;
        }
        
        // Act for successful hit. Prepares for next shot.
        else if(!strcmp(line, "hit")) {
            coord[0] = x;
            coord[1] = y;
            
            // Checks if ship is dead
            (*count)++;
            if((*count) == 3) {
                (*use) = 0;
            }
            
            // Fire next shot based on this coordinate
            else {
                (*use) = 1;
            }
        }
        
        // Mark shot as missed
        else {
            grid[x][y].hit = true;
        }
    }
    
    // Bases shot off of last shot
    else if((*use) == 1) {
        x = coord[0];
        y = coord[1];
        
        // Check all indices around last shot, fire at first available.
        if((y - 1) > -1 && !grid[x][y-1].hit) {
            coord[2] = x;
            coord[3] = y - 1;
        }
        
        else if((x + 1) < 9 && !grid[x+1][y].hit) {
            coord[2] = x + 1;
            coord[3] = y;
        }
        
        else if((y + 1) < 9 && !grid[x][y+1].hit) {
            coord[2] = x;
            coord[3] = y + 1;
        }
        
        else if((x - 1) > -1 && !grid[x-1][y].hit) {
            coord[2] = x - 1;
            coord[3] = y;
        }
        
        // If none are available, return to square one and fire random shot next.
        else {
            (*use) = 0;
        }
        
        // Sends current coordinates.
        x = coord[2];
        y = coord[3];
        
        // Send x
        bzero(buffer, 256);
        sprintf(buffer, "%d", x);
        write(connfd,buffer,256);
        
        // Send y
        bzero(buffer, 256);
        sprintf(buffer, "%d", y);
        write(connfd,buffer,256);
        
        // Read result
        bzero(buffer, 256);
        read(connfd,buffer,256);
        strcpy(line, buffer);
        
        if(!strcmp(line, "dead")) {
            finish = true;
        }
        
        // If hit, move to next spot and shoot from there.
        else if(!strcmp(line, "hit")) {
            grid[x][y].hit = true;
            coord[2] = x;
            coord[3] = y;
            
            // Check if ship is dead.
            (*count)++;
            if((*count) == 3) {
                (*use) = 0;
            }
            
            // Move to nex index.
            else 
                (*use) = 2;
        }
        
        else {
            grid[x][y].hit = true;
        }
        
        
    }
    
    else if((*use) == 2) {
        x = coord[2];
        y = coord[3];
        
        // Search around coordinate and fire at available space.
        if((y - 1) > -1 && !grid[x][y-1].hit) {
            coord[4] = x;
            coord[5] = y - 1;
        }
        
        else if((x + 1) < 9 && !grid[x+1][y].hit) {
            coord[4] = x + 1;
            coord[5] = y;
        }
        
        else if((y + 1) < 9 && !grid[x][y+1].hit) {
            coord[4] = x;
            coord[5] = y + 1;
        }
        
        else if((x - 1) > -1 && !grid[x-1][y].hit) {
            coord[4] = x - 1;
            coord[5] = y;
        }
        
        // If none available, fire from previous spot.
        else {
            (*use) = 1;
            x = coord[0];
            y = coord[1];

            if((y - 1) > -1 && !grid[x][y-1].hit) {
                coord[2] = x;
                coord[3] = y - 1;
            }

            else if((x + 1) < 9 && !grid[x+1][y].hit) {
                coord[2] = x + 1;
                coord[3] = y;
            }

            else if((y + 1) < 9 && !grid[x][y+1].hit) {
                coord[2] = x;
                coord[3] = y + 1;
            }

            else if((x - 1) > -1 && !grid[x-1][y].hit) {
                coord[2] = x - 1;
                coord[3] = y;
            }
            x = coord[2];
            y = coord[3];
        }
        
        if((*use) == 2) {
            x = coord[4];
            y = coord[5];
        }
        
        // Send coordinates
        // Send x
        bzero(buffer, 256);
        sprintf(buffer, "%d", x);
        write(connfd,buffer,256);
        
        // Send y
        bzero(buffer, 256);
        sprintf(buffer, "%d", y);
        write(connfd,buffer,256);
        
        // Read result
        bzero(buffer, 256);
        read(connfd,buffer,256);
        strcpy(line, buffer);
        
        // Check result
        if(!strcmp(line, "dead")) {
            finish = true;
        }
        
        // If hit, move back to previous iteration in case of middle shot.
        else if(!strcmp(line, "hit")) {
            grid[x][y].hit = true;
            
            // Check if client is dead.
            (*count)++;
            if((*count) == 3) {
                (*use) = 0;
            }
            
            else 
                (*use) = 1;
        }
        
        else {
            grid[x][y].hit = true;
        }
    }
}


// Thread code
void *thread(void *vargp) {
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    server(connfd);
    close(connfd);
}
