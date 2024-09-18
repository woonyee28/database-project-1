#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <windows.h>  
#include "storage.h"
#include "bptree_iterative.h"
#include "bptree_bulk_loading.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
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
int compare_records(const void *a, const void *b) {
    const NBA_Record *rec_a = (const NBA_Record *)a;
    const NBA_Record *rec_b = (const NBA_Record *)b;
    return (rec_a->fg_pct_home > rec_b->fg_pct_home) - (rec_a->fg_pct_home < rec_b->fg_pct_home);
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

int main() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
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
    read_data_from_binary_file("nba_data.bin", &records, &num_records, block_size);
    qsort(records, num_records, sizeof(NBA_Record), compare_records);
    int node_size = sizeof(BPlusTreeNode);

    /*        
    This line calculates the Global N value dynamically based on block size and compiler. 
    For 32-bit and 4KB-block, records_per_block = 508
    For 64-bit and 4KB-block, records_per_block = 337
    int records_per_block = MIN((block_size - node_size) / (sizeof(float) + sizeof(BPlusTreeNode*)),   // Leaf: max n of dataPtr/key pair
                                (block_size - node_size - sizeof(BPlusTreeNode*)) / (sizeof(float) + sizeof(BPlusTreeNode*))); // Internal: max n of(n+1 * child pointer)/n*key pair
    */
    // Fixing to 337 first
    int records_per_block = 337;
    //int num_blocks = (num_records + records_per_block - 1) / records_per_block;
    /*
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
    bulkLoadBPlusTree(tree, keys, data, num_records, records_per_block); 
    */
   
    int maxKeys = records_per_block;
    int minKeys = maxKeys/2;
    
    //Deserialising from saved tree
    //printf("loading Tree ... \n");
    BPlusTree *tree = deserializeBPlusTree("BulkTree.bin", records_per_block);

    // Print the tree for visual verification
    //printTreeDetailed(tree->root, 0);

    //save the tree to a file
    //serializeBPlusTree(tree, "BulkTree.bin");

    // Additional prints for analysis
    printf("Size of a node structure: %d bytes\n", node_size);
    printf("Number of records: %d\n", num_records);
    //printf("Number of blocks required: %d\n", num_blocks);
    printf("\nB+ Tree Statistics:\n");
    printf("1. N of B+ Tree: %d\n", records_per_block);
    printf("2. Number of nodes: %d\n", countNodes(tree->root)); 
    printf("3. Number of levels: %d\n", treeHeight(tree->root)); 
    printf("4. Content of the root node: ");
    printRootKeys(tree);  
    searchRange(tree, 0.500, 0.800);
    printf("\nBrute Force Linear Scan Method: \n");
    bruteForceScan(records, num_records, 0.500, 0.800, block_size);

    // Free resources
    //free(keys);
    //free(data);
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
