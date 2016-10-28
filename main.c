#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Response {	//resp struct for storing Responses in Word structs
	char* resp; //calloced
	int freq;
	struct Response* next; //calloced
};

struct Word {   //Word struct for storing which words come after others
    char* stim;	//calloced
    int freq;
    struct Response resp;
    struct Word* next;  //calloced
};

int addWord(struct Word** list, char* newStr, char* prevStr) {	//adds word to chain
	if (*list == NULL) {
		*list = calloc(1, sizeof(struct Word));
		(*list)->stim = calloc(strlen(prevStr) + 1, sizeof(char));
		strcpy((*list)->stim, prevStr);

        (*list)->freq = 1;

		(*list)->resp.resp = calloc(strlen(newStr) + 1, sizeof(char));
		strcpy((*list)->resp.resp, newStr);

		(*list)->resp.freq = 1;
		(*list)->resp.next = NULL;
		(*list)->next = NULL;
		return 0;
	}
    //cycles through Words
    struct Word** head = list;
    struct Word** prev = &(*list);
    do {
        if (strcmp((*list)->stim, prevStr) == 0) {
            struct Response* prevResp = &((*list)->resp);
            struct Response* curr = &((*list)->resp);
            do {
                if (strcmp(curr->resp, newStr) == 0) {
                    //found the Word and Response
                    curr->freq++;
                    (*list)->freq++;
                    return 0;
                }
                prevResp = curr;
                curr = curr->next;
            } while (curr != NULL);
            //found the word but need to create new Response
            (*list)->freq++;
            struct Response* newResp = calloc(1, sizeof(struct Response));
            newResp->resp = calloc(strlen(newStr) + 1, sizeof(char));
            strcpy(newResp->resp, newStr);
            newResp->freq = 1;
            newResp->next = NULL;
            prevResp->next = newResp;
            return 0;
        }
        prev = list;
        list = &((*list)->next);
    } while (*list != NULL);
    //creates whole new Word
    struct Word* newNode = calloc(1, sizeof(struct Word));
    newNode->stim = calloc(strlen(prevStr) + 1, sizeof(char));
    strcpy(newNode->stim, prevStr);
    newNode->freq = 1;
    newNode->resp.resp = calloc(strlen(newStr) + 1, sizeof(char));
    strcpy(newNode->resp.resp, newStr);
    newNode->resp.freq = 1;
    newNode->resp.next = NULL;
    newNode->next = NULL;
    (*prev)->next = newNode;
	return 0;
}

void constructChain(FILE* chainFile, struct Word** chainPtr) {  //construct Word array from file
    char* prevWord = calloc(1, sizeof(char));
    char* word = calloc(1, sizeof(char));
    char curr;
    while ((curr = getc(chainFile)) != EOF) {
        if (isspace(curr) || curr == '\"' || curr == '(' || curr == ')' || curr == '[' || curr == ']' || curr == '{' || curr == '}') {	//Adds word to chain
            if (strlen(word) > 0) {
				if (strlen(prevWord) > 0) {
                	addWord(chainPtr, word, prevWord);
				}
                free(prevWord);
                prevWord = calloc(strlen(word) + 1, sizeof(char));
                strcpy(prevWord, word);
                free(word);
                word = calloc(1, sizeof(char));
            }
        }
        else {	//Adds curr to word
            size_t length = strlen(word);
            char* temp = calloc(length + 1, sizeof(char));
            strcpy(temp, word);
            free(word);
            word = calloc(length + 2, sizeof(char));
            strcpy(word, temp);
            word[length] = curr;
            word[length + 1] = '\0';
            free(temp);
        }
    }
    free(word);
    free(prevWord);
}

void generateSentence(struct Word* chain) {
    char* word;
    int totalFreq = 0;
    struct Word* curr = chain;
    while (curr != NULL) {
        totalFreq += curr->freq;
        curr = curr->next;
    }
    int index = (int) (totalFreq * ((double) rand() / (double) RAND_MAX));
    curr = chain;
    while (index > 0) {
        index -= curr->freq;
        curr = curr->next;
    }
    word = calloc(strlen(curr->stim) + 1, sizeof(char));
    strcpy(word, curr->stim);
    if (islower(word[0])) {
        word[0] = (char) toupper((int) word[0]);
        printf("%s", word);
        word[0] = (char) tolower((int) word[0]);
    } else {
        printf("%s", word);
    }
    while (strchr(word, '.') == NULL && strchr(word, '?') == NULL && strchr(word, '!') == NULL) {
        curr = chain;
        while (curr != NULL) {
            if (strcmp(word, curr->stim) == 0) {
                break;
            }
            curr = curr->next;
        }
        if (curr == NULL) {
            putc('.', stdout);
            break;
        }
        index = (int) (curr->freq * ((double) rand() / (double) RAND_MAX));
        struct Response* currResp = &curr->resp;
        index -= currResp->freq;
        while (index > 0 && currResp != NULL) {
            currResp = currResp->next;
            index -= currResp->freq;
        }
        if (currResp == NULL) {
            putc('.', stdout);
            break;
        }
        free(word);
        word = calloc(strlen(currResp->resp) + 1, sizeof(char));
        strcpy(word, currResp->resp);
        printf(" %s", word);
    }
    printf("\n");
    free(word);
}

void freeResp(struct Response** currResp) {
    struct Response** prevResp = currResp;
    while ((*currResp)->next != NULL) {
        prevResp = currResp;
        currResp = &(*currResp)->next;
    }
    if ((*prevResp)->next != NULL) {
        free((*prevResp)->next);
        (*prevResp)->next = NULL;
    }
    free((*currResp)->resp);
    free(*currResp);
    if (currResp == prevResp) {
        *currResp = NULL;
    }
}

void freeWord(struct Word** chain) {
    struct Word** prev = chain;
    while ((*chain)->next != NULL) {
        prev = chain;
        chain = &(*chain)->next;
    }
    if ((*prev)->next != NULL) {
        free((*prev)->next);
        (*prev)->next = NULL;
    }
    free((*chain)->stim);
    struct Response* currResp = &(*chain)->resp;
    while (currResp != NULL) {
        freeResp(&currResp);
    }
    free(*chain);
    if (chain == prev) {
        *chain = NULL;
    }
}

void freeChain(struct Word** chain) {
    while (*chain != NULL) {
        freeWord(chain);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    struct Word* chain = NULL;
    FILE* chainFile = fopen(argv[1], "r");
    if (chainFile == NULL) {
        fprintf(stderr, "Unable to open %s\n", argv[1]);
        return 1;
    }
    srand(time(NULL));
    constructChain(chainFile, &chain);
    generateSentence(chain);
	//freeChain(&chain);
    return 0;
}
