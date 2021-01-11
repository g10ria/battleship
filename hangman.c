#include "./headers/hangman.h"

/**
 * Hangman guesser written for Info Theory (20-21)
 * @created 01/01/20
 * 
 * asdfjkl;
 */

/* ----- MACROS ----- */

#define DEBUG 1        // set to 1 to print debug messages, 0 otherwise
#define ALPH_LENGTH 26 // length of the alphabet

#define WORDS_LIST_FILENAME "list.txt"
#define WORDS_SHORTLIST_FILENAME "shortlist.txt"
#define WORDS_LIST_LENGTH 1000
#define WORDS_SHORTLIST_LENGTH 100

/* ----- GLOBAL VARIABLES ----- */

struct status
{
    int numGuesses;
    int guesses[ALPH_LENGTH];
    int guessResults[ALPH_LENGTH];
    int lieFound;
    int lieIndex;
};

char wordsList[WORDS_LIST_LENGTH];
char wordsShortList[WORDS_SHORTLIST_LENGTH];

/* ----- FUNCTION DECLARATIONS ----- */

double entropy(struct status *status);
double entropyLieFound(struct status *status);
double entropyLieNotFound(struct status *status);

/* ----- CODE ----- */

/**
 * Prints the welcome screen. According to the player's action,
 * plays the game or quits.
 */
int main()
{
    FILE wordsListFile = open

    return 0;
}

double entropy(struct status *status)
{
    if (status->lieFound)
        return entropyLieFound(status);
    return entropyLieNotFound(status);
}

double entropyLieFound(struct status *status)
{
    return 0.0;
}
double entropyLieNotFound(struct status *status)
{
    return 0.0;
}