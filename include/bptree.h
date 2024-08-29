#ifndef BPTREE_H
#define BPTREE_H

#include "storage.h"

#define DEGREE 3  

typedef struct BPlusTreeNode {
    int *keys;                
    struct BPlusTreeNode **children;  
    struct BPlusTreeNode *next; 
    int numKeys;             
    int isLeaf;            
} BPlusTreeNode;

typedef struct BPlusTree {
    BPlusTreeNode *root;
} BPlusTree;

BPlusTree* createBPlusTree();
void insert(BPlusTree *tree, int key, NBA_Record *record);
void insertNonFull(BPlusTreeNode *node, int key, NBA_Record *record);
void splitChild(BPlusTreeNode *parent, int index, BPlusTreeNode *child);
void traverse(BPlusTree *tree);
void traverseNode(BPlusTreeNode *node);
void searchRange(BPlusTree *tree, int min, int max);
void saveBPlusTreeToDisk(BPlusTree *tree, const char *filename);
void saveNodeToDisk(FILE *file, BPlusTreeNode *node);
BPlusTree* loadBPlusTreeFromDisk(const char *filename);
BPlusTreeNode* loadNodeFromDisk(FILE *file);
int countNodes(BPlusTreeNode *node);  
int treeHeight(BPlusTreeNode *node);  
void printRootKeys(BPlusTree *tree);  

#endif
