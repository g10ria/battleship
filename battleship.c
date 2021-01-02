#include "./headers/battleship.h"

/**
 * Constants
 * 
 * MAX_CONFIGS_TESTED = max # of configs to test in each round of calculation
 * DEBUG = set to 1 to print debug messages
 */ 
float MAX_CONFIGS_TESTED = 5;
int DEBUG = 0;

#define BOARD_SIDELENGTH 10
#define BOARD_PADDING 4

/**
 * Current board status
 * Extra 4 spaces of padding on each side make calculation logic simpler
 * 
 * 0 = padding square
 * 1 = unguessed
 * 2 = missed
 * 3 = hit, not on a sunk ship
 * 4 = hit, on a sunk ship
 */ 
int S[BOARD_SIDELENGTH + 2*BOARD_PADDING][BOARD_SIDELENGTH + 2*BOARD_PADDING];

float remainingConfigs = 30093975536; // obtained from online

int sunken[5]; // set to 1 if ship is sunk
// ship  order: 2, 3, 3, 4, 5
int numShipsSunk;

int shipConfigs[5][200]; // stores the valid ship orientations
// stores a number formatted as such: spot index * 10 + orientation
// spot index is a number from 0 to 99
// orientation is 0 or 1, depending on top or right orientation
int numShipConfigs[5]; // stores the # of valid ship orientations for each ship

int currConfig[5][3]; // stores a configuration for testing
// For each ship, first two values represent x and y coordinates and
// third value represents direction (to the right or to the bottom)

int probs[100][5][2];

char initialValue = '\0';
struct entry* shipCollisionMap; // for storing if two ships intersect
struct entry* shipPositionFrequencyMap;

// DEBUG VARIABLES
// asldfjdask

void init(void);
int generateMove(void); // returns an integer in [0, 99]

void resetProbs(void); // resets probability array to 0s

void generateShipConfigs(void); // generates all valid configs for each ship
void determineShipCollisions(void); // determines if two ship configs will collide
// does this for all pairs of ship configs
int shipConfigsCollide(int, int, int, int); // if two ship configs collide

void generateConfig(void); // generates a new configuration without testing validation
int validConfig(int[5]);      // tests if the current configuration is valid
void incrementProbs(void); // increments the corresponding probabilities for the current configuration

int shipLengthFromIndex(int);

void printWelcomeScreen();
void promptShipInput();
void printPreviousGuesses();
void printBoard();
void promptInput();

void testCollide(int x1, int y1, int x2, int y2, int s1, int s2, int o1, int o2) {

    int ship1Config = y1 * 100 + x1*10 + o1;
    int ship2Config = y2*100 + x2*10 + o2;
    int keyVal = s1 * 10000000 + s2 * 1000000 + ship1Config*100 + ship2Config;
    printf("%d \n",get(keyVal, shipCollisionMap));
    
    return;
}

int main()
{
    init();

    printWelcomeScreen();
    
    promptShipInput();

    printPreviousGuesses();
    printBoard();
    
    promptInput();

    // clock_t start, stop;
    // start = clock();
    // stop = clock();
    // printf("\nstart: %6.3f\n", start);
    // printf("stop: %6.3f", stop);
    

    // UI stuff to call functions

    return 0;
}

void printWelcomeScreen() {
    printf("WELCOME TO BATTLESHIP");
}

void promptShipInput() {
    printf("Enter the coordinates of your first ship (length = 2)");
}

void printPreviousGuesses() {
    
}

void promptInput() {
    printf("PRESS 1 TO SEE NEXT GUESS\nPRESS 2 TO INPUT SHIP SINKAGE\n");
}

void printBoard() {
    printf("-----BOARD STATUS-----\n\n");

    for(int x = BOARD_PADDING + BOARD_SIDELENGTH - 1; x>= BOARD_PADDING; x--) {
        printf(" %-3d", x - BOARD_PADDING + 1);
        for(int y = BOARD_PADDING; y < BOARD_PADDING + BOARD_SIDELENGTH; y++) {
            switch (S[x][y]) {
                case 1:
                    printf("%c ", '-');
                    break;
                case 2:
                    printf("%c ", 'X');
                    break;
                case 3:
                    printf("%c ", 'O');
                    break;
                case 4:
                    printf("%c ", 'S');
                    break;
            }
        }
        printf("\n");
    }

    printf("    ");
    for(int x = 1; x<= BOARD_SIDELENGTH;x++) {
        printf("%d ", x);
    }
    printf("\n\n");
}

int shipLengthFromIndex(int i) {
    switch (i) {
        case 0: return 2;
        case 1: return 3;
        case 2: return 3;
        case 3: return 4;
        case 4: return 5;
    }
    printf("%d \n", i);
    printf("Something has gone terribly wrong...\n");
    return 0;
}

void generateShipConfigs(void) {
    if (DEBUG) {
        printf("Generating ship configs...\n");
    }

    // resetting numShipConfigs
    for(int i=0;i<5;i++) numShipConfigs[i] = 0;
    // ignores if ships have already been sunk (accounted for later)
    for(int x=BOARD_PADDING;x<BOARD_SIDELENGTH + BOARD_PADDING;x++) {
        for(int y=BOARD_PADDING;y<BOARD_SIDELENGTH + BOARD_PADDING;y++) {
            // for each spot on the board

            int indexMultiplied = ((y-BOARD_PADDING)*10 + x-BOARD_PADDING) * 10; // index of the space in 0-99, *10

            // testing 5 ship
            /* up */    if ((S[x][y] * S[x][y+1] * S[x][y+2] * S[x][y+3] * S[x][y+4]) % 2 == 1) {
                            shipConfigs[4][numShipConfigs[4]] = indexMultiplied + 0;
                            numShipConfigs[4]++;
                        }
            /* right */ if ((S[x][y] * S[x+1][y] * S[x+2][y] * S[x+3][y] * S[x+4][y]) % 2 == 1) {
                            shipConfigs[4][numShipConfigs[4]] = indexMultiplied + 1;
                            numShipConfigs[4]++;
                        }

            // testing 4 ship
            /* up */    if ((S[x][y] * S[x][y+1] * S[x][y+2] * S[x][y+3]) % 2 == 1) {
                            shipConfigs[3][numShipConfigs[3]] = indexMultiplied + 0;
                            numShipConfigs[3]++;
                        }
            /* right */ if ((S[x][y] * S[x+1][y] * S[x+2][y] * S[x+3][y]) % 2 == 1) {
                            shipConfigs[3][numShipConfigs[3]] = indexMultiplied + 1;
                            numShipConfigs[3]++;
                        }
            
            // testing 3 ships
            /* up */    if ((S[x][y] * S[x][y+1] * S[x][y+2]) % 2 == 1) {
                            // printf("up %d %d %d setting %d\n", y-4, x-4, indexMultiplied + 0, numShipConfigs[2]);

                            shipConfigs[2][numShipConfigs[2]] = indexMultiplied + 0;
                            shipConfigs[1][numShipConfigs[1]] = indexMultiplied + 0;
                            numShipConfigs[2]++;
                            numShipConfigs[1]++;

                        }
            /* right */ if ((S[x][y] * S[x+1][y] * S[x+2][y]) % 2 == 1) {
                            // printf("right %d %d %d setting %d\n", y-4, x-4, indexMultiplied + 1, numShipConfigs[2]);
                            
                            shipConfigs[2][numShipConfigs[2]] = indexMultiplied + 1;
                            shipConfigs[1][numShipConfigs[1]] = indexMultiplied + 1;
                            numShipConfigs[2]++;
                            numShipConfigs[1]++;
                        }

            // testing 2 ship
            /* up */    if ((S[x][y] * S[x][y+1]) % 2 == 1) {
                            shipConfigs[0][numShipConfigs[0]] = indexMultiplied + 0;
                            numShipConfigs[0]++;
                        }
            /* right */ if ((S[x][y] * S[x+1][y]) % 2 == 1) {
                            shipConfigs[0][numShipConfigs[0]] = indexMultiplied + 1;
                            numShipConfigs[0]++;
                        }
        }
    }

    if (DEBUG==1) {
        printf("Printing # of valid ship configs\n");
        for(int i=0;i<5;i++) {
            printf("%d %d\n", i, numShipConfigs[i]);
        }
    }
}

void determineShipCollisions(void) {

    for(int s1=0;s1<5;s1++) { // ship 1
        for(int s2 = s1+1;s2<5;s2++) { // ship 2
            // printf("Comparing ship %d and ship %d\n", s1, s2);
            for(int c1=0;c1<numShipConfigs[s1];c1++) { // iterate through ship 1 configs
                for(int c2 = 0;c2<numShipConfigs[s2];c2++) { // iterate through ship 2 configs
                    
                    int ship1Config = shipConfigs[s1][c1];
                    int ship2Config = shipConfigs[s2][c2];

                    if (shipConfigsCollide(s1, s2, ship1Config, ship2Config)==1) {                
                        // add this to the hashmap for collisions
                        int keyVal = s1 * 10000000 + s2 * 1000000 + ship1Config*100 + ship2Config;
                        put(keyVal, 67, shipCollisionMap);
                    }
                }
            }
        }
    }
}

int shipConfigsCollide(int s1, int s2, int c1, int c2) {

    int x1 = (c1 / 10) % 10;
    int y1 = c1 / 100;
    int x2 = (c2 / 10) % 10;
    int y2 = c2 / 100;

    int o1 = c1 % 10;
    int o2 = c2 % 10; // orientations, 0 (up) or 1 (right)

    int ship1Length = shipLengthFromIndex(s1);
    int ship2Length = shipLengthFromIndex(s2);

    for(int l1 = 0; l1<ship1Length; l1++) { // compare each location of ship 1
        for(int l2 = 0; l2<ship2Length; l2++) { // to each location of ship 2
            
            int ship1Location = o1==1 ? y1*10+x1+l1 : (y1+l1)*10+x1;
            int ship2Location = o2==1 ? y2*10+x2+l2 : (y2+l2)*10+x2;
            // printf("Comparing locations %d and %d\n", ship1Location, ship2Location);

            if (ship1Location == ship2Location) return 1;
        }
    }

    return 0;
}

int generateMove(void)
{
    // resetProbs();
    shipCollisionMap = initializeHashmap();
    shipPositionFrequencyMap = initializeHashmap();

    generateShipConfigs();
    determineShipCollisions();

    for(float i=0;i<MAX_CONFIGS_TESTED;i++) {

        int testedShipConfigs[5]; // randomly selected ship configs
        for(int i=0;i<5;i++) {
            if (!sunken[i]) {
                testedShipConfigs[i] = shipConfigs[i][rand() % numShipConfigs[i]];
            }
        }

        if(validConfig(testedShipConfigs)) {
            // add stuff to arrays
        }

    }

    // calculating the best move
    // int bestMove = 0;
    // double bestProb = 0.0;

    // for(int i=0;i<MAX_CONFIGS_TESTED) {

    // }

    // for (int i = 0; i < 100; i++)
    // {
    //     double prob = 0;
    //     for (int j = 0; j < 5; j++)
    //     {
    //         // add to prob to calculate the average probability
    //         // also check if the values are -1; if so, skip this square
    //     }
    //     prob /= (5.0 - (double)numShipsSunk);
    //     if (prob > bestProb)
    //     {
    //         bestProb = prob;
    //         bestMove = i;
    //     }
    // }

    free(shipCollisionMap);

    return 1;
}

void init()
{
    srand(time(0));
    // Initialize all UI stuff

    // set all padding squares to 0 (padding)
    for(int x=0;x<BOARD_SIDELENGTH+2*BOARD_PADDING;x++) {
        for(int y = 0;y<BOARD_SIDELENGTH+2*BOARD_PADDING;y++) {
            S[x][y] = 0;
        }
    }
    // set all board squares to 1 (unguessed)
    for(int x=BOARD_PADDING;x<BOARD_SIDELENGTH+BOARD_PADDING;x++) {
        for(int y=BOARD_PADDING;y<BOARD_SIDELENGTH+BOARD_PADDING;y++) {
            S[x][y] = 1;
        }
    }
}

void resetProbs()
{
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            for (int k = 0; k < 2; k++)
                probs[i][j][k] = -1;
        }
    }
}

int validConfig(int shipConfigs[5])
{
    // Things to test:
    // 1. no ship intersections (use the map)
    // 2. all 'hit' squares are covered

    // make sure to account for already-sunk ships
}