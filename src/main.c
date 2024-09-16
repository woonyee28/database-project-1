#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <windows.h>  
#include "D:/Download/SC3020/include/storage.h"
#include "D:/Download/SC3020/include/bptree_iterative.h"
#include "D:/Download/SC3020/include/bptree_bulk_loading.h"

void printNBARecord(NBA_Record *record) {
    printf("Game Date: %s\n", record->game_date_est);
    printf("Team ID (Home): %d\n", record->team_id_home);
    printf("Points (Home): %d\n", record->pts_home);
    printf("Field Goal %% (Home): %.3f\n", record->fg_pct_home);
    printf("Free Throw %% (Home): %.3f\n", record->ft_pct_home);
    printf("3-Point %% (Home): %.3f\n", record->fg3_pct_home);
    printf("Assists (Home): %d\n", record->ast_home);
    printf("Rebounds (Home): %d\n", record->reb_home);
    printf("Home Team Wins: %d\n", record->home_team_wins);
    printf("----\n");
}
bool verifyBPlusTree(BPlusTreeNode *root, int minKeys, int maxKeys, BPlusTree *tree) {
    if (!root) return true;

    // For the root node, the number of keys can be from 1 to MAX_KEYS
    if (root == tree->root) {
        if (root->numKeys < 1 || root->numKeys > maxKeys) {
            printf("Number of Keys Incorrect in Root! Node has %d keys, expected between %d and %d\n", root->numKeys, 1, maxKeys);
            return false;
        }
    } else if (!root->isLeaf) {
        // For internal nodes (non-root)
        if (root->numKeys < minKeys || root->numKeys > maxKeys) {
            printf("Number of Keys Incorrect! Node has %d keys, expected between %d and %d\n", root->numKeys, minKeys, maxKeys);
            return false;
        }
    }

    // Verify that keys are sorted
    for (int i = 0; i < root->numKeys - 1; i++) {
        if (root->keys[i] > root->keys[i + 1]) {
            printf("Key not sorted! Keys: %.2f >= %.2f\n", root->keys[i], root->keys[i + 1]);
            return false;
        }
    }

    // Recursively verify children
    if (!root->isLeaf) {
        for (int i = 0; i <= root->numKeys; i++) {
            if (!verifyBPlusTree(root->children[i], minKeys, maxKeys,tree)) return false;
        }
    }

    return true;
}

void printTreeDetailed(BPlusTreeNode *node, int level) {
    if (node == NULL) return;

    // Check if the node is a leaf
    if (node->isLeaf) {
        // Print only the first and last key for leaf nodes
        if (node->numKeys > 0) {
            printf("Level %d, Leaf Keys: %.3f ... %.3f\n", level, node->keys[0], node->keys[node->numKeys - 1]);
        }
    } else {
        // For internal nodes, print all keys
        printf("Level %d, Internal Keys:", level);
        for (int i = 0; i < node->numKeys; i++) {
            printf(" %.3f", node->keys[i]);
        }
        printf("\n");
        // Recursively print child nodes
        for (int i = 0; i <= node->numKeys; i++) {
            printTreeDetailed(node->children[i], level + 1);
        }
    }
}
int countNode(BPlusTreeNode* node) {
    if (node == NULL) return 0;
    if (node->isLeaf) return 1;

    int count = 1; // Count this internal node
    for (int i = 0; i <= node->numKeys; i++) {
        count += countNodes(node->children[i]);
    }
    return count;
}

// Function to get the height (levels) of the tree
int getTreeHeight(BPlusTreeNode* node) {
    int height = 0;
    while (node != NULL) {
        height++;
        node = node->isLeaf ? NULL : node->children[0];  // Go down to the first child
    }
    return height;
}

// Report tree statistics
void reportTreeStats(BPlusTree* tree) {
    printf("B+ Tree Statistics:\n");
    
    // Parameter n of the B+ tree
    printf("Parameter n (max number of keys per node): %d\n", tree->root->numKeys);

    // Number of nodes in the B+ tree
    int nodeCount = countNode(tree->root);
    printf("Number of Nodes: %d\n", nodeCount);

    // Number of levels in the B+ tree (Height)
    int height = getTreeHeight(tree->root);
    printf("Number of Levels: %d\n", height);

    // Content of the root node (keys)
    printf("Keys in Root Node: ");
    for (int i = 0; i < tree->root->numKeys; i++) {
        printf("%.2f ", tree->root->keys[i]);
    }
    printf("\n");
}

void searchBPlusTree(BPlusTree* tree, float lower_bound, float upper_bound, int records_per_block) {
    clock_t start, end; 
    BPlusTreeNode *node = tree->root;

    // Traverse from the root to the first leaf node
    start = clock();
    int level = 0;
    int index_blocks_accessed = 1;  // Start with the root block (Level 0)
    while (!node->isLeaf) {
        node = node->children[0];
        level++;
        index_blocks_accessed++;  // Each level access is a new index block
    }

    int total_leaf_blocks = 0, total_records = 0;
    float sum_fg3_pct = 0.0;  // Sum for FG3_PCT_home
    int current_block = -1;
    
    // Traverse the leaf nodes and find records in the range
    while (node != NULL) {
        int leaf_block_accessed = 0; // Flag to track if a new block was accessed
        for (int i = 0; i < node->numKeys; i++) {
            if (node->keys[i] >= lower_bound && node->keys[i] <= upper_bound) {
                NBA_Record *record = (NBA_Record*) node->data[i];
                sum_fg3_pct += record->fg3_pct_home;
                total_records++;

                // Calculate which block the current record belongs to
                int block_id = total_records / records_per_block;
                if (block_id != current_block) {
                    current_block = block_id;
                    total_leaf_blocks++;  // A new leaf block (data block) accessed
                    leaf_block_accessed = 1;
                }
            }
        }
        
        // If no record from this node was accessed but this node is a new block, count the block
        if (!leaf_block_accessed && total_records > 0) {
            total_leaf_blocks++; 
        }

        node = node->next;
    }
    end = clock();

    float average_fg3_pct = total_records ? sum_fg3_pct / total_records : 0;
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    // Print search results
    printf("Search Results: FG_PCT_home between %.2f and %.2f\n", lower_bound, upper_bound);
    printf("Number of Index Blocks Accessed: %d\n", index_blocks_accessed);
    printf("Number of Leaf (Data) Blocks Accessed: %d\n", total_leaf_blocks);
    printf("Average FG3_PCT_home: %.3f\n", average_fg3_pct);
    printf("Total Records Found: %d\n", total_records);
    printf("Query time: %lf seconds\n", time_taken);
}

int main() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    int globalRecordsPerBlock;
    long block_size = si.dwPageSize;
    if (block_size == -1) {
        perror("Failed to get block size");
        return 1;
    }
    
    printf("Block size: %ld bytes\n", block_size);

    int max_records = 27000;  
    NBA_Record *records = (NBA_Record *)malloc(max_records * sizeof(NBA_Record));
    if (records == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    int num_records = 0;
    read_data_from_file_with_sorting("games.txt", records, &num_records);
    int record_size = sizeof(NBA_Record);
    int records_per_block = block_size / record_size;
    int num_blocks = (num_records + records_per_block - 1) / records_per_block;
    BPlusTree* tree = createTree();
    float* keys = (float*) malloc(num_records * sizeof(float));
    void** data = (void**) malloc(num_records * sizeof(void*));
    if (!keys || !data) {
        perror("Failed to allocate memory for keys/data arrays");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_records; i++) {
        keys[i] = records[i].fg_pct_home;  
        data[i] = &records[i];           
    }
    printf("Starting bulk load...\n");
    bulkLoadBPlusTree(tree, keys, data, num_records, records_per_block);
    printf("Bulk load complete.\n");
    int maxKeys = records_per_block;
    int minKeys = maxKeys/2;
    

    // Print the tree for visual verification
    //printTreeDetailed(tree->root, 0);
    if (verifyBPlusTree(tree->root, minKeys, maxKeys, tree)) {
        printf("The B+ tree is correctly structured.\n");
    } 
    else {
        printf("The B+ tree structure is incorrect!\n");
    }
    // Additional prints for analysis
    printf("Size of a record: %d bytes\n", record_size);
    printf("Number of records: %d\n", num_records);
    printf("Records per block: %d\n", records_per_block);
    printf("Number of blocks required: %d\n", num_blocks);
    reportTreeStats(tree);
    searchBPlusTree(tree, 0.5, 0.8);
    // Free resources
    free(keys);
    free(data);
    free(records);

    return 0;
    /*
    printf("\n---------------------------------Data Verification---------------------------------\n");
    printf("Verifying the first 5 records after sorting:\n");
    for (int i = 0; i < 200 && i < num_records; i++) {
        printf("%d - Date: %s, Team ID: %d, Points: %d, FG%%: %.3f\n",
               i + 1,
               records[i].game_date_est,
               records[i].team_id_home,
               records[i].pts_home,
               records[i].fg_pct_home);
    }*/

    
    /* read_data_from_file("games.txt", records, &num_records);

    store_data_to_disk(records, num_records, "nba_data.bin", block_size);

    int record_size = sizeof(NBA_Record);
    int records_per_block = block_size / record_size;
    int num_blocks = (num_records + records_per_block - 1) / records_per_block;

    printf("\n---------------------------------Task 1---------------------------------\n");

    printf("Size of a record: %d bytes\n", record_size);
    printf("Number of records: %d\n", num_records);
    printf("Records per block: %d\n", records_per_block);
    printf("Number of blocks required: %d\n", num_blocks);

    free(records);

    NBA_Record *records_v2 = (NBA_Record *)malloc(max_records * sizeof(NBA_Record));

    read_data_from_binary_file("nba_data.bin", &records_v2, &num_records, block_size);

    // Print the 120rd record (index 119) to verify correctness
    // printNBARecord(&records_v2[120]);

    printf("\n---------------------------------Task 2---------------------------------\n");

    BPlusTree *bptree = createBPlusTree();

    // Print the size of BPlusTreeNode
    printf("\n\nSize of a BPlusTreeNode: %lu bytes\n", (unsigned long)sizeof(BPlusTreeNode));

    for (int i = 0; i < num_records; i++) {
        insert(bptree, records_v2[i].fg_pct_home, &records_v2[i]);  
    }

    printf("\nB+ Tree Statistics:\n");
    printf("1. N of B+ Tree: %d\n", N);
    printf("2. Number of nodes: %d\n", countNodes(bptree->root)); 
    printf("3. Number of levels: %d\n", treeHeight(bptree->root)); 
    printf("4. Content of the root node: ");
    printRootKeys(bptree);  

    printf("\n---------------------------------Task 3 (Iterative Method) ---------------------------------\n");

    printf("\nQuerying B+ Tree for FG_PCT_home between 0.5 and 0.8:\n");

    searchRange(bptree, 0.500, 0.800);  
    printf("\n");

    printf("\nBrute Force Linear Scan Method: \n");

    bruteForceScan(records_v2, num_records, 0.500, 0.800, block_size);

    free(records_v2); 
    return 0;*/
}
