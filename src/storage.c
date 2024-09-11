#include "storage.h"
#include <stdlib.h>
#include <string.h>


void read_data_from_file(const char *filename, NBA_Record *records, int *num_records) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }
    char line[256];
    fgets(line, sizeof(line), file);  

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
        if (records[i].fg_pct_home >= 0.500 && records[i].fg_pct_home <= 0.800) {
            record_count_in_range++;
        }
        i++;
    }
    *num_records = i;
    // printf("Number of records within the range [0.5000, 0.800]: %d\n", record_count_in_range);
    fclose(file);
}

void store_data_to_disk(NBA_Record *records, int num_records, const char *filename, int block_size) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }
    int records_per_block = block_size / sizeof(NBA_Record);
    Block *block = (Block *)malloc(block_size);
    for (int i = 0; i < num_records; i += records_per_block) {
        int records_in_current_block = (i + records_per_block > num_records) ? (num_records - i) : records_per_block;
        block->record_count = records_in_current_block;

        memcpy(block->records, &records[i], records_in_current_block * sizeof(NBA_Record));

        int remaining_space = block_size - (sizeof(int) + records_in_current_block * sizeof(NBA_Record));
        if (remaining_space > 0) {
            memset(((char *)block->records) + records_in_current_block * sizeof(NBA_Record), 0, remaining_space);
        }

        fwrite(block, 1, block_size, file);
    }
    free(block);
    fclose(file);
}

void read_data_from_binary_file(const char *filename, NBA_Record **records, int *num_records, int block_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open binary file");
        exit(EXIT_FAILURE);
    }
    int total_records = 0;
    Block *block = (Block *)malloc(block_size);
    NBA_Record *temp_records = NULL;

    while (fread(block, 1, block_size, file) == block_size) {
        temp_records = realloc(temp_records, (total_records + block->record_count) * sizeof(NBA_Record));
        memcpy(&temp_records[total_records], block->records, block->record_count * sizeof(NBA_Record));
        total_records += block->record_count;
    }

    *records = temp_records;
    *num_records = total_records;

    free(block);
    fclose(file);
}
