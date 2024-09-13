#include "D:/Download/SC3020/include/storage.h"
#include "D:/Download/SC3020/include/bptree_iterative.h"
#include "D:/Download/SC3020/include/bptree_bulk_loading.h"
int compare_records(const void *a, const void *b) {
    const NBA_Record *rec_a = (const NBA_Record *)a;
    const NBA_Record *rec_b = (const NBA_Record *)b;
    return (rec_a->fg_pct_home > rec_b->fg_pct_home) - (rec_a->fg_pct_home < rec_b->fg_pct_home);
}

void read_data_from_file_with_sorting(const char *filename, NBA_Record *records, int *num_records) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    fgets(line, sizeof(line), file);  // Skip header line, assuming the first line is a header

    int i = 0;
    int record_count_in_range = 0; 
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%10s %d %d %f %f %f %hd %hd %hd",
               records[i].game_date_est,
               &records[i].team_id_home,
               &records[i].pts_home,
               &records[i].fg_pct_home,
               &records[i].ft_pct_home,
               &records[i].fg3_pct_home,
               &records[i].ast_home,
               &records[i].reb_home,
               &records[i].home_team_wins);
        /*/if (records[i].fg_pct_home >= 0.500 && records[i].fg_pct_home <= 0.800) {
            record_count_in_range++;
        }/*/
        i++;
    }
    *num_records = i;

    qsort(records, *num_records, sizeof(NBA_Record), compare_records);
    //printf("Number of records within the range [0.500, 0.800]: %d\n", record_count_in_range);
    fclose(file);
}
void adjustInternalNodes(BPlusTreeNode **nodes, int nodeCount, int minKeys, int maxKeys) {
    for (int i = nodeCount - 1; i > 0; i--) {
        BPlusTreeNode *current = nodes[i];
        BPlusTreeNode *previous = nodes[i - 1];

        // Redistribute keys to maintain minimum keys constraint
        while (current->numKeys < minKeys && previous->numKeys > minKeys) {
            // Shift all keys in current node right
            memmove(&current->keys[1], &current->keys[0], current->numKeys * sizeof(*(current->keys)));
            // Move the last key of the previous node to the first key of the current node
            current->keys[0] = previous->keys[previous->numKeys - 1];

            // Shift all child pointers in current node right
            memmove(&current->children[1], &current->children[0], (current->numKeys + 1) * sizeof(*(current->children)));
            // Move the last child of the previous node to the first child of the current node
            current->children[0] = previous->children[previous->numKeys];

            current->numKeys++;
            previous->numKeys--;

            // Update the last key of the previous node to be the smallest key of its new last child
            if (previous->numKeys > 0) {
                previous->keys[previous->numKeys - 1] = previous->children[previous->numKeys]->keys[0];
            }
        }

        // If previous node becomes underfull, merge it with current
        if (previous->numKeys < minKeys) {
            // Transfer all keys and children from current to previous
            memcpy(&previous->keys[previous->numKeys], &current->keys[0], current->numKeys * sizeof(*(current->keys)));
            memcpy(&previous->children[previous->numKeys], &current->children[0], (current->numKeys + 1) * sizeof(*(current->children)));

            previous->numKeys += current->numKeys;

            // Adjust the links
            if (current->next) {
                previous->next = current->next;
            } else {
                previous->next = NULL;
            }

            // Free the current node
            free(current->keys);
            free(current->data);
            free(current->children);
            free(current);

            // Decrease the count of nodes
            nodeCount--;
        }
    }

    // In case only one node is left, it becomes the root
    if (nodeCount == 1) {
        nodes[0]->next = NULL; // There's no next node at the root level
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
            int childCount = 0;
            for (int k = i; k <= i + MAX_KEYS && k < leafIndex; k++) {
                parent->children[childCount] = leaves[k];
                if (childCount > 0) {
                    parent->keys[childCount - 1] = leaves[k]->keys[0]; // Promote the first key of the right child
                }
                childCount++;
            }
            parent->numKeys = childCount - 1;
            parents[parentIndex++] = parent;
        }

        adjustInternalNodes(parents, parentIndex, (MAX_KEYS + 1) / 2, MAX_KEYS); // Adjust the nodes to ensure all are balanced except maybe the last one

        free(leaves);
        leaves = parents;
        leafIndex = parentIndex;
    }

    tree->root = leaves[0];
    free(leaves);
}


BPlusTreeNode* createNode1(bool isLeaf,int MAX_KEYS) {
    BPlusTreeNode* node = (BPlusTreeNode*) malloc(sizeof(BPlusTreeNode));
    if (!node) {
        return NULL;
    }
    node->keys = (float*) malloc(MAX_KEYS * sizeof(float));
    node->data = malloc(MAX_KEYS * sizeof(void*));
    node->children = isLeaf ? NULL : (BPlusTreeNode**) malloc((MAX_KEYS + 1) * sizeof(BPlusTreeNode*));
    node->numKeys = 0;
    node->isLeaf = isLeaf;
    node->next = NULL;

    if (node->keys == NULL || node->data == NULL || (!isLeaf && node->children == NULL)) {
        free(node->keys);
        free(node->data);
        free(node->children);
        free(node);
        return NULL;
    }

    if (!isLeaf) {
        memset(node->children, 0, (MAX_KEYS + 1) * sizeof(BPlusTreeNode*));
    }
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
