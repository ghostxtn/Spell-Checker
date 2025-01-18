#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORDS 1000
#define MAX_WORD_LENGTH 50
#define MAX_LINE_LENGTH 1024

typedef struct {
    char** words;
    int size;
} WordSet;

    void addWord(WordSet* wordSet, const char* word) {
    wordSet->words[wordSet->size] = (char*)malloc(strlen(word) + 1);
    strcpy(wordSet->words[wordSet->size], word);
    wordSet->size++;
}

int contains(WordSet* wordSet, const char* word) {
    for (int i = 0; i < wordSet->size; i++) {
        if (strcasecmp(wordSet->words[i], word) == 0) {
            return 1;
        }
    }
    return 0;
}

char* swap(const char* word, int i, int j) {
    char* swapped = strdup(word);
    char temp = swapped[i];
    swapped[i] = swapped[j];
    swapped[j] = temp;
    return swapped;
}

char* insert(const char* word, int i, char character) {
    char* inserted = (char*)malloc(strlen(word) + 2);
    strncpy(inserted, word, i);
    inserted[i] = character;
    strcpy(inserted + i + 1, word + i);
    return inserted;
}

char* delete(const char* word, int i) {
    char* deleted = (char*)malloc(strlen(word));
    strncpy(deleted, word, i);
    strcpy(deleted + i, word + i + 1);
    return deleted;
}

char* replace(const char* word, int i, char character) {
    char* replaced = strdup(word);
    replaced[i] = character;
    return replaced;
}

void spellCheck(WordSet* wordSet, const char* word, WordSet* suggestions) {
    if (contains(wordSet, word)) {
        return;
    }

    for (int x = 0; x < strlen(word) - 1; x++) {
        char* swapped = swap(word, x, x + 1);
        if (contains(wordSet, swapped)) {
            addWord(suggestions, swapped);
        }
        free(swapped);
    }

    for (int i = 0; i <= strlen(word); i++) {
        for (char character = 'a'; character <= 'z'; character++) {
            char* inserted = insert(word, i, character);
            if (contains(wordSet, inserted)) {
                addWord(suggestions, inserted);
            }
            free(inserted);
        }
    }

    for (int i = 0; i < strlen(word); i++) {
        char* deleted = delete(word, i);
        if (contains(wordSet, deleted)) {
            addWord(suggestions, deleted);
        }
        free(deleted);
    }

    for (int i = 0; i < strlen(word); i++) {
        for (char character = 'a'; character <= 'z'; character++) {
            char* replaced = replace(word, i, character);
            if (contains(wordSet, replaced)) {
                addWord(suggestions, replaced);
            }
            free(replaced);
        }
    }
}

void freeWordSet(WordSet* wordSet) {
    for (int i = 0; i < wordSet->size; i++) {
        free(wordSet->words[i]);
    }
    free(wordSet->words);
}

void loadLexicon(const char* filename, WordSet* wordSet) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char word[MAX_WORD_LENGTH];
    while (fscanf(file, "%s", word) != EOF) {
        addWord(wordSet, word);
    }

    fclose(file);
}

void processWord(const char* rawWord, char* cleanWord) {
    int j = 0;
    for (int i = 0; rawWord[i] != '\0'; i++) {
        if (isalnum(rawWord[i])) {
            cleanWord[j++] = tolower(rawWord[i]);
        }
    }
    cleanWord[j] = '\0';
}

void check(const char* input, WordSet* wordSet, const char* output) {
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;

    FILE* inputFile = fopen(input, "r");
    if (!inputFile) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    FILE* outputFile = fopen(output, "w");
    if (!outputFile) {
        perror("Error opening output file");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, MAX_LINE_LENGTH, inputFile)) {
        lineNumber++;
        char* word = strtok(line, " \t\n\r");
        int columnNumber = 1;

        while (word) {
            char cleanWord[MAX_WORD_LENGTH];
            processWord(word, cleanWord);

            if (strlen(cleanWord) > 0 && !contains(wordSet, cleanWord)) {
                fprintf(outputFile, "Checking word: '%s' at line %d, column %d\n", word, lineNumber, columnNumber);

                WordSet suggestions = {.words = (char**)malloc(MAX_WORDS * sizeof(char*)), .size = 0};
                spellCheck(wordSet, cleanWord, &suggestions);

                fprintf(outputFile, "Suggestions:\n");
                for (int i = 0; i < suggestions.size; i++) {
                    fprintf(outputFile, "  %s\n", suggestions.words[i]);
                }
                fprintf(outputFile, "\n");

                freeWordSet(&suggestions);
            }

            columnNumber += strlen(word) + 1;
            word = strtok(NULL, " \t\n\r");
        }
    }

    fclose(inputFile);
    fclose(outputFile);
}

int main() {
    WordSet wordSet = {.words = (char**)malloc(MAX_WORDS * sizeof(char*)), .size = 0};

    loadLexicon("lexicon.txt", &wordSet);

    check("input.txt", &wordSet, "output.txt");

    freeWordSet(&wordSet);

    return 0;
}
