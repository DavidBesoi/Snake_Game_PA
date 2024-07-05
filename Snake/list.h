#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

struct node {
    int key;
    char grila[5][501];
    int rasp[4];
    struct node *next;
};

extern struct node *head;

void makeList(int key, char grila[][501], int rasp[]);
void readFile();
void printList();

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */