#include "./headers/battleship.h"

/**
 * Battleship guesser written for Info Theory (20-21)
 * @created 12/21/20
 * 
 * Todos [low priority]:
 * - Add options for different ship quantities/sizes
 * - Add input validation for ship sinkage prompt
 * - Add cmd line flags for program macros/constants
 * - Change MAX_CONFIGS_TESTED to a time constraint instead
 */

/* ----- MACROS ----- */

#define BOARD_SIDELENGTH 10 // side length of square battleship board. MAX 10
#define BOARD_PADDING 4     // amount to pad on each side. should be equal to
// the longest ship's length - 1
#define DEBUG 1 // set to 1 to print debug messages, 0 otherwise

/* ----- GLOBAL VARIABLES ----- */

// max # of configs to test in each round of calculation
int MAX_CONFIGS_TESTED = 10000000;

/**
 * Board status
 * 0 = padding square
 * 1 = unguessed
 * 2 = missed
 * 3 = hit, not on a sunk ship
 * 4 = hit, on a sunk ship
 */
int S[BOARD_SIDELENGTH + 2 * BOARD_PADDING][BOARD_SIDELENGTH + 2 * BOARD_PADDING];

// keeps track of which ships are sunk
// ship order: 2,3,3,4,5
int sunken[5];
// keeps track of where sunken ships are
int sunkenLocations[5];

// stores the valid ship orientations (sort of like a list)
// stores numbers formatted as such: spot index * 10 + orientation
// spot index is a number from 0 to 99
// orientation is 0 or 1, depending on up or right orientation
int shipConfigs[5][200];
// stores the # of valid ship orientations for each ship
int numShipConfigs[5];

// an int-int map that stores if two ships will intersect in a specific configuration
struct entry *shipCollisionMap;

// an int-int map that stores the frequency of each ship position occuring
// given the remaining board configurations possible
struct entry *shipPositionFrequencyMap;

// stores the current # of guesses
int numGuesses;

/* ----- FUNCTION DECLARATIONS ----- */

// Initializes important variables, memory, etc.
void init(void);

/* UI FUNCTIONS */

// plays a game of battleship until forfeit or win
void playGame();
// Prints the welcome screen
int printWelcomeScreen();
// Prints the current board status
void printBoard(int board[BOARD_SIDELENGTH + 2 * BOARD_PADDING][BOARD_SIDELENGTH + 2 * BOARD_PADDING]);
// Prompts for input from the user
int promptInput();

void promptGuess();

void promptShipSinkage();

/* MOVE GENERATION FUNCTIONS */

// Overarching move generation function; returns an integer in [0,99]
int generateMove(void);
// Generates all valid configurations for each ship
void generateShipConfigs(void);
// Determines for all pairs of ship configs if the ships will collide
void determineShipCollisions(void);
// Returns if two ship configs collide (uses results from determineShipCollisions)
int shipConfigsCollide(int, int, int, int);
// tests if the given ship configuration is possible (given current board status)
int validConfig(int[5]);
// randomly tests MAX_CONFIGS_TESTED configs
int randomlyTestConfigs();
// brute force tests all possible configs
int bruteForceTestConfigs();
// calculates and returns the best move after all ship frequencies have been determined
int calculateBestMove(int);

/* HELPER/DEBUG FUNCTIONS */

// returns a ship's length given its index
// in order: 0,1,2,3,4 --> 2,3,3,4,5
int shipLengthFromIndex(int);
// Meant for testing generateShipConfigs and determineShipCollisions
// Retrieves from determineShipCollision's map if two ships collide
void testCollide(int, int, int, int, int, int, int, int);

/* ----- CODE ----- */

/**
 * Prints the welcome screen. According to the player's action,
 * plays the game or quits.
 */
int main()
{
    int inp1 = printWelcomeScreen();

    if (inp1 == 1)
    {
        playGame();
    }
    else if (inp1 == 2)
    {
        return 0;
    }
    else
    {
        printf("There was an error.\n");
        return 1;
    }

    return 0;
}

/**
 * Initializes necessary variables for the program
 */
void init()
{
    // srand(time(0));
    init_genrand(time(0));
    numGuesses = 0;

    // set all padding squares to 0 (padding)
    for (int x = 0; x < BOARD_SIDELENGTH + 2 * BOARD_PADDING; x++)
    {
        for (int y = 0; y < BOARD_SIDELENGTH + 2 * BOARD_PADDING; y++)
            S[x][y] = 0;
    }
    // set all board squares to 1 (unguessed)
    for (int x = BOARD_PADDING; x < BOARD_SIDELENGTH + BOARD_PADDING; x++)
    {
        for (int y = BOARD_PADDING; y < BOARD_SIDELENGTH + BOARD_PADDING; y++)
            S[x][y] = 1;
    }
}

/**
 * Determine if the game is won (all ships have been sunk)
 * @return 1 if won, 0 otherwise
 */
int gameOver()
{
    for (int s = 0; s < 5; s++)
        if (!sunken[s])
            return 0;
    return 1;
}

/**
 * Plays the game until the player quits or the game is won
 */
void playGame()
{
    init();

    int quitGame = 0;
    while (!gameOver() && !quitGame)
    {
        printBoard(S);
        printf("Guesses so far: %d\n", numGuesses);
        quitGame = promptInput(); // gets set to 1 when error
    }

    if (quitGame)
    {
        printf("Quit game at %d guesses.\n", numGuesses);
    }
    else
        printf("Game over in %d guesses.\n", numGuesses);

    return;
}

/**
 * Prints the welcome screen for the game
 * @return player's input
 */
int printWelcomeScreen()
{
    printf("\nWELCOME TO BATTLESHIP\n\n");
    printf("Board size: %d x %d\n\n", BOARD_SIDELENGTH, BOARD_SIDELENGTH);
    printf("Press 1 to play new game.\nPress 2 to quit.\n\n");

    int inp;
    scanf(" %d", &inp);

    while (inp != 1 && inp != 2)
    {
        printf("Bad input, try again.\n");
        scanf(" %d", &inp);
    }

    return inp;
}

/**
 * Prints the current board status as well as axis labels
 */
void printBoard(int board[BOARD_SIDELENGTH + 2 * BOARD_PADDING][BOARD_SIDELENGTH + 2 * BOARD_PADDING])
{
    printf("\n-----BOARD STATUS-----\n\n");

    for (int y = BOARD_PADDING + BOARD_SIDELENGTH - 1; y >= BOARD_PADDING; y--)
    {
        printf(" %-3d", y - BOARD_PADDING + 1);
        for (int x = BOARD_PADDING; x < BOARD_PADDING + BOARD_SIDELENGTH; x++)
        {
            // printf("%d ", board[x][y]);
            switch (board[y][x])
            {
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
    for (int x = 1; x <= BOARD_SIDELENGTH; x++)
        printf("%d ", x);
    printf("\n\n");

    return;
}

/**
 * Prompts the user to select what they would like to input next in the game
 * @return 1 if the user quit the game, 0 otherwise
 */
int promptInput()
{
    printf("Press 1 for next guess.\nPress 2 to input ship sinkage.\nPress 3 to quit game.\n\n");

    int inp;
    scanf(" %d", &inp);

    while (inp != 1 && inp != 2 && inp != 3)
    {
        printf("Bad input, try again.\n");
        scanf(" %d", &inp);
    }

    if (inp == 1)
    {
        promptGuess();
    }
    else if (inp == 2)
    {
        promptShipSinkage();
    }
    else if (inp == 3)
    {
        return 1;
    }

    return 0;
}

/**
 * Displays the next guess to the user and asks them if it was a hit or miss
 */
void promptGuess()
{
    numGuesses++;
    // printf("...\n");

    int move;
    // if (numGuesses == 1)
    //     move = 55; // Hardcode first move in (always the same)
    // else
    move = generateMove();

    // Print the guess coordinates
    printf("\nGuess %d: <%d, %d>\n", numGuesses, move % 10 + 1, move / 10 + 1);
    printf("Enter 1 for hit.\nEnter 2 for miss.\n");

    int inp;
    scanf(" %d", &inp);

    while (inp != 1 && inp != 2)
    {
        printf("Bad input, try again.\n");
        scanf(" %d", &inp);
    }

    // Set the square in the status matrix accordingly
    S[BOARD_PADDING + move / 10][BOARD_PADDING + move % 10] = inp == 1 ? 3 : 2;

    return;
}

/**
 * Prompts the user to input information for a sunken ship
 */
void promptShipSinkage()
{
    printf("Which ship was sunk? (Enter a number between 1-5)\n");
    printf("Note: ship order is 2, 3, 3, 4, 5.\n\n");

    int s;
    scanf(" %d", &s);

    while (s < 1 || s > 5 || sunken[s - 1])
    {
        printf("Bad input or that ship has been sunk already, try again.\n");
        scanf(" %d", &s);
    }

    printf("What is the x-coordinate of the ship's left or bottom square?\n");
    int x;
    scanf(" %d", &x); // todo: error checking

    printf("What is the y-coordinate of the ship's left or bottom square?\n");
    int y;
    scanf(" %d", &y); // todo: error checking

    printf("Is the ship facing up or right? Enter 0 for up and 1 for right.\n");
    int o;
    scanf(" %d", &o); // todo: error checking

    // update the sunken arrays
    sunken[s - 1] = 1;
    sunkenLocations[s - 1] = (y - 1) * 100 + (x - 1) * 10 + o;

    // set the square in the status matrix correctly
    int shipLength = shipLengthFromIndex(s - 1);
    if (o == 0)
    { // facing up
        for (int i = 0; i < shipLength; i++)
        {
            S[y + BOARD_PADDING - 1 + i][x + BOARD_PADDING - 1] = 4;
        }
    }
    else if (o == 1)
    { // facing right
        for (int i = 0; i < shipLength; i++)
        {
            S[y + BOARD_PADDING - 1][x + BOARD_PADDING - 1 + i] = 4;
        }
    }

    return;
}

/**
 * Generates all valid configs for each ship on the board
 * Iterates over every single square / every ship / every orientation (up or right)
 * 
 * Because of the way the squares in the status matrix are set up, to see if
 * a ship configuration is valid the squares under the ship need to be
 * all odd numbers (either 1, unguessed, or 3, hit but not on a sunk ship).
 * This guarantees all generated ship configs don't go on missed squares or
 * hit but on a sunk ship squares.
 */
void generateShipConfigs(void)
{
    if (DEBUG)
        printf("Generating ship configs...\n");

    // resetting numShipConfigs array
    for (int i = 0; i < 5; i++)
        numShipConfigs[i] = 0;

    // ignores if ships have already been sunk (accounted for later)
    for (int x = BOARD_PADDING; x < BOARD_SIDELENGTH + BOARD_PADDING; x++)
    {
        for (int y = BOARD_PADDING; y < BOARD_SIDELENGTH + BOARD_PADDING; y++)
        {
            // for each spot on the board

            int indexMultiplied = ((y - BOARD_PADDING) * 10 + x - BOARD_PADDING) * 10; // index of the space in 0-99, *10

            // testing 5 ship
            /* up */ if ((S[y][x] * S[y + 1][x] * S[y + 2][x] * S[y + 3][x] * S[y + 4][x]) % 2 == 1)
            {
                shipConfigs[4][numShipConfigs[4]] = indexMultiplied + 0;
                numShipConfigs[4]++;
            }
            /* right */ if ((S[y][x] * S[y][x + 1] * S[y][x + 2] * S[y][x + 3] * S[y][x + 4]) % 2 == 1)
            {
                shipConfigs[4][numShipConfigs[4]] = indexMultiplied + 1;
                numShipConfigs[4]++;
            }

            // testing 4 ship
            /* up */ if ((S[y][x] * S[y + 1][x] * S[y + 2][x] * S[y + 3][x]) % 2 == 1)
            {
                shipConfigs[3][numShipConfigs[3]] = indexMultiplied + 0;
                numShipConfigs[3]++;
            }
            /* right */ if ((S[y][x] * S[y][x + 1] * S[y][x + 2] * S[y][x + 3]) % 2 == 1)
            {
                shipConfigs[3][numShipConfigs[3]] = indexMultiplied + 1;
                numShipConfigs[3]++;
            }

            // testing 3 ships
            /* up */ if ((S[y][x] * S[y + 1][x] * S[y + 2][x]) % 2 == 1)
            {
                // printf("up %d %d %d setting %d\n", y-4, x-4, indexMultiplied + 0, numShipConfigs[2]);

                shipConfigs[2][numShipConfigs[2]] = indexMultiplied + 0;
                shipConfigs[1][numShipConfigs[1]] = indexMultiplied + 0;
                numShipConfigs[2]++;
                numShipConfigs[1]++;
            }
            /* right */ if ((S[y][x] * S[y][x + 1] * S[y][x + 2]) % 2 == 1)
            {
                // printf("right %d %d %d setting %d\n", y-4, x-4, indexMultiplied + 1, numShipConfigs[2]);

                shipConfigs[2][numShipConfigs[2]] = indexMultiplied + 1;
                shipConfigs[1][numShipConfigs[1]] = indexMultiplied + 1;
                numShipConfigs[2]++;
                numShipConfigs[1]++;
            }

            // testing 2 ship
            /* up */ if ((S[y][x] * S[y + 1][x]) % 2 == 1)
            {
                shipConfigs[0][numShipConfigs[0]] = indexMultiplied + 0;
                numShipConfigs[0]++;
            }
            /* right */ if ((S[y][x] * S[y][x + 1]) % 2 == 1)
            {
                shipConfigs[0][numShipConfigs[0]] = indexMultiplied + 1;
                numShipConfigs[0]++;
            }
        }
    }

    if (DEBUG)
    {
        printf("Printing # of valid ship configs:\n");
        for (int i = 0; i < 5; i++)
        {
            printf("Ship %d: %d config(s)\n", i, numShipConfigs[i]);
        }
    }

    return;
}

/**
 * Determines which ship configurations collide with each other
 * Iterates over each pair of 2 ship configurations and stores
 * setups that collide in a hashmap
 * 
 * This prevents the program from having to test the same pair
 * of configurations over and over again during the board
 * configuration generation phase
 */
void determineShipCollisions(void)
{
    for (int s1 = 0; s1 < 5; s1++)
    { // ship 1
        for (int s2 = s1 + 1; s2 < 5; s2++)
        { // ship 2
            for (int c1 = 0; c1 < numShipConfigs[s1]; c1++)
            { // iterate through ship 1 configs
                for (int c2 = 0; c2 < numShipConfigs[s2]; c2++)
                { // iterate through ship 2 configs

                    int ship1Config = shipConfigs[s1][c1];
                    int ship2Config = shipConfigs[s2][c2];

                    if (shipConfigsCollide(s1, s2, ship1Config, ship2Config))
                    {
                        // add this to the hashmap for collisions
                        int keyVal = s1 * 10000000 + s2 * 1000000 + ship1Config * 100 + ship2Config;
                        put(keyVal, 67, shipCollisionMap);
                    }
                }
            }
        }
    }
    return;
}

/**
 * Returns if two ships collide in a specific configuration
 * 
 * @param s1 index of the first ship
 * @param s2 index of the second ship
 * @param c1 config id of the first ship (y, x, o)
 * @param c2 config id of the second ship (y, x, o)
 * 
 * @return 1 if the ships collide, 0 otherwise
 */
int shipConfigsCollide(int s1, int s2, int c1, int c2)
{
    int x1 = (c1 / 10) % 10;
    int y1 = c1 / 100;
    int x2 = (c2 / 10) % 10;
    int y2 = c2 / 100;

    int o1 = c1 % 10;
    int o2 = c2 % 10; // 0 (up) or 1 (right)

    int ship1Length = shipLengthFromIndex(s1);
    int ship2Length = shipLengthFromIndex(s2);

    for (int l1 = 0; l1 < ship1Length; l1++)
    { // compare each location of ship 1
        for (int l2 = 0; l2 < ship2Length; l2++)
        { // to each location of ship 2

            int ship1Location = o1 == 1 ? y1 * 10 + x1 + l1 : (y1 + l1) * 10 + x1;
            int ship2Location = o2 == 1 ? y2 * 10 + x2 + l2 : (y2 + l2) * 10 + x2;

            if (ship1Location == ship2Location)
                return 1;
        }
    }

    return 0;
}

/**
 * Returns the amount of configurations to be tested in a round
 * (AKA # of configs for each ship multiplied together)
 */
double numConfigsToBeTested()
{
    double num = 1;
    for (int i = 0; i < 5; i++)
    {
        if (!sunken[i]) num *= numShipConfigs[i];
    }
    return num;
}

/**
 * Overall function for generating the next move
 */
int generateMove(void)
{
    printf("Generating move...\n");

    // initialize needed maps
    shipCollisionMap = initializeHashmap();
    shipPositionFrequencyMap = initializeHashmap();

    generateShipConfigs();
    if (DEBUG)
        printf("Ship configs generated\n");

    determineShipCollisions();
    if (DEBUG)
        printf("Ship collisions generated\n");

    clock_t CPU_time_1 = clock(); // store START time

    int validConfigs = 0;

    double configsToBeTested = numConfigsToBeTested();
    int totalTested;

    if (configsToBeTested > MAX_CONFIGS_TESTED) {
        if (DEBUG) printf("Randomly testing configs\n");
        totalTested = MAX_CONFIGS_TESTED;
        validConfigs = randomlyTestConfigs();
    } else {
        if (DEBUG) printf("Brute force testing configs\n");
        totalTested = configsToBeTested;
        validConfigs = bruteForceTestConfigs();
    }

    clock_t CPU_time_2 = clock(); // store END time

    printf("Time taken: %fs\n", ((double)(CPU_time_2 - CPU_time_1)) / CLOCKS_PER_SEC);

    if (DEBUG) 
        printf("\n# valid configs: %d out of %d\n", validConfigs, totalTested);

    int move = calculateBestMove(validConfigs);

    if (DEBUG)
        printf("\nBest move calculated, was %d\n", move);

    // free memory :)
    free(shipCollisionMap);
    free(shipPositionFrequencyMap);

    if (DEBUG)
        printf("Maps freed, returning best move.\n");

    return move;
}

/**
 * Randomly generates and tests MAX_CONFIGS_TESTED configs
 * Used when the # of remaining configs is more than MAX_CONFIGS_TESTED
 */
int randomlyTestConfigs()
{
    int validConfigs = 0;
    for (int i = 0; i < MAX_CONFIGS_TESTED; i++)
    {

        if (DEBUG && i % 1000000 == 0)
            printf("Testing config %d\n", i);

        int testedShipConfigs[5]; // randomly selected ship configs

        // randomly select a config for each of the 5 ships (if not sunken)
        for (int j = 0; j < 5; j++)
        {
            int index = genrand_int32() % numShipConfigs[j];
            if (!sunken[j])
                testedShipConfigs[j] = shipConfigs[j][rand() % numShipConfigs[j]];
            else
                testedShipConfigs[j] = sunkenLocations[j];
        }

        // if the set of 5 generated ship configs is valid, add them to a
        // hashmap denoting the frequency of each ship config
        if (validConfig(testedShipConfigs))
        {
            validConfigs++;
            for (int s = 0; s < 5; s++)
            {
                int id = s * 1000 + testedShipConfigs[s];
                int previousFrequency = get(id, shipPositionFrequencyMap);
                if (previousFrequency == -1)
                    put(id, 1, shipPositionFrequencyMap);
                else
                    put(id, previousFrequency + 1, shipPositionFrequencyMap);
            }
        }
    }
    return validConfigs;
}

/**
 * Brute force generates and tests all remaining configs
 * Used when the # of remaining configs is less than MAX_CONFIGS_TESTED
 */
int bruteForceTestConfigs()
{

    int validConfigs = 0;

    // stores the configs of each ship in the testing
    int testedShipConfigs[5];

    int numShipConfigsUpdated[5];
    for(int i=0;i<5;i++) {
        if (sunken[i]) numShipConfigsUpdated[i] = 1;
        else numShipConfigsUpdated[i] = numShipConfigs[i];
    }

    // this is not a very good way to do this at all
    for (int c1 = 0; c1 < numShipConfigsUpdated[0]; c1++)
    {
        if (sunken[0]) testedShipConfigs[0] = sunkenLocations[0];
        else testedShipConfigs[0] = shipConfigs[0][c1];
        for (int c2 = 0; c2 < numShipConfigsUpdated[1]; c2++)
        {
            if (sunken[1]) testedShipConfigs[1] = sunkenLocations[1];
            else testedShipConfigs[1] = shipConfigs[1][c2];
            for (int c3 = 0; c3 < numShipConfigsUpdated[2]; c3++)
            {
                if (sunken[2]) testedShipConfigs[2] = sunkenLocations[2];
                else testedShipConfigs[2] = shipConfigs[2][c3];
                for (int c4 = 0; c4 < numShipConfigsUpdated[3]; c4++)
                {
                    if (sunken[3]) testedShipConfigs[3] = sunkenLocations[3];
                    else testedShipConfigs[3] = shipConfigs[3][c4];
                    for (int c5 = 0; c5 < numShipConfigsUpdated[4]; c5++)
                    {
                        if (sunken[4]) testedShipConfigs[4] = sunkenLocations[4];
                        else testedShipConfigs[4] = shipConfigs[4][c5];

                        // if the set of 5 generated ship configs is valid, add them to a
                        // hashmap denoting the frequency of each ship config
                        if (validConfig(testedShipConfigs))
                        {
                            validConfigs++;
                            for (int s = 0; s < 5; s++)
                            {
                                int id = s * 1000 + testedShipConfigs[s];
                                int previousFrequency = get(id, shipPositionFrequencyMap);
                                if (previousFrequency == -1)
                                    put(id, 1, shipPositionFrequencyMap);
                                else
                                    put(id, previousFrequency + 1, shipPositionFrequencyMap);
                            }
                        }
                    }
                }
            }
        }
    }

    return validConfigs;
}

/**
 * Given the configs of all 5 ships, tests to see if it is a valid board config
 * 1. Makes sure no ships are intersecting (using the map generated before)
 * 2. Makes sure all hit squares are covered
 */
int validConfig(int testedShipConfigs[5])
{
    // Makes sure none of the ships intersect with each other
    for (int s1 = 0; s1 < 5; s1++)
    {
        for (int s2 = s1 + 1; s2 < 5; s2++)
        {
            if (shipConfigsCollide(s1, s2, testedShipConfigs[s1], testedShipConfigs[s2]))
                return 0;
        }
    }

    // Stores which squares are covered by this board configuration
    int coveredSquares[BOARD_SIDELENGTH * BOARD_SIDELENGTH];
    for (int i = 0; i < BOARD_SIDELENGTH * BOARD_SIDELENGTH; i++)
        coveredSquares[i] = 0;

    for (int s = 0; s < 5; s++)
    {

        int currentCoord = testedShipConfigs[s] / 10;
        int right = testedShipConfigs[s] % 10;

        for (int l = 0; l < shipLengthFromIndex(s); l++)
        {
            coveredSquares[currentCoord] = 1;

            if (right)
                currentCoord++;
            else
                currentCoord += 10;
        }
    }

    // Makes sure all hit (but not on a sunk ship) squares are covered
    for (int x = BOARD_PADDING; x < BOARD_PADDING + BOARD_SIDELENGTH; x++)
    {
        for (int y = BOARD_PADDING; y < BOARD_PADDING + BOARD_SIDELENGTH; y++)
        {
            int spotID = 10 * (y - BOARD_PADDING) + x - BOARD_PADDING;
            if (S[y][x] == 3 && coveredSquares[spotID] == 0)
                return 0;
        }
    }

    return 1;
}

/**
 * Now that the frequencies of each ship configuration have been calculated,
 * this function actually fills out the frequencies of each individual
 * square on the board and uses these values to find the best move.
 * 
 * Finds the square with # of hits closest to t/2
 */
int calculateBestMove(int totalTested)
{
    int moveFrequencies[BOARD_SIDELENGTH * BOARD_SIDELENGTH];

    for (int i = 0; i < BOARD_SIDELENGTH * BOARD_SIDELENGTH; i++)
        moveFrequencies[i] = 0;

    for (int s = 0; s < 5; s++)
    {
        int numConfigs = numShipConfigs[s];
        int shipLength = shipLengthFromIndex(s);
        int shipMultiplied = s * 1000;

        if (!sunken[s])
        {
            for (int c = 0; c < numConfigs; c++)
            {

                // ship index, y, x, orientation
                int currConfig = shipConfigs[s][c];

                int configFrequency = get(shipMultiplied + currConfig, shipPositionFrequencyMap);

                int currentCoord = currConfig / 10;
                int right = currConfig % 10;

                if (configFrequency > 0)
                {
                    for (int l = 0; l < shipLength; l++)
                    {
                        moveFrequencies[currentCoord] += configFrequency;

                        int x = currentCoord % 10;
                        int y = currentCoord / 10;

                        if (right)
                            currentCoord++;
                        else
                            currentCoord += 10;
                    }
                }
            }
        }
    }

    double targetHits = ((double) totalTested) / 2; // don't worry about truncation

    int bestMove = -1;
    double bestDifference = __INT_MAX__;

    for (int i = 0; i < BOARD_SIDELENGTH * BOARD_SIDELENGTH; i++)
    {
        // iterate through all unguessed squares
        if (S[i / 10 + BOARD_PADDING][i % 10 + BOARD_PADDING] == 1)
        {
            if (moveFrequencies[i] != 0) {
                double diff = fabs(moveFrequencies[i] - targetHits);
                if (diff < bestDifference)
                {
                    bestDifference = diff;
                    bestMove = i;
                }
            }
        }
    }

    if (DEBUG) {
        printf("Best difference: %f\n", bestDifference);
    }

    free(moveFrequencies);

    return bestMove;
}

int shipLengthFromIndex(int i)
{
    switch (i)
    {
    case 0:
        return 2;
    case 1:
        return 3;
    case 2:
        return 3;
    case 3:
        return 4;
    case 4:
        return 5;
    }
    printf("%d \n", i);
    printf("Something has gone terribly wrong...\n");
    return 0;
}

void testCollide(int x1, int y1, int x2, int y2, int s1, int s2, int o1, int o2)
{

    int ship1Config = y1 * 100 + x1 * 10 + o1;
    int ship2Config = y2 * 100 + x2 * 10 + o2;
    int keyVal = s1 * 10000000 + s2 * 1000000 + ship1Config * 100 + ship2Config;
    printf("%d \n", get(keyVal, shipCollisionMap));

    return;
}