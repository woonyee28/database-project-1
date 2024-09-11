#ifndef BPTREE_ITERATIVE_H
#define BPTREE_ITERATIVE_H

#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h> 

// N is the number of keys in a BPlusTreeNode
#define N 340

typedef struct BPlusTreeNode {
    float *keys;   
    void **data;                 
    struct BPlusTreeNode **children;  
    struct BPlusTreeNode *next; 
    int numKeys;             
    bool isLeaf;        
} BPlusTreeNode;

typedef struct BPlusTree {
    BPlusTreeNode *root;
} BPlusTree;

BPlusTree* createBPlusTree();
void insert(BPlusTree *tree, float key, NBA_Record *record);
void insertNonFull(BPlusTreeNode *node, float key, NBA_Record *record);
void splitChild(BPlusTreeNode *parent, int index, BPlusTreeNode *child);
void traverse(BPlusTree *tree);
void traverseNode(BPlusTreeNode *node);
void searchRange(BPlusTree *tree, float min, float max);
int countNodes(BPlusTreeNode *node);  
int treeHeight(BPlusTreeNode *node);  
void printRootKeys(BPlusTree *tree);  
void bruteForceScan(NBA_Record *records, int num_records, float min, float max, int block_size);

#endif
