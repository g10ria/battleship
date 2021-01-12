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

struct status status;

char *wordsList;
int numWords;
int *numWordsOfEachLength;
int *wordLengths;
int wordMaxLength;
int maxWordsOverLengths;

int totalPossibs = 1;

char *currLetters;
int tempMissed[ALPH_LENGTH]; // for checking entropy; missed flags for letters

int phraseLength;

/* ----- FUNCTION DECLARATIONS ----- */

double entropy();
double entropyLieFound();
double entropyLieNotFound();
double entropyOfStatus();

void populateWordsList();
void allocateCurrLetters();

int ind2(int i, int j);
int ind3(int i, int j, int k);
char ch(int i, int j, int k);

/* ----- CODE ----- */

/**
 * Prints the welcome screen. According to the player's action,
 * plays the game or quits.
 */
int main()
{
    numWords = 2;
    wordLengths = malloc(sizeof(int) * numWords);
    wordLengths[0] = 15;
    wordLengths[1] = 16;

    allocateCurrLetters();
    populateWordsList();

    status.numGuesses = 0;
    status.lieFound = 1;

    
    // move this somewhere else lol
    for (int i = 0; i < ALPH_LENGTH; i++)
    {
        tempMissed[i] = 0;
    }
    tempMissed[12] = 1;

    printf("%f\n", entropyOfStatus());

    return 0;
}

void allocateCurrLetters()
{
    phraseLength = 0;
    for (int i = 0; i < numWords; i++)
    {
        phraseLength += wordLengths[i];
        if (i > 0)
            phraseLength++; // add the space
    }
    currLetters = malloc(sizeof(char) * phraseLength);

    for (int i = 0; i < phraseLength; i++)
    {
        currLetters[i] = ' ';
    }

    return;
}

// struct status
// {
//     int numGuesses;
//     int guesses[ALPH_LENGTH];
//     int guessResults[ALPH_LENGTH];
//     int lieFound;
//     int lieIndex;
// };

int testCombination(int *indices)
{
    int curr = 0;
    char c;
    for (int j = 0; j < numWords; j++)
    {
        for (int k = 0; k < wordLengths[j]; k++)
        {
            c = ch(j, indices[j], k);
            // printf("%c is %d\n", c, c);
            
            for(int i=0;i<ALPH_LENGTH;i++) {
                
                if (tempMissed[c-97]) return 0;
            }

            if (currLetters[curr] != ' ') // filled
            {
                if (ch(j, indices[j], k) != currLetters[curr])
                {
                    return 0;
                }
            }

            curr++;
        }
        curr++;
    }
    return 1;
}

double entropyLieFound() {
    return 0.0;
}

double entropyOfStatus()
{
    int validCombos = 0;

    int *indices = malloc(sizeof(int) * numWords);
    for (int i = 0; i < numWords; i++)
        indices[i] = 0;
    indices[numWords - 1] = -1;

    char *currTested = malloc(sizeof(char) * phraseLength);

    for (int i = 0; i < totalPossibs; i++)
    {

        int overflow = 1;
        for (int j = numWords - 1; j >= 0; j--)
        {

            if (overflow)
                indices[j]++;
            if (indices[j] == numWordsOfEachLength[j])
            {
                indices[j] = 0;
            }
            else
                overflow = 0;
        }

        if (testCombination(indices)) {
            // lol it worked
            validCombos++;
            printf("this one worked %d %d\n", indices[0], indices[1]);
        } else {
            printf("this one didn't work %d %d\n", indices[0], indices[1]);
        }
    }
    return log2((double) validCombos);
}

double entropyLieNotFound()
{
    return 0.0;
}

double entropy()
{
    if (status.lieFound)
        return entropyLieFound(status);
    return entropyLieNotFound(status);
}

void populateWordsList()
{
    FILE *wordsListFile;

    if (numWords == 1)
        wordsListFile = fopen(WORDS_LIST_FILENAME, "r");
    else
        wordsListFile = fopen(WORDS_SHORTLIST_FILENAME, "r"); // use shortlist for phrases

    numWordsOfEachLength = malloc(sizeof(int) * numWords);
    for (int i = 0; i < numWords; i++)
        numWordsOfEachLength[i] = 0;

    // get the max word length
    wordMaxLength = 0;
    for (int i = 0; i < numWords; i++)
        if (wordLengths[i] > wordMaxLength)
            wordMaxLength = wordLengths[i] + 1;

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
                if (currentWordLength - 1 == wordLengths[i])
                    numWordsOfEachLength[i]++;
            currentWordLength = 0;
        }
    }
    maxWordsOverLengths = 0; // = max # of words over the lengths we have
    for (int i = 0; i < numWords; i++)
    {
        if (DEBUG)
            printf("%d words with length %d\n", numWordsOfEachLength[i], wordLengths[i]);
        if (numWordsOfEachLength[i] > maxWordsOverLengths)
            maxWordsOverLengths = numWordsOfEachLength[i];
    }

    wordsList = malloc(numWords * maxWordsOverLengths * (wordMaxLength + 1) * sizeof(char));

    if (DEBUG)
        printf("Allocating %d bytes of memory\n", numWords * 50 * 21 * sizeof(char));

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
            for (int j = 0; j < numWordsOfEachLength[i]; j++)
                printf("%s\n", wordsList + ind2(i, j));
        }
    }

    for (int i = 0; i < numWords; i++)
    {
        totalPossibs *= numWordsOfEachLength[i];
    }

    return;
}

char ch(int i, int j, int k)
{
    return wordsList[ind3(i, j, k)];
}

int ind2(int i, int j)
{
    return i * (maxWordsOverLengths * (wordMaxLength + 1)) + j * (wordMaxLength + 1);
}

int ind3(int i, int j, int k)
{
    return i * (maxWordsOverLengths * (wordMaxLength + 1)) + j * (wordMaxLength + 1) + k;
}
