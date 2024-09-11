// bptree.c

#include "bptree_iterative.h"
#include "storage.h"


BPlusTreeNode* createNode(bool isLeaf) {
    BPlusTreeNode *node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    node->isLeaf = isLeaf;
    node->keys = (float*)malloc((N) * sizeof(float));
    if (isLeaf) {
        node->data = (void**)malloc(N * sizeof(void*));  
        node->children = NULL;  
    } else {
        node->children = (BPlusTreeNode**)malloc((N + 1) * sizeof(BPlusTreeNode*)); 
        node->data = NULL;
    }
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
            node->data[i + 1] = node->data[i]; 
            i--;
        }
        node->keys[i + 1] = key;
        node->data[i + 1] = (void*)record; 
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
    newNode->numKeys = t;
    child->numKeys = N - t ;

    for (int j = 0; j < newNode->numKeys; j++) {
        newNode->keys[j] = child->keys[j + N - t];
    }

    if (child->isLeaf) {
        // Move the data pointers to the new leaf node
        for (int j = 0; j < newNode->numKeys; j++) {
            newNode->data[j] = child->data[j + N - t];
        }
        // Adjust the linked list of leaves
        newNode->next = child->next;
        child->next = newNode;
    } else {
        // Move the child pointers for internal nodes
        for (int j = 0; j <= newNode->numKeys; j++) {
            newNode->children[j] = child->children[j + N - t];
        }
    }

    // Shift parent's children to make room for the new node
    for (int j = parent->numKeys; j >= index + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[index + 1] = newNode;

    // Shift parent's keys to make room for the middle key
    for (int j = parent->numKeys - 1; j >= index; j--) {
        parent->keys[j + 1] = parent->keys[j];
    }

    // Copy the median key from child to the parent
    parent->keys[index] = newNode->keys[0];  // Use the middle key in child

    // Increase the number of keys in the parent
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
    int index_node_count = 0;  
    int data_block_count = 0;  
    int record_count = 0;      
    float fg3_pct_sum = 0.0;   
    clock_t start, end; 

    start = clock();

    while (!node->isLeaf) {
        index_node_count++;
        int i = 0;
        while (i < node->numKeys && min > node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }

    while (node != NULL) {
        data_block_count++;
        for (int i = 0; i < node->numKeys; i++) {
            if (node->keys[i] >= min && node->keys[i] <= max) {
                fg3_pct_sum += ((NBA_Record*)node->data[i])->fg3_pct_home;
                record_count++; 
            }
            else if (node->keys[i] > max) {
                break;
            }
        }
        node = node->next;
    }
    end = clock();

    printf("Number of records found in range: %d\n", record_count);
    printf("Number of index nodes accessed: %d\n", index_node_count);
    printf("Number of data blocks accessed: %d\n", data_block_count);
    
    if (record_count > 0) {
        float fg3_pct_avg = fg3_pct_sum / record_count;  
        printf("Average FG3_PCT_home of records: %.3f\n", fg3_pct_avg);
    } else {
        printf("No records found in the range.\n");
    }

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Query time: %lf seconds\n", time_taken);
}

int countNodes(BPlusTreeNode *node) {
    if (node == NULL) return 0;
    int count = 1;  
    
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

void bruteForceScan(NBA_Record *records, int num_records, float min, float max, int block_size) {
    int data_block_count = 0;
    int record_count = 0;
    float fg3_pct_sum = 0.0;
    clock_t start, end;

    start = clock();  

    int records_per_block = block_size / sizeof(NBA_Record);

    for (int i = 0; i < num_records; i++) {
        if (i % records_per_block == 0) {
            data_block_count++;
        }

        if (records[i].fg_pct_home >= min && records[i].fg_pct_home <= max) {
            fg3_pct_sum += records[i].fg3_pct_home;
            record_count++;
        }
    }

    end = clock(); 
    printf("Number of records found in range: %d\n", record_count);

    printf("Number of data blocks accessed: %d\n", data_block_count);

    if (record_count > 0) {
        float fg3_pct_avg = fg3_pct_sum / record_count;  
        printf("Average FG3_PCT_home of records: %.3f\n", fg3_pct_avg);
    }

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Brute-force scan time: %lf seconds\n", time_taken);
}

