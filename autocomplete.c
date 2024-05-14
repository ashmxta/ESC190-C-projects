//Main file

#include "autocomplete.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmp1(const void *term_1, const void *term_2){
    return strcmp(((term*)term_1)->term, ((term*)term_2)->term);
}

int cmp2(const void *term_1, const void *term_2){
    if((((term*)term_2)->weight - ((term*)term_1)->weight) > 0){
        return 1;
    }
    else if((((term*)term_2)->weight - ((term*)term_1)->weight) < 0){
        return -1;
    }
    else{
        return 0;
    }
}

void read_in_terms(term **terms, int *pnterms, char *filename){
    *pnterms = 0; int n; char src[399];
    FILE *fp = fopen(filename, "r");
    fgets(src, 399, fp);
    n = atoi(src);
    *terms = (term *)malloc(n * sizeof(term));
    while (*pnterms < n){
        fgets(src, 399, fp);
        char *holder_1 = strtok(src, "\t");
        (*terms)[*pnterms].weight = atof(holder_1);
        char *holder_2 = strtok('\0', "\n");
        strcpy((*terms)[(*pnterms)++].term, holder_2);
    }
    fclose(fp);
    qsort(*terms, n, sizeof(term), cmp1);
}

int lowest_match(term *terms, int nterms, char *substr){
    int bot = 0; int top = nterms - 1; int res = -69; int mid;
    while (bot < top + 1){
        mid = (bot + top)/2;
        if (strncmp(terms[mid].term, substr, strlen(substr)) == 0){
            res = mid;
            top = mid - 1;
        }
        else if (strncmp(terms[mid].term, substr, strlen(substr)) > 0){
            top = mid - 1;
        }
        else{
            bot = mid + 1;
        }
    }
    return res;
}

int highest_match(struct term *terms, int nterms, char *substr){
    int bot = 0; int top = nterms - 1; int res = -69; int mid;
    while (bot < top + 1){
        mid = (bot + top)/2;
        if (strncmp(terms[mid].term, substr, strlen(substr)) == 0){
            res = mid;
            bot = mid + 1;
        }
        else if (strncmp(terms[mid].term, substr, strlen(substr)) > 0){
            top = mid - 1;
        }
        else{
            bot = mid + 1;
        }
    }
    return res;
}

void autocomplete(term **answer, int *n_answer, term *terms, int nterms, char *substr){
    int l = lowest_match(terms, nterms, substr);
    int h = highest_match(terms, nterms, substr);
    if(l == -69 && h == -69){
        *n_answer = 0;
    }
    else{
        *n_answer = h - l + 1;
    }
    *answer = (term *)malloc(*n_answer * sizeof(term));
    for ( int i = 0; i < *n_answer; i++){
        (*answer)[i] = terms[l + i];
    }
    qsort(*answer, *n_answer, sizeof(term), cmp2);
}
