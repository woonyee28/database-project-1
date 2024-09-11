#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include "storage.h"
#include "bptree_iterative.h"

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

int main() {
    long block_size = sysconf(_SC_PAGESIZE); 
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
    read_data_from_file("games.txt", records, &num_records);

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
    printf("\n\nSize of a BPlusTreeNode: %lu bytes\n",sizeof(BPlusTreeNode));

    for (int i = 0; i < num_records; i++) {
        insert(bptree, records_v2[i].fg_pct_home, &records_v2[i]);  
    }

    printf("\nB+ Tree Statistics:\n");
    printf("1. N of B+ Tree: %d\n", N);
    printf("2. Number of nodes: %d\n", countNodes(bptree->root)); 
    printf("3. Number of levels: %d\n", treeHeight(bptree->root)); 
    printf("4. Content of the root node: ");
    printRootKeys(bptree);  

    printf("\n---------------------------------Task 3---------------------------------\n");

    printf("\nQuerying B+ Tree for FG_PCT_home between 0.5 and 0.8:\n");

    searchRange(bptree, 0.500, 0.800);  
    printf("\n");

    free(records_v2);
    return 0;
}
