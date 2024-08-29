// bptree.c

#include <stdio.h>
#include <stdlib.h>
#include "bptree.h"
#include "storage.h"

BPlusTreeNode* createNode(int isLeaf) {
    BPlusTreeNode *node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    node->isLeaf = isLeaf;
    node->keys = (int*)malloc((DEGREE - 1) * sizeof(int));
    node->children = (BPlusTreeNode**)malloc(DEGREE * sizeof(BPlusTreeNode*));
    node->numKeys = 0;
    node->next = NULL;
    return node;
}


BPlusTree* createBPlusTree() {
    BPlusTree *tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    tree->root = createNode(1); 
    return tree;
}

void insert(BPlusTree *tree, int key, NBA_Record *record) {
    BPlusTreeNode *root = tree->root;

    if (root->numKeys == DEGREE - 1) {
        BPlusTreeNode *newRoot = createNode(0);
        newRoot->children[0] = root;
        splitChild(newRoot, 0, root);
        int i = (newRoot->keys[0] < key) ? 1 : 0;
        insertNonFull(newRoot->children[i], key, record);
        tree->root = newRoot;
    } else {
        insertNonFull(root, key, record);
    }
}

void insertNonFull(BPlusTreeNode *node, int key, NBA_Record *record) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->numKeys++;
    } else {
        while (i >= 0 && node->keys[i] > key) {
            i--;
        }
        i++;
        if (node->children[i]->numKeys == DEGREE - 1) {
            splitChild(node, i, node->children[i]);
            if (node->keys[i] < key) {
                i++;
            }
        }
        insertNonFull(node->children[i], key, record);
    }
}

void splitChild(BPlusTreeNode *parent, int index, BPlusTreeNode *child) {
    int t = DEGREE / 2;
    BPlusTreeNode *newNode = createNode(child->isLeaf);
    newNode->numKeys = t - 1;

    for (int j = 0; j < t - 1; j++) {
        newNode->keys[j] = child->keys[j + t];
    }

    if (!child->isLeaf) {
        for (int j = 0; j < t; j++) {
            newNode->children[j] = child->children[j + t];
        }
    }

    child->numKeys = t - 1;

    for (int j = parent->numKeys; j >= index + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[index + 1] = newNode;

    for (int j = parent->numKeys - 1; j >= index; j--) {
        parent->keys[j + 1] = parent->keys[j];
    }
    parent->keys[index] = child->keys[t - 1];
    parent->numKeys++;
}

void traverseNode(BPlusTreeNode *node) {
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) {
            traverseNode(node->children[i]);
        }
        printf(" %d", node->keys[i]);
    }

    if (!node->isLeaf) {
        traverseNode(node->children[i]);
    }
}

void traverse(BPlusTree *tree) {
    if (tree->root != NULL) {
        traverseNode(tree->root);
    }
}

void searchRange(BPlusTree *tree, int min, int max) {  
    BPlusTreeNode *node = tree->root;
    while (!node->isLeaf) {
        int i = 0;
        while (i < node->numKeys && min > node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }

    while (node != NULL) {
        for (int i = 0; i < node->numKeys; i++) {
            if (node->keys[i] >= min && node->keys[i] <= max) {
                printf("Found key: %d\n", node->keys[i]);
            }
        }
        node = node->next;
    }
}

void saveNodeToDisk(FILE *file, BPlusTreeNode *node) {
    fwrite(&node->isLeaf, sizeof(int), 1, file);
    fwrite(&node->numKeys, sizeof(int), 1, file);
    fwrite(node->keys, sizeof(int), node->numKeys, file);

    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) {
            saveNodeToDisk(file, node->children[i]);
        }
    }
}

void saveBPlusTreeToDisk(BPlusTree *tree, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file for writing");
        exit(1);
    }

    saveNodeToDisk(file, tree->root);
    fclose(file);
}

BPlusTreeNode* loadNodeFromDisk(FILE *file) {
    BPlusTreeNode *node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    fread(&node->isLeaf, sizeof(int), 1, file);
    fread(&node->numKeys, sizeof(int), 1, file);

    node->keys = (int*)malloc((DEGREE - 1) * sizeof(int));
    fread(node->keys, sizeof(int), node->numKeys, file);

    if (!node->isLeaf) {
        node->children = (BPlusTreeNode**)malloc(DEGREE * sizeof(BPlusTreeNode*));
        for (int i = 0; i <= node->numKeys; i++) {
            node->children[i] = loadNodeFromDisk(file);
        }
    }

    return node;  
}

BPlusTree* loadBPlusTreeFromDisk(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file for reading");
        exit(1);
    }

    BPlusTree *tree = createBPlusTree();
    tree->root = loadNodeFromDisk(file);
    fclose(file);
    return tree;
}

int countNodes(BPlusTreeNode *node) {
    if (node == NULL) return 0;
    int count = 1;
    for (int i = 0; i <= node->numKeys; i++) {
        count += countNodes(node->children[i]);
    }
    return count;
}

int treeHeight(BPlusTreeNode *node) {
    if (node == NULL) return 0;
    int height = 1;
    while (!node->isLeaf) {
        node = node->children[0];
        height++;
    }
    return height;
}

void printRootKeys(BPlusTree *tree) { 
    if (tree->root != NULL) {
        for (int i = 0; i < tree->root->numKeys; i++) {
            printf("%d ", tree->root->keys[i]);
        }
        printf("\n");
    }
}
