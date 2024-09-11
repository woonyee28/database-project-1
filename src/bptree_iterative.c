// bptree.c

#include "bptree_iterative.h"
#include "storage.h"

BPlusTreeNode* createNode(bool isLeaf) {
    BPlusTreeNode *node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    node->isLeaf = isLeaf;
    node->keys = (float*)malloc((N) * sizeof(float));
    node->children = (BPlusTreeNode**)malloc( (N+1) * sizeof(BPlusTreeNode*));
    node->numKeys = 0;
    node->next = NULL;
    return node;
}


BPlusTree* createBPlusTree() {
    BPlusTree *tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    tree->root = createNode(1); 
    return tree;
}

void insert(BPlusTree *tree, float key, NBA_Record *record) {
    BPlusTreeNode *root = tree->root;

    if (root->numKeys == N) {
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

void insertNonFull(BPlusTreeNode *node, float key, NBA_Record *record) {
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
        if (node->children[i]->numKeys == N) {
            splitChild(node, i, node->children[i]);
            if (node->keys[i] < key) {
                i++;
            }
        }
        insertNonFull(node->children[i], key, record);
    }
}

void splitChild(BPlusTreeNode *parent, int index, BPlusTreeNode *child) {
    int t = (N+1) / 2;
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

    newNode->next = child->next;  
    child->next = newNode;   

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
        printf(" %f", node->keys[i]);
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

void searchRange(BPlusTree *tree, float min, float max) {
    BPlusTreeNode *node = tree->root;

    while (!node->isLeaf) {
        int i = 0;
        while (i < node->numKeys && min > node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }

    int found = 0;
    while (node != NULL) {
        for (int i = 0; i < node->numKeys; i++) {
            if (node->keys[i] >= min && node->keys[i] <= max) {
                printf("%f ", node->keys[i]);
                found = 1;
            }
            else if (node->keys[i] > max) {
                return;
            }
        }
        node = node->next;
    }

    if (!found) {
        printf("No keys found in the range [%f, %f].\n", min, max);
    }
}

int countNodes(BPlusTreeNode *node) {
    if (node == NULL) return 0;
    int count = 1;  // Count the current node.
    // Only traverse the children if this is not a leaf node.
    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) {
            if (node->children[i] != NULL) {
                count += countNodes(node->children[i]);
            }
        }
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
            printf("%f ", tree->root->keys[i]);
        }
        printf("\n");
    }
}
