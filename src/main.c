// main.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "storage.h"
#include "bptree_iterative.h"
#include "bptree_bulk_loading.h"

// Define constants for file paths
#define TEXT_DATA_FILE "games.txt"
#define BINARY_DATA_FILE "nba_data.bin"
#define BPTREE_ITERATIVE_FILE "BulkTree_Iterative.bin"
#define BPTREE_BULK_FILE "BulkTree_Bulk.bin"

// Define maximum number of records (adjust as needed)
#define MAX_RECORDS 27000

// Function Prototypes
void task1_store_data(int block_size, NBA_Record **records, int *num_records);
void task2_build_bptrees(int block_size, NBA_Record *records, int num_records, BPlusTree **tree_iterative, BPlusTree **tree_bulk);
void task3_search(BPlusTree *tree_iterative, BPlusTree *tree_bulk, NBA_Record *records, int num_records, int block_size);
long get_block_size(void); // Prototype for get_block_size
int compare_records(const void *a, const void *b);

int main() {
    // Task 1: Storage Component
    printf("=== Task 1: Storage Component ===\n");
    long block_size = get_block_size();
    printf("Block size: %ld bytes\n\n", block_size);

    NBA_Record *records = NULL;
    int num_records = 0;
    task1_store_data(block_size, &records, &num_records);

    // Task 2: Indexing Component
    printf("\n=== Task 2: Indexing Component ===\n");
    BPlusTree *tree_iterative = NULL;
    BPlusTree *tree_bulk = NULL;
    task2_build_bptrees(block_size, records, num_records, &tree_iterative, &tree_bulk);

    // Task 3: Search Operations
    printf("\n=== Task 3: Search Operations ===\n");
    task3_search(tree_iterative, tree_bulk, records, num_records, block_size);

    // Cleanup
    free(records);
    freeBPlusTree(tree_iterative);
    freeBPlusTree(tree_bulk);

    return 0;
}

/**
 * @brief Retrieves the system's page size (block size).
 * 
 * This function uses platform-specific methods to obtain the block size.
 * 
 * @return long The block size in bytes.
 */
long get_block_size(void) {
    long block_size = 0;

    #ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        block_size = si.dwPageSize;
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        long res = sysconf(_SC_PAGESIZE);
        if (res == -1) {
            perror("Failed to get block size");
            exit(EXIT_FAILURE);
        }
        block_size = res;
    #else
        fprintf(stderr, "Unsupported operating system.\n");
        exit(EXIT_FAILURE);
    #endif

    return block_size;
}

int compare_records(const void *a, const void *b) {
    const NBA_Record *rec_a = (const NBA_Record *)a;
    const NBA_Record *rec_b = (const NBA_Record *)b;
    return (rec_a->fg_pct_home > rec_b->fg_pct_home) - (rec_a->fg_pct_home < rec_b->fg_pct_home);
}

/**
 * @brief Task 1: Read data from text file and store it into a binary file.
 * 
 * @param block_size The size of a block in bytes.
 * @param records Pointer to the NBA_Record array.
 * @param num_records Pointer to the number of records.
 */
void task1_store_data(int block_size, NBA_Record **records, int *num_records) {
    // Read data from text file
    printf("Reading data from %s...\n", TEXT_DATA_FILE);
    *records = (NBA_Record *)malloc(MAX_RECORDS * sizeof(NBA_Record));
    if (*records == NULL) {
        perror("Failed to allocate memory for records");
        exit(EXIT_FAILURE);
    }
    read_data_from_file(TEXT_DATA_FILE, *records, num_records);
    printf("Total records read: %d\n", *num_records);

    // Store data to binary file
    printf("Storing data to binary file %s...\n", BINARY_DATA_FILE);
    store_data_to_disk(*records, *num_records, BINARY_DATA_FILE, block_size);
    printf("Data stored successfully.\n");

    // Report Statistics
    printf("\n--- Storage Statistics ---\n");
    printf("Size of a record: %lu bytes\n", sizeof(NBA_Record));
    printf("Number of records: %d\n", *num_records);
    int records_per_block = block_size / sizeof(NBA_Record);
    int num_blocks = (*num_records + records_per_block - 1) / records_per_block;
    printf("Number of records per block: %d\n", records_per_block);
    printf("Total number of blocks: %d\n", num_blocks);
}

/**
 * @brief Task 2: Build both Iterative and Bulk-Loaded B+ Trees and report statistics.
 * 
 * @param block_size The size of a block in bytes.
 * @param records The NBA_Record array.
 * @param num_records The number of records.
 * @param tree_iterative Pointer to the Iterative BPlusTree structure.
 * @param tree_bulk Pointer to the Bulk-Loaded BPlusTree structure.
 */
void task2_build_bptrees(int block_size, NBA_Record *records, int num_records, BPlusTree **tree_iterative, BPlusTree **tree_bulk) {
    // Read data from binary file
    printf("Reading data from binary file %s...\n", BINARY_DATA_FILE);
    NBA_Record *binary_records = NULL;
    int binary_num_records = 0;
    read_data_from_binary_file(BINARY_DATA_FILE, &binary_records, &binary_num_records, block_size);
    qsort(binary_records, binary_num_records, sizeof(NBA_Record), compare_records);
    printf("Total records loaded from binary file: %d\n", binary_num_records);

    // ======================
    // Build Iterative B+ Tree
    // ======================
    printf("\nBuilding Iterative B+ Tree on FG_PCT_home...\n");
    *tree_iterative = createBPlusTree();
    if (*tree_iterative == NULL) {
        fprintf(stderr, "Failed to create Iterative B+ Tree.\n");
        exit(EXIT_FAILURE);
    }

    // Insert records one by one
    for (int i = 0; i < binary_num_records; i++) {
        insert(*tree_iterative, binary_records[i].fg_pct_home, &binary_records[i]);
    }
    printf("Iterative B+ Tree built successfully.\n");

    // Report Iterative B+ Tree Statistics
    printf("\n--- Iterative B+ Tree Statistics ---\n");
    printf("Parameter n (max keys per node): %d\n", N);
    int total_nodes_iterative = countNodes((*tree_iterative)->root);
    printf("Number of nodes in Iterative B+ Tree: %d\n", total_nodes_iterative);
    int height_iterative = treeHeight((*tree_iterative)->root);
    printf("Number of levels in Iterative B+ Tree: %d\n", height_iterative);
    printf("Keys in the root node of Iterative B+ Tree: ");
    printRootKeys(*tree_iterative);

    // ======================
    // Build Bulk-Loaded B+ Tree
    // ======================
    printf("\nBuilding Bulk-Loaded B+ Tree on FG_PCT_home...\n");
    float *keys = (float *)malloc(binary_num_records * sizeof(float));
    void **data_ptrs = (void **)malloc(binary_num_records * sizeof(void *));
    if (keys == NULL || data_ptrs == NULL) {
        perror("Failed to allocate memory for keys/data pointers");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < binary_num_records; i++) {
        keys[i] = binary_records[i].fg_pct_home;
        data_ptrs[i] = &binary_records[i];
    }

    *tree_bulk = createBPlusTree();
    if (*tree_bulk == NULL) {
        fprintf(stderr, "Failed to create Bulk-Loaded B+ Tree.\n");
        free(keys);
        free(data_ptrs);
        exit(EXIT_FAILURE);
    }

    bulkLoadBPlusTree(*tree_bulk, keys, data_ptrs, binary_num_records, N); // N is defined as 337 in bptree_iterative.h
    printf("Bulk-Loaded B+ Tree built successfully.\n");

    // Report Bulk-Loaded B+ Tree Statistics
    printf("\n--- Bulk-Loaded B+ Tree Statistics ---\n");
    printf("Parameter n (max keys per node): %d\n", N);
    int total_nodes_bulk = countNodes((*tree_bulk)->root);
    printf("Number of nodes in Bulk-Loaded B+ Tree: %d\n", total_nodes_bulk);
    int height_bulk = treeHeight((*tree_bulk)->root);
    printf("Number of levels in Bulk-Loaded B+ Tree: %d\n", height_bulk);
    printf("Keys in the root node of Bulk-Loaded B+ Tree: ");
    printRootKeys(*tree_bulk);
}

/**
 * @brief Task 3: Perform range search using both B+ trees and brute-force scan.
 * 
 * @param tree_iterative The Iterative BPlusTree structure.
 * @param tree_bulk The Bulk-Loaded BPlusTree structure.
 * @param records The NBA_Record array.
 * @param num_records The number of records.
 * @param block_size The size of a block in bytes.
 */
void task3_search(BPlusTree *tree_iterative, BPlusTree *tree_bulk, NBA_Record *records, int num_records, int block_size) {
    float min = 0.500;
    float max = 0.800;

    // ======================
    // Search in Iterative B+ Tree
    // ======================
    printf("Searching for records with FG_PCT_home in range [%.2f, %.2f] using Iterative B+ Tree...\n", min, max);
    searchRange(tree_iterative, min, max);

    // ======================
    // Search in Bulk-Loaded B+ Tree
    // ======================
    printf("\nSearching for records with FG_PCT_home in range [%.2f, %.2f] using Bulk-Loaded B+ Tree...\n", min, max);
    searchRange(tree_bulk, min, max);

    // ======================
    // Brute-Force Linear Scan
    // ======================
    printf("\nPerforming brute-force linear scan for comparison...\n");
    bruteForceScan(records, num_records, min, max, block_size);
}
