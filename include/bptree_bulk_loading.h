//Using bptree_iterative.h

#include <string.h>


void read_data_from_file_with_sorting(const char *filename, NBA_Record *records, int *num_records);
void bulkLoadBPlusTree(BPlusTree* tree, float* keys, void** data, int numRecords,int MAX_KEYS);
BPlusTree* createTree();
BPlusTreeNode* createNode1(bool isLeaf,int MAX_KEYS);
void balanceLeafNodes(BPlusTreeNode **leaves, int leafCount, int minKeys, int maxKeys);
void adjustInternalNodes(BPlusTreeNode **nodes, int nodeCount, int minKeys, int maxKeys);
