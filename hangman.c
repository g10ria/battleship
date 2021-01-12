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
    int numGuesses; // for printing
    int guesses[ALPH_LENGTH]; // for printing
    int guessResults[ALPH_LENGTH]; // for printing

    int lieFound; // if a lie has been found
    int lieIndex; // index of the lie (in the guesses) - also for printing

    int alphabetStatuses[ALPH_LENGTH]; // status of each letter
};

struct status status; // global status variable

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

int numUnconfirmedMisses;
int numConfirmedMisses;
int totalMisses;

int testingMissPossibility;
char mustHave;

/* ----- FUNCTION DECLARATIONS ----- */

double entropy();
double entropyLieFound();
double entropyLieNotFound();
double entropyOfStatus();
double entropyNoLie();
double entropyWithLie();

void populateWordsList();
void allocateCurrLetters();

int ind2(int i, int j);
int ind3(int i, int j, int k);
char ch(int i, int j, int k);

void init();

/* ----- CODE ----- */

/**
 * Prints the welcome screen. According to the player's action,
 * plays the game or quits.
 */
int main()
{
    numWords = 2;
    wordLengths = malloc(sizeof(int) * numWords);
    wordLengths[0] = 14;
    wordLengths[1] = 15;

    init();

    allocateCurrLetters();
    populateWordsList();

    status.numGuesses = 0;
    status.lieFound = 1;

    // int * test = malloc(8);
    // printf("dunzo\n");
    
    // move this somewhere else lol
    
    status.alphabetStatuses[2] = 3; // set c to unconfirmed miss
    // currLetters[0] = 'r';

    numUnconfirmedMisses = 1;

    printf("%f\n", entropyWithLie());

    return 0;
}

void init() {
    testingMissPossibility = 0;
    for (int i = 0; i < ALPH_LENGTH; i++)
    {
        status.alphabetStatuses[i] = 0;
    }
}

int testCombination(int *indices)
{
    int has = 0; // if we have the mustHave or not

    int curr = 0;
    char c;

    for (int j = 0; j < numWords; j++)
    {
        for (int k = 0; k < wordLengths[j]; k++)
        {
            // current character in the combination being tested
            c = ch(j, indices[j], k);
            // printf("%c is %d\n", c, c);

            // we have the mustHave character
            if (testingMissPossibility && c-97 == mustHave) has = 1;
            
            for(int i=0;i<ALPH_LENGTH;i++) {
                // if the character is supposed to be missed
                if (status.alphabetStatuses[c-97] == 2 || status.alphabetStatuses[c-97] == 3)
                    return 0;
            }

            // if the character doesn't match the template
            if (currLetters[curr] != ' ' && ch(j, indices[j], k) != currLetters[curr]) return 0;

            curr++;
        }
        curr++;
    }
    if (testingMissPossibility && !has) return 0;
    return 1;
}

double entropyNoLie() {
    return entropyOfStatus(1.0);
    // takes the entropy of the current status; assumes the lie has
    // already been found and corrected
}

double entropyWithLie() {
    
    /**
     * alphabetStatuses:
     * 
     * 0 = unguessed
     * 1 = hit
     * 2 = missed (confirmed)
     * 3 = missed (unconfirmed)
     * 4 = must have (don't know where)
     * 
     */ 

    // if something going from a miss to a hit yields no results, then
    // it's a confirmed miss (cannot have been a lie)
    for(int i=0;i<ALPH_LENGTH;i++) {
        if (status.alphabetStatuses[i] == 3) {

            status.alphabetStatuses[i] = 4; // set to must have
            mustHave = i;
            testingMissPossibility = 1;

            if (entropyOfStatus(1.0) < 0) // invalid
            {
                printf("%c is a confirmed miss\n", i);
                status.alphabetStatuses[i] = 2; // confirmed miss
                numUnconfirmedMisses--;
                numConfirmedMisses++;
            }
            testingMissPossibility = 0;
            status.alphabetStatuses[i] = 3;
        }
    }

    // if (numUnconfirmedMisses == 0) return entropyNoLie();

    double probOfLie = 1.0 / ((double) numUnconfirmedMisses + 1);

    printf("%f chance of lying\n", probOfLie);

    double entropy = 0.0;

    printf("getting entropy with no misses:\n");

    entropy += entropyOfStatus(probOfLie);

    printf("entropy before misses: %f\n", entropy);

    printf("getting entropy with a miss\n");

    for(int i=0;i<ALPH_LENGTH;i++) {
        if (status.alphabetStatuses[i] == 3) {

            status.alphabetStatuses[i] = 4; // set to must have
            mustHave = i;
            testingMissPossibility = 1;

            double e = entropyOfStatus(probOfLie);

            entropy += e;

            testingMissPossibility = 0;

            status.alphabetStatuses[i] = 3;
        }
    }

    return entropy;
}

double entropyOfStatus(double probFactor)
{
    int validCombos = 0;

    // indices used for multi-word submissions
    int * indices;

    indices = malloc(sizeof(int) * numWords);

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

        if (testCombination(indices)) { // combination worked
            validCombos++;
            // printf("this one worked %d %d\n", indices[0], indices[1]);
        } else {
            // printf("this one didn't work %d %d\n", indices[0], indices[1]);
        }
    }

    free(indices);
    free(currTested);

    if (validCombos == 0) return -1.0; // essentially "infinite" entropy

    printf("number combos: %d\n", validCombos);

    // printf("%f\n", log2(validCombos / probFactor));

    return probFactor * log2((double) validCombos / probFactor);
}




/** Functions below this point not important */



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
    
    // printf("%d %d %d\n", numWords, maxWordsOverLengths, wordMaxLength + 1);

    if (DEBUG)
        printf("Allocating %d bytes of memory\n", numWords * maxWordsOverLengths * (wordMaxLength + 1) * sizeof(char));

    rewind(wordsListFile);

    // actually populate the words
    int *numPopulatedWords = malloc(sizeof(int) * numWords);
    for (int i = 0; i < numWords; i++)
        numPopulatedWords[i] = 0;

    printf("max length is %d\n", wordMaxLength);
    char *tempWord = malloc(wordMaxLength * sizeof(char));
    currentWordLength = 0;
    c = fgetc(wordsListFile);

    while (c != EOF)
    {
        if (currentWordLength < wordMaxLength) {
            tempWord[currentWordLength] = c;
        }
        
        currentWordLength++;

        c = fgetc(wordsListFile);

        if (c == '\n')
        {
            for (int i = 0; i < numWords; i++)
            {
                if (currentWordLength == wordLengths[i])
                {
                    // printf("adding word to length %d\n", currentWordLength);
                    
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
            for (int j = 0; j < numWordsOfEachLength[i]; j++) {
                printf("%s\n", wordsList + ind2(i, j));
            }
                
        }
    }

    for (int i = 0; i < numWords; i++)
    {
        totalPossibs *= numWordsOfEachLength[i];
    }

    printf("total possibs: %d\n\n", totalPossibs);

    free(numPopulatedWords);
    free(tempWord);

    fclose(wordsListFile);

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