#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

// Global variables
const int L = 9;
const int W = 9;
char line[256];
char buffer[256];
bool two = false;
bool init = true;

// Node definition
struct Node {
    bool hit;
    bool shot;
    bool ship;
};

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// Function definitions
void client(int);
void PrintGrid(struct Node[L][W]);
void PlaceShips(struct Node[L][W], int);
void GetCoords(struct Node[L][W], int*, int*, int*, int*, int);
bool Validate(struct Node[L][W], int, int, int, int, int);


// Main
int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    // Get port no.
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    // Get host name
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // Assign server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    
    // Assign portno. to server address
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    client(sockfd);
    
    printf("\nConnection closed.\n");
    return 0;
}


void client(int sockfd) {
    struct Node mGrid[L][W];
    struct Node eGrid[L][W];
    
    int choice = 0;
    int hitsTaken = 0;
    int myX, myY = 0;
    int turns = 0;
    bool finish = false;
    
    // Initialize all nodes to false
    for(int i = 0; i < L; i++) {
        for(int j = 0; j < W; j++) {
            mGrid[i][j].hit = false;
            mGrid[i][j].shot = false;
            mGrid[i][j].ship = false;
            
            eGrid[i][j].hit = false;
            eGrid[i][j].shot = false;
            eGrid[i][j].ship = false;
        }
    }
    
    printf("\nHello, Welcome to BattleBoats!\n\n");
    printf("Here is the key:\n");
    printf("# = Boat part\n");
    printf("- = Empty or Undiscovered water\n");
    printf("0 = Missed shot\n");
    printf("X = Hit shot\n");
    
    // Place ships
    printf("\nWhat would you like to do?\n(1) Use preset fleet\n(2) Enter custom cleet\n\nChoice: ");
    
    bzero(line, 256);
    fgets(line, 256, stdin);
    choice = atoi(line);
    
    while((choice != 1) && (choice != 2)) {
        printf("\nInvalid Choice.\n\n");
        printf("\nWhat would you like to do?\n(1) Use preset fleet\n(2) Enter custom cleet\n\nChoice: ");
        bzero(line, 256);
        fgets(line, 256, stdin);
        choice = atoi(line);
    }
    
    // Call place ships function
    PlaceShips(mGrid, choice);
    fflush(stdin);
    printf("\n\nYour fleet:\n");
    PrintGrid(mGrid);
    
    
    while(!finish) {
        // Print menu
        printf("\n-----------------------------------\nKnown Enemy Waters:\n");
        PrintGrid(eGrid);
        printf("\nWhat would you like to do?\n(1) Fire a shot\n(2) View your fleet\n");
        
        // Check if missile is ready
        if(turns == 3) {
            printf("(3) Fire tomahawk missile\n");
        }
        
        else {
            printf("(3) Tomahawk missile ready in %d turn(s)\n", (3 - turns));
        }
        
        printf("(4) Quit\n\nChoice: ");
        
        // Get menu choice input
        bzero(line, 256);
        if(two && init) {
            fgets(line, 256, stdin);
            init = false;
        }
        fgets(line, 256, stdin);
        choice = atoi(line);

        // Validate menu choice for if tomahawk is ready
        if(turns == 3) {
            while((choice != 1) && (choice != 2) && (choice != 3) && (choice != 4)) {
                printf("\nInvalid Choice.\n\n");
                printf("What would you like to do?\n(1) Fire a shot\n");
                printf("(2) View your fleet\n(3) Fire tomahawk missile\n(4) Quit\n\nChoice: ");
                bzero(line, 256);
                fgets(line, 256, stdin);
                choice = atoi(line);
            }
        }
        
        // Validate menu choice for if tomahawk is not ready
        else {
            while((choice != 1) && (choice != 2) && (choice != 4)) {
                printf("\nInvalid Choice.\n\n");
                printf("What would you like to do?\n(1) Fire a shot\n");
                printf("(2) View your fleet\n(3) Tomahawk missile ready in %d turn(s)\n(4) Quit\n\nChoice: ", (3 - turns));
                bzero(line, 256);
                fgets(line, 256, stdin);
                choice = atoi(line);
            }   
        }


        // Enter menu choice one: fire missile
        if(choice == 1) {
            // Get x coordinate
            printf("\nSelect X coordinate: ");
            bzero(line, 256);
            fgets(line, 256, stdin);
            myY = atoi(line);

            // Validate menu choice
            while(!((myY > 0) && (myY < 10))) {
                printf("\nInvalid Choice.\n\n");
                printf("Select X coordinate: ");
                bzero(line, 256);
                fgets(line, 256, stdin);
                myY = atoi(line);
            }

            // Get y coordinate
            printf("Select Y coordinate: ");
            bzero(line, 256);
            fgets(line, 256, stdin);
            myX = atoi(line);

            // Validate menu choice
            while(!((myY > 0) && (myY < 10))) {
                printf("\nInvalid Choice.\n\n");
                printf("Select Y coordinate: ");
                bzero(line, 256);
                fgets(line, 256, stdin);
                myY = atoi(line);
            }
            myX--;
            myY--;
            
            // Write non-tomahawk shot
            bzero(buffer, 256);
            strcpy(buffer, "base");
            write(sockfd,buffer,256);
            
            // Send x
            bzero(buffer, 256);
            sprintf(buffer, "%d", myX);
            write(sockfd,buffer,256);

            // Send y
            bzero(buffer, 256);
            sprintf(buffer, "%d", myY);
            write(sockfd,buffer,256);

            // Recieve message
            bzero(buffer, 256);
            bzero(line, 256);
            read(sockfd,buffer,256);
            strcpy(line, buffer);

            // Print if hit or not, with delay
            printf("\n\nMissile Away ");
            for(int i = 0; i < 3; i++) {
                fflush(stdout);
                sleep(1);
                printf(".");
            }

            // Print if hit was successful
            if((!strcmp(line, "hit"))) {
                eGrid[myX][myY].hit = true;
                printf("  BOOM!");
                printf("\n\n        Successful hit at (%d, %d)", myY + 1, myX + 1);
            }
            
            // End program if enemy is dead
            else if((!strcmp(line, "dead"))) {
                printf("\n\nYOU WON --- CONGRATS!\n\n");
                finish = true;
            }
            
            // Print if shot missed
            else {
                printf("\n\n        Miss at (%d, %d)", myY + 1, myX + 1);
                eGrid[myX][myY].shot = true;
            }

        }

        
        // Choice two: print current ships
        if(choice == 2) {
            printf("\n\nYour fleet:\n");
            PrintGrid(mGrid);
            continue;
        }
        
        // Choice three: Fire tomahawk missile
        if(choice == 3) {
            turns = 0;
            // Get x coordinate
            printf("\nTomahawk missle selected. Missile will fire in cross pattern.");
            printf("\nSelect X coordinate of center: ");
            bzero(line, 256);
            fgets(line, 256, stdin);
            myY = atoi(line);

            // Validate menu choice
            while(!((myY > 0) && (myY < 10))) {
                printf("\nInvalid Choice.\n\n");
                printf("Select X coordinate of center: ");
                bzero(line, 256);
                fgets(line, 256, stdin);
                myY = atoi(line);
            }

            // Get y coordinate
            printf("Select Y coordinate of center: ");
            bzero(line, 256);
            fgets(line, 256, stdin);
            myX = atoi(line);

            // Validate menu choice
            while(!((myY > 0) && (myY < 10))) {
                printf("\nInvalid Choice.\n\n");
                printf("Select Y coordinate of center: ");
                bzero(line, 256);
                fgets(line, 256, stdin);
                myY = atoi(line);
            }
            myX--;
            myY--;
            
            // Send hawk
            bzero(buffer, 256);
            strcpy(buffer, "hawk");
            write(sockfd,buffer,256);
            
            // Send x
            bzero(buffer, 256);
            sprintf(buffer, "%d", myX);
            write(sockfd,buffer,256);

            // Send y
            bzero(buffer, 256);
            sprintf(buffer, "%d", myY);
            write(sockfd,buffer,256);
            
            // Print if hit or not
            printf("\nMissile Away ");
            for(int i = 0; i < 3; i++) {
                fflush(stdout);   // flush the output buffer to ensure that the dots are printed immediately
                sleep(1);
                printf(".");
                
            }

            // Recieve message
            for(int i = 0; i < 5; i++) {
                bzero(buffer, 256);
                bzero(line, 256);
                read(sockfd,buffer,256);
                strcpy(line, buffer);
                printf("\n%s", line);
                
                // Print if hit was successful and update enemy grid.
                if((!strcmp(line, "hit"))) {
                    if((myX + 1) < 9 && i == 0) {
                        eGrid[myX + 1][myY].hit = true;
                    }
                    
                    else if((myX - 1) > -1 && i == 1) {
                        eGrid[myX - 1][myY].hit = true;
                    }
                    
                    else if((myY + 1) < 9 && i == 2) {
                        eGrid[myX][myY + 1].hit = true;
                    }
                    
                    else if((myY - 1) > -1 && i == 3) {
                        eGrid[myX][myY - 1].hit = true;
                    }
                    
                    else {
                        eGrid[myX][myY].hit = true;
                    }
                    
                    printf("  BOOM!\n");   
                }
                
                // Print if hit missed and update enemy grid
                else if((!strcmp(line, "miss"))) {
                    if((myX + 1) < 9 && i == 0) {
                        eGrid[myX + 1][myY].shot = true;
                    }
                    
                    else if((myX - 1) > -1 && i == 1) {
                        eGrid[myX - 1][myY].shot = true;
                    }
                    
                    else if((myY + 1) < 9 && i == 2) {
                        eGrid[myX][myY + 1].shot = true;
                    }
                    
                    else if((myY - 1) > -1 && i == 3) {
                        eGrid[myX][myY - 1].shot = true;
                    }
                    
                    else {
                        eGrid[myX][myY].shot = true;
                    }
                }
            }
            
            // End program if enemy is dead
            if((!strcmp(line, "dead"))) {
                printf("  BOOM!\n");
                printf("\n\nYOU WON --- CONGRATS!\n\n");
                finish = true;
            }
        }
        
        // Choice four: end program
        if(choice == 4) {
            finish = true;
        }
        
        
        // Start code to recieve enemy shot
        if(!finish) {
            // Read x
            bzero(buffer, 256);
            read(sockfd,buffer,256);
            myX = atoi(buffer);

            // Read y
            bzero(buffer, 256);
            read(sockfd,buffer,256);
            myY = atoi(buffer);

            printf("\n\nEnemy fired at (%d, %d)", myY + 1, myX + 1);

            // Send result to cpu
            if(mGrid[myX][myY].ship && !mGrid[myX][myY].hit) {
                mGrid[myX][myY].hit = true;
                hitsTaken++;

                // Check if player has died and send to server.
                if(hitsTaken == 9) {
                    bzero(buffer, 256);
                    strcpy(buffer, "dead");
                    write(sockfd,buffer,256);
                    printf("\n\nGAME OVER --- YOU LOSE\n\n");
                    finish = true;
                }

                // If hit, print that enemy hit and send to server.
                else {
                    printf(" and hit!\n");
                    bzero(buffer, 256);
                    strcpy(buffer, "hit");
                    write(sockfd,buffer,256);
                    //printf("\nSent: %s\n", buffer); //
                }
            }

            // Print that enemy missed and send to server.
            else {
                printf(" and missed!\n");
                bzero(buffer, 256);
                strcpy(buffer, "miss");
                write(sockfd,buffer,256);
                //printf("\nSent: %s\n", buffer); //
            }
        }
        
        // Update tomahawk counter.
        if(!(turns == 3)) 
            turns++;
    }
}

// Called when player inputs ship locations or chooses preset.
void PlaceShips(struct Node grid[L][W], int choice) {
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    char c;
    
    // Open text file
    FILE* file;
    file = fopen("Presets.txt", "r");
    if (file == NULL) {
        printf("Unable to open file\n");
        return;
    }
    
    // Code for using preset.
    if(choice == 1) {
        printf("\nLook at the text file and choose a preset.\n\n Choice: ");
        bzero(line, 256);
        fgets(line, 256, stdin);
        choice = atoi(line);

        // Validate choice.
        while(!((choice > 0) && (choice < 5))) {
            printf("\nInvalid Choice.\n\n");
            printf("\nEnter fleet choice.\n\nChoice: ");
            bzero(line, 256);
            fgets(line, 256, stdin);
            choice = atoi(line);
        }
        
        // Read selected preset and create map accordingly, checking for "#" character.
        while ((c = fgetc(file)) != EOF) {
            if(c == (choice + '0')) {
                for(int i = 0; i < 9; i++) {
                    for(int j = 0; j < 10; j++) {
                        c = fgetc(file);
                        if(c == '#') {
                            grid[i][j - 1].ship = true;
                        }
                    }
                }
            }
        }
    }
    
    // Code for manual entry. Player enters in ship locations.
    else if(choice == 2) {
        two = true;
        printf("\nPlace 3 ships. Ships are 3 units long.\n");
        PrintGrid(grid);
        
        // Get coordinates for each ship (3)
        for(int i = 0; i < 3; i++) {
            do {
                GetCoords(grid, &x1, &x2, &y1, &y2, (i+1));
            } while(!Validate(grid, x1, x2, y1, y2, (i+1))); // Continue to prompt ship placement while ship is invalid.
            
            // Place ship
            grid[x1][y1].ship = true;
            grid[x2][y2].ship = true;

            // Connect ship in the middle
            if(x1 == x2) {
                if(y2 > y1)
                    grid[x1][y2 - 1].ship = true;
                else if(y2 < y1)
                    grid[x1][y1 - 1].ship = true;
            }

            // Connect ship in the middle
            if(y1 == y2) {
                if(x2 > x1)
                    grid[x2 - 1][y1].ship = true;
                else if(x2 < x1)
                    grid[x1 - 1][y1].ship = true;
            }
                
            // Show new ship after its placed.
            PrintGrid(grid);
        }
    }
}


// Checks if the passed coordinates will place a valid ship.
bool Validate(struct Node grid[L][W], int x1, int x2, int y1, int y2, int i) {
    
    // Checks if ship is on the same x or y plane, aka in a straight line
    if(x1 != x2 && y1 != y2) {
        printf("\nInvalid ship placement. Ship must be a straight line.\n\n");
        return false;
    }

    // Checks ship length on x axis.
    if(x1 == x2) {
        if(y2 > y1) {
            if((y2 - y1) != 2) {
                printf("\nInvalid ship length. Ship must be 3 tiles long.\n");
                return false;
            }
        }
        else if(x2 < x1) {
            if((y1 - y2) != 2) {
                printf("\nInvalid ship length. Ship must be 3 tiles long.\n");
                return false;
            }
        }
    }

    // Checks ship length on y axis.
    if(y1 == y2) {
        if(x2 > x1) {
            if((x2 - x1) != 2) {
                printf("\nInvalid ship length. Ship must be 3 tiles long.\n");
                return false;
            }
        }
        else if(x2 < y1) {
            if((x1 - x2) != 2) {
                printf("\nInvalid ship length. Ship must be 3 tiles long.\n");
                return false;
            }
        }
    }

    // Check for intersections on x axis
    if(x1 == x2) {
        if(y2 > y1)
            if(grid[x1][y2 - 1].ship) {
                printf("\nInvalid ship placement. Ships must not intersect.\n\n");
                return false;
            }
            else if(y2 < y1)
                if(grid[x1][y1 - 1].ship) {
                    printf("\nInvalid ship placement. Ships must not intersect.\n\n");
                    return false;
                }
    }

    // Check for intersections on y axis
    if(y1 == y2) {
        if(x2 > x1)
            if(grid[x2 - 1][y1].ship) {
                printf("\nInvalid ship placement. Ships must not intersect.\n\n");
                return false;
            }
        else if(x2 < x1)
            if(grid[x1 - 1][y1].ship) {
                printf("\nInvalid ship placement. Ships must not intersect.\n\n");
                return false;
            }
    }
    
    // returns valid if all checks were passed.
    return true;
}



void GetCoords(struct Node grid[L][W], int* x1, int* x2, int* y1, int* y2, int num) {
    // Get head of ship
    printf("\nEnter X coordinate for FRONT of ship %d: ", num);
    scanf("%d", &(*y1));
    while(!((*y1 > 0) && (*y1 < 10))) {
        printf("\nInvalid. Enter a new one: ");
        scanf("%d", &(*y1));
    }
    *y1 = *y1 - 1;

    printf("Enter Y coordinate for FRONT of ship %d: ", num);
    scanf("%d", &(*x1));
    while(!((*x1 > 0) && (*x1 < 10))) {
        printf("\nInvalid. Enter a new one: ");
        scanf("%d", &(*x1));
    }
    *x1 = *x1 - 1;

    // Check if coordinate entered is valid.
    while(grid[*x1][*y1].ship) {
        printf("\nThere is already a ship there.\n\n");
        printf("Enter X coordinate for FRONT of ship %d: ", num);
        scanf("%d", &(*y1));
        while(!((*y1 > 0) && (*y1 < 10))) {
            printf("\nInvalid. Enter a new one: ");
            scanf("%d", &(*y1));
        }
        *y1 = *y1 - 1;

        printf("Enter Y coordinate for FRONT of ship %d: ", num);
        scanf("%d", &(*x1));
        while(!((*x1 > 0) && (*x1 < 10))) {
            printf("\nInvalid. Enter a new one: ");
            scanf("%d", &(*x1));
        }
        *x1 = *x1 - 1;
    }

    // Get tail
    printf("\nEnter X coordinate for BACK of ship %d: ", num);
    scanf("%d", &(*y2));
    while(!((*y2 > 0) && (*y2 < 10))) {
        printf("\nInvalid. Enter a new one: ");
        scanf("%d", &(*y2));
    }
    *y2 = *y2 - 1;

    printf("Enter Y coordinate for BACK of ship %d: ", num);
    scanf("%d", &(*x2));
    while(!((*x2 > 0) && (*x2 < 10))) {
        printf("\nInvalid. Enter a new one: ");
        scanf("%d", &(*x2));
    }
    *x2 = *x2 - 1;

    // Check if coordinate entered is valid.
    while(grid[*x2][*y2].ship) {
        printf("\nThere is already a ship there.\n\n");
        printf("Enter X coordinate for BACK of ship %d: ", num);
        scanf("%d", &(*y2));
        while(!((*y2 > 0) && (*y2 < 10))) {
            printf("\nInvalid. Enter a new one: ");
            scanf("%d", &(*y2));
        }
        *y2 = *y2 - 1;

        printf("Enter Y coordinate for BACK of ship %d: ", num);
        scanf("%d", &(*x2));
        while(!((*x2 > 0) && (*x2 < 10))) {
            printf("\nInvalid. Enter a new one: ");
            scanf("%d", &(*x2));
        }
        *x2 = *x2 - 1;
    }
    
}


// Print the grid with appropriate symbols representing ship state.
void PrintGrid(struct Node grid[L][W]) {
    printf("\n  ");
    // Print x axis numbers.
    for(int i = 0; i <L; i++) {
        printf("%d ", (i+1));
    }
    printf("x \n");
    
    for(int i = 0; i < L; i++) {
        // Print y axis numbers.
        printf("%d ", (i+1));
        
        for(int j = 0; j < W; j++) {
            if(!grid[i][j].hit && !grid[i][j].shot && !grid[i][j].ship) {
                printf("- ");
            }
            else if(grid[i][j].hit && grid[i][j].shot && !grid[i][j].ship) {
                printf("X ");
            }
            else if(!grid[i][j].hit && grid[i][j].shot && !grid[i][j].ship) {
                printf("0 ");
            }
            else if(grid[i][j].hit && !grid[i][j].shot && !grid[i][j].ship) {
                printf("X ");
            }
            else if(!grid[i][j].hit && !grid[i][j].shot && grid[i][j].ship) {
                printf("# ");
            }
            else if(grid[i][j].hit && !grid[i][j].shot && grid[i][j].ship) {
                printf("X ");
            }
        }
        printf("\n");
    }
    printf("y\n");
}


