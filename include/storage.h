#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>

#pragma pack(1)  

typedef struct {
    char game_date_est[11]; 
    int team_id_home;
    int pts_home;
    float fg_pct_home;
    float ft_pct_home;
    float fg3_pct_home;
    short ast_home;
    short reb_home;
    short home_team_wins;
} NBA_Record;

typedef struct {
    int record_count;  
    NBA_Record records[];  
} Block;

#pragma pack()  

void read_data_from_file(const char *filename, NBA_Record *records, int *num_records);
void store_data_to_disk(NBA_Record *records, int num_records, const char *filename, int block_size);
void read_data_from_binary_file(const char *filename, NBA_Record **records, int *num_records);
void printNBARecord(NBA_Record *record);

#endif 
