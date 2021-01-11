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

#define WORDS_LIST_FILENAME "words/list.txt"
#define WORDS_SHORTLIST_FILENAME "words/shortlist.txt"
#define WORDS_LIST_MAXLENGTH 1000
#define WORDS_SHORTLIST_MAXLENGTH 100

#define WORDS_

/* ----- GLOBAL VARIABLES ----- */

struct status
{
    int numGuesses;
    int guesses[ALPH_LENGTH];
    int guessResults[ALPH_LENGTH];
    int lieFound;
    int lieIndex;
};

char *wordsList;
char *wordsShortList;

int numWords;
int *numWordsOfEachLength;
int *wordLengths;

int wordMaxLength;
int maxWordsOverLengths;

/* ----- FUNCTION DECLARATIONS ----- */

double entropy(struct status *status);
double entropyLieFound(struct status *status);
double entropyLieNotFound(struct status *status);

void populateWordsList();

int ind2(int i, int j);
int ind3(int i, int j, int k);

/* ----- CODE ----- */

/**
 * Prints the welcome screen. According to the player's action,
 * plays the game or quits.
 */
int main()
{

    populateWordsList();

    

    return 0;
}

void populateWordsList() {
    numWords = 2;

    FILE *wordsListFile;

    if (numWords == 1) wordsListFile = fopen(WORDS_LIST_FILENAME, "r");
    else wordsListFile = fopen(WORDS_SHORTLIST_FILENAME, "r"); // use shortlist for phrases

    wordLengths = malloc(sizeof(int) * numWords);
    wordLengths[0] = 16;
    wordLengths[1] = 16;

    numWordsOfEachLength = malloc(sizeof(int) * numWords);
    for (int i = 0; i < numWords; i++)
        numWordsOfEachLength[i] = 0;

    // get the max word length
    wordMaxLength = 0;
    for (int i = 0; i < numWords; i++)
        if (wordLengths[i] > wordMaxLength) wordMaxLength = wordLengths[i] + 1;

    // get the max number of word possibilities per word
    int currentWordLength = 0;
    char c = fgetc(wordsListFile);
    while (c != EOF)
    {
        currentWordLength++;

        c = fgetc(wordsListFile);
        if (c == '\n')
        {
            for (int i = 0; i < numWords; i++)
                if (currentWordLength - 1 == wordLengths[i]) numWordsOfEachLength[i]++;
            currentWordLength = 0;
        }
    }
    maxWordsOverLengths = 0; // = max # of words over the lengths we have
    for (int i = 0; i < numWords; i++)
    {
        if (DEBUG) printf("%d words with length %d\n", numWordsOfEachLength[i], wordLengths[i]);
        if (numWordsOfEachLength[i] > maxWordsOverLengths)
            maxWordsOverLengths = numWordsOfEachLength[i];
    }

    wordsList = malloc(numWords * maxWordsOverLengths * (wordMaxLength + 1) * sizeof(char));

    if (DEBUG) printf("Allocating %d bytes of memory\n", numWords * 50 * 21 * sizeof(char));

    rewind(wordsListFile);

    // actually populate the words
    int *numPopulatedWords = malloc(sizeof(int) * numWords);
    for (int i = 0; i < numWords; i++)
        numPopulatedWords[i] = 0;
    
    char *tempWord = malloc(wordMaxLength * sizeof(char) + 1);
    currentWordLength = 0;
    c = fgetc(wordsListFile);

    while (c != EOF)
    {
        tempWord[currentWordLength] = c;
        currentWordLength++;

        c = fgetc(wordsListFile);

        if (c == '\n')
        {
            for (int i = 0; i < numWords; i++)
            {
                if (currentWordLength == wordLengths[i])
                {

                    int index = ind2(i, numPopulatedWords[i]);
                    for (int j = 0; j < currentWordLength; j++)
                    {
                        wordsList[index] = tempWord[j];
                        index++;
                    }

                    wordsList[index] = '\0'; // add null term
                    numPopulatedWords[i]++;
                }
            }

            currentWordLength = 0;
            c = fgetc(wordsListFile);
        }
    }

    if (DEBUG)
    {
        printf("Reached the end\n");
        for (int i = 0; i < numWords; i++)
        {
            printf("Words with length %d:\n", wordLengths[i]);
            for (int j = 0; j < numWordsOfEachLength[i]; j++) printf("%s\n", wordsList + ind2(i, j));
        }
    }

    return;
}

int ind2(int i, int j)
{
    return i * (maxWordsOverLengths * (wordMaxLength + 1)) + j * (wordMaxLength + 1);
}

int ind3(int i, int j, int k)
{
    return i * (maxWordsOverLengths * (wordMaxLength + 1)) + j * (wordMaxLength + 1) + k;
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