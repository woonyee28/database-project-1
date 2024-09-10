#ifndef BPTREE_H
#define BPTREE_H

#include "storage.h"

#define DEGREE 341

typedef struct BPlusTreeNode {
    float *keys;                
    struct BPlusTreeNode **children;  
    struct BPlusTreeNode *next; 
    int numKeys;             
    int isLeaf;            
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

#endif
