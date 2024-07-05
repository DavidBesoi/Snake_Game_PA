#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node *head = NULL;

void makeList(int key, char grila[][501], int rasp[]) {
    struct node* link = (struct node*) malloc(sizeof(struct node));

    link->key = key;
    for (int i = 0; i < 5; i++) {
        strcpy(link->grila[i], grila[i]);
    }
    for (int i = 0; i < 4; i++) {
        link->rasp[i] = rasp[i];
    }
    link->next = head;
    head = link;
}

void readFile() {
    FILE* ptr;
    char prop[501], rasp[11], *p, mat_prop[5][501];
    int v_rasp[4], cnt = 0;

    ptr = fopen("Questions.txt", "r");
    if (ptr == NULL) {
        printf("File can't be opened!");
        return;
    }

    while (fgets(prop, 501, ptr) != NULL) {
        strcpy(mat_prop[0], prop);
        for (int i = 0; i < 4; i++) {
            fgets(prop, 501, ptr);
            strcpy(mat_prop[i + 1], prop);
        }

        fgets(rasp, 11, ptr);
        memset(v_rasp, 0, sizeof(v_rasp)); // Ensure v_rasp is reset to 0
        p = strtok(rasp, " ");
        while (p) {
            int nr = atoi(p);
            if (nr >= 1 && nr <= 4) {
                v_rasp[nr - 1] = 1;
            }
            p = strtok(NULL, " ");
        }
        makeList(cnt, mat_prop, v_rasp);
        cnt++;
    }

    fclose(ptr);
}

void printList() {
    struct node *ptr = head;
    while (ptr != NULL) {
        printf("Key: %d\n", ptr->key);
        for (int i = 0; i < 5; i++) {
            printf("Grila[%d]: %s", i, ptr->grila[i]);
        }
        printf("Rasp: ");
        for (int i = 0; i < 4; i++) {
            printf("%d ", ptr->rasp[i]);
        }
        printf("\n\n");
        ptr = ptr->next;
    }
}