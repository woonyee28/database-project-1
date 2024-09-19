//Using bptree_iterative.h

#include <string.h>


void bulkLoadBPlusTree(BPlusTree* tree, float* keys, void** data, int numRecords,int MAX_KEYS);
BPlusTree* createTree();
BPlusTreeNode* createNode1(bool isLeaf,int MAX_KEYS);
void balanceLeafNodes(BPlusTreeNode **leaves, int leafCount, int minKeys, int maxKeys);
void adjustInternalNodes(BPlusTreeNode **nodes, int nodeCount, int minKeys, int maxKeys);
void serializeNode(FILE *file, BPlusTreeNode *node);
void serializeBPlusTree(BPlusTree *tree, const char *filename);
BPlusTreeNode* deserializeNode(FILE *file, int maxKeys);
BPlusTree* deserializeBPlusTree(const char *filename, int maxKeys);