#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

void read_data_from_file(const char *filename, NBA_Record *records, int *num_records) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    fgets(line, sizeof(line), file); 

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%10s %d %d %f %f %f %d %d %d",
               records[i].game_date_est,
               &records[i].team_id_home,
               &records[i].pts_home,
               &records[i].fg_pct_home,
               &records[i].ft_pct_home,
               &records[i].fg3_pct_home,
               &records[i].ast_home,
               &records[i].reb_home,
               &records[i].home_team_wins);
        i++;
    }
    *num_records = i;

    fclose(file);
}

void store_data_to_disk(NBA_Record *records, int num_records, const char *filename, int block_size) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    int records_per_block = block_size / sizeof(NBA_Record);
    NBA_Record *block = (NBA_Record *)malloc(block_size);

    for (int i = 0; i < num_records; i += records_per_block) {
        int current_block_size = (i + records_per_block > num_records) ? (num_records - i) * sizeof(NBA_Record) : block_size;
        memcpy(block, &records[i], current_block_size);
        fwrite(block, 1, block_size, file);
    }

    free(block);
    fclose(file);
}
