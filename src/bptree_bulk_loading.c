#include "storage.h"
#include "bptree_iterative.h"
#include "bptree_bulk_loading.h"

void balanceLeafNodes(BPlusTreeNode **leaves, int leafCount, int minKeys, int maxKeys) {
    for (int i = 0; i < leafCount - 1; i++) {
        // Check if the next leaf needs keys
        while (leaves[i]->numKeys > minKeys && leaves[i + 1]->numKeys < minKeys) {
            // Transfer a key from the current node to the next
            float keyToTransfer = leaves[i]->keys[leaves[i]->numKeys - 1];
            void* dataToTransfer = leaves[i]->data[leaves[i]->numKeys - 1];
            leaves[i]->numKeys--;

            // Find the insertion point in the next leaf while making space 
            int insertionPoint = leaves[i + 1]->numKeys;
            while (insertionPoint > 0 && leaves[i + 1]->keys[insertionPoint - 1] > keyToTransfer) {
                leaves[i + 1]->keys[insertionPoint] = leaves[i + 1]->keys[insertionPoint - 1];
                leaves[i + 1]->data[insertionPoint] = leaves[i + 1]->data[insertionPoint - 1];
                insertionPoint--;
            }

            // Insert the key and data in the correct position
            leaves[i + 1]->keys[insertionPoint] = keyToTransfer;
            leaves[i + 1]->data[insertionPoint] = dataToTransfer;
            leaves[i + 1]->numKeys++;
        }
    }
}
void adjustInternalNodes(BPlusTreeNode **nodes, int nodeCount, int minKeys, int maxKeys) {
    // Loop through internal nodes and adjust them
    for (int i = nodeCount - 1; i > 0; i--) {
        BPlusTreeNode *current = nodes[i];
        BPlusTreeNode *previous = nodes[i - 1];
        // Redistribute keys and children between nodes
        while (current->numKeys < minKeys && previous->numKeys > minKeys) {

            // Transfer the smallest key of the **next child** to the current node
            // moves all key and children, e.g. index0 -> index1, index1 ->index2 ....
            memmove(&current->keys[1], &current->keys[0], current->numKeys * sizeof(float));
            memmove(&current->children[1], &current->children[0], (current->numKeys + 1) * sizeof(BPlusTreeNode*));

            // The correct key for the internal node should reflect the smallest key of the next child
            current->keys[0] = current->children[1]->keys[0];
            current->children[0] = previous->children[previous->numKeys];
            current->numKeys++;

            previous->numKeys--;

            // Update the parent key to be the smallest key of the next child in the previous node
            if (previous->numKeys > 0) {
                previous->keys[previous->numKeys - 1] = previous->children[previous->numKeys]->keys[0];
            }
        }

        // If the previous node becomes underfull, merge it with the current node
        if (previous->numKeys < minKeys) {
            // Transfer all keys and children from current to previous
            memcpy(&previous->keys[previous->numKeys], &current->keys[0], current->numKeys * sizeof(float));
            memcpy(&previous->children[previous->numKeys + 1], &current->children[0], (current->numKeys + 1) * sizeof(BPlusTreeNode*));

            previous->numKeys += current->numKeys;

            // Adjust the links
            previous->next = current->next;
            // Free the current node
            free(current->keys);
            free(current->data);
            free(current->children);
            free(current);

            nodeCount--;
            i--;  // Decrement i to continue checking the current node
        }
    }

    // Final pass to update all parent keys
    for (int j = 0; j < nodeCount - 1; j++) {
        if (nodes[j + 1]->children[0]) {
            nodes[j]->keys[nodes[j]->numKeys - 1] = nodes[j + 1]->children[0]->keys[0]; // Set the key to the smallest key of the i+1th child
        }
    }

    // Ensure the root node points correctly if only one node remains
    if (nodeCount == 1) {
        nodes[0]->next = NULL;
    }
}
void bulkLoadBPlusTree(BPlusTree* tree, float* keys, void** data, int numRecords, int MAX_KEYS) {
    int numLeaves = (numRecords + MAX_KEYS - 1) / MAX_KEYS;
    BPlusTreeNode **leaves = (BPlusTreeNode**) malloc(numLeaves * sizeof(BPlusTreeNode*));
    if (!leaves) {
        return; // Memory allocation failure
    }

    int i = 0, leafIndex = 0;
    while (i < numRecords) {
        BPlusTreeNode* leaf = createNode1(true,MAX_KEYS);
        if (!leaf) {
            while (leafIndex > 0) {
                free(leaves[--leafIndex]->data);
                free(leaves[leafIndex]->keys);
                free(leaves[leafIndex]);
            }
            free(leaves);
            return;
        }

        while (leaf->numKeys < MAX_KEYS && i < numRecords) {
            leaf->keys[leaf->numKeys] = keys[i];
            leaf->data[leaf->numKeys] = data[i];
            leaf->numKeys++;
            i++;
        }

        leaves[leafIndex++] = leaf;
    }

    balanceLeafNodes(leaves, leafIndex, (MAX_KEYS+1)/2, MAX_KEYS);
    // Link the leaves
    for (int j = 0; j < leafIndex - 1; j++) {
        leaves[j]->next = leaves[j + 1];
    }

    // Create upper levels
    while (leafIndex > 1) {
        int parentIndex = 0;
    int numParents = (leafIndex + MAX_KEYS - 1) / MAX_KEYS;
    BPlusTreeNode **parents = (BPlusTreeNode**) malloc(numParents * sizeof(BPlusTreeNode*));

    for (int i = 0; i < leafIndex; i += MAX_KEYS) {
        BPlusTreeNode* parent = createNode1(false, MAX_KEYS);
        // Ensure the first child is linked correctly
        parent->children[0] = leaves[i];

        // Start k from i to group MAX_KEYS children under one parent
        int k;
        for (k = i; k < i + MAX_KEYS - 1 && k + 1 < leafIndex; k++) {
            // Promote the first key of the next child node
            parent->keys[parent->numKeys] = leaves[k + 1]->keys[0];
            parent->children[parent->numKeys + 1] = leaves[k + 1];
            parent->numKeys++;
        }
        // Special handling for the last group if it does not fill up to MAX_KEYS
        if (k < leafIndex) {
            parent->children[parent->numKeys + 1] = leaves[k];
        }
        parents[parentIndex++] = parent;
    }

        adjustInternalNodes(parents, parentIndex, (MAX_KEYS) / 2, MAX_KEYS);

        free(leaves);
        leaves = parents;
        leafIndex = parentIndex;
    }

    tree->root = leaves[0];
    free(leaves);
}


BPlusTreeNode* createNode1(bool isLeaf,int MAX_KEYS) {
    BPlusTreeNode *node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    node->isLeaf = isLeaf;
    node->keys = (float*)malloc((MAX_KEYS) * sizeof(float));
    if (isLeaf) {
        node->data = (void**)malloc(MAX_KEYS * sizeof(void*));  
        node->children = NULL;  
    } else {
        node->children = (BPlusTreeNode**)malloc((MAX_KEYS + 1) * sizeof(BPlusTreeNode*)); 
        node->data = NULL;
    }
    node->numKeys = 0;
    node->next = NULL;
    return node;
}

// Function to create an empty B+ tree
BPlusTree* createTree() {
    BPlusTree* tree = (BPlusTree*) malloc(sizeof(BPlusTree));
    if (!tree) {
        perror("Failed to allocate memory for B+ tree");
        exit(EXIT_FAILURE);
    }
    tree->root = NULL;
    return tree;
}

void serializeNode(FILE *file, BPlusTreeNode *node) {
    if (!node) return;

    // Write basic node information
    fwrite(&node->numKeys, sizeof(int), 1, file);
    fwrite(&node->isLeaf, sizeof(bool), 1, file);

    // Write keys
    fwrite(node->keys, sizeof(float), node->numKeys, file);

    if (node->isLeaf) {
        // Serialize pointers to the records (e.g., memory addresses or file offsets)
        for (int i = 0; i < node->numKeys; i++) {
            fwrite(&node->data[i], sizeof(void*), 1, file);  // Write pointers (or some reference)
        }

        // Serialize the 'next' pointer (if it exists)
        bool hasNext = node->next != NULL;
        fwrite(&hasNext, sizeof(bool), 1, file);
        if (hasNext) {
            serializeNode(file, node->next);  // Recursively serialize the next leaf node
        }
    } else {
        // For internal nodes, recursively serialize the children
        for (int i = 0; i <= node->numKeys; i++) {
            serializeNode(file, node->children[i]);
        }
    }
}

void serializeBPlusTree(BPlusTree *tree, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    // Serialize the root node
    serializeNode(file, tree->root);

    fclose(file);
}
BPlusTreeNode* deserializeNode(FILE *file, int maxKeys) {
    BPlusTreeNode *node = (BPlusTreeNode*) malloc(sizeof(BPlusTreeNode));
    if (!node) return NULL;

    // Read basic node information
    fread(&node->numKeys, sizeof(int), 1, file);
    fread(&node->isLeaf, sizeof(bool), 1, file);

    // Allocate memory for keys
    node->keys = (float*) malloc(maxKeys * sizeof(float));
    fread(node->keys, sizeof(float), node->numKeys, file);

    if (node->isLeaf) {
        // Allocate memory for pointers to the records (not the actual records)
        node->data = malloc(maxKeys * sizeof(void*));
        fread(node->data, sizeof(void*), node->numKeys, file);  // Read the pointers (or references)

        // Deserialize the 'next' pointer
        bool hasNext;
        fread(&hasNext, sizeof(bool), 1, file);
        if (hasNext) {
            node->next = deserializeNode(file, maxKeys);  // Recursively deserialize the next leaf node
        } else {
            node->next = NULL;  // No next leaf node
        }
    } else {
        // For internal nodes, recursively deserialize the children
        node->children = (BPlusTreeNode**) malloc((maxKeys + 1) * sizeof(BPlusTreeNode*));
        for (int i = 0; i <= node->numKeys; i++) {
            node->children[i] = deserializeNode(file, maxKeys);
        }
    }

    return node;
}

BPlusTree* deserializeBPlusTree(const char *filename, int maxKeys) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file for reading");
        return NULL;
    }

    BPlusTree *tree = (BPlusTree*) malloc(sizeof(BPlusTree));
    tree->root = deserializeNode(file, maxKeys);

    fclose(file);
    return tree;
}
