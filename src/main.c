#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <time.h>
#include "storage.h"
#include "bptree.h"

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

    printf("Size of a record: %d bytes\n", record_size);
    printf("Number of records: %d\n", num_records);
    printf("Records per block: %d\n", records_per_block);
    printf("Number of blocks required: %d\n", num_blocks);

    free(records);

    read_data_from_binary_file("nba_data.bin", &records, &num_records);

    // Print the 103rd record (index 102) to verify correctness
    printNBARecord(&records[102]);

    BPlusTree *bptree = createBPlusTree();
    for (int i = 0; i < num_records; i++) {
        insert(bptree, (int)(records[i].fg_pct_home * 1000), &records[i]);  
    }

    printf("B+ Tree Statistics:\n");
    printf("1. Degree of B+ Tree: %d\n", DEGREE);
    printf("2. Number of nodes: %d\n", countNodes(bptree->root)); 
    printf("3. Number of levels: %d\n", treeHeight(bptree->root)); 
    printf("4. Content of the root node: ");
    printRootKeys(bptree);  

    printf("\n\nQuerying B+ Tree for FG_PCT_home between 0.5 and 0.8:\n");
    clock_t start = clock();
    searchRange(bptree, 500, 800);  
    clock_t end = clock();

    printf("Query time: %lf seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);

    free(records);
    return 0;
}
