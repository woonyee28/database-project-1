#ifndef STORAGE_H
#define STORAGE_H

typedef struct {
    char game_date_est[11]; 
    int team_id_home;
    int pts_home;
    float fg_pct_home;
    float ft_pct_home;
    float fg3_pct_home;
    int ast_home;
    int reb_home;
    int home_team_wins;
} NBA_Record;

void read_data_from_file(const char *filename, NBA_Record *records, int *num_records);
void store_data_to_disk(NBA_Record *records, int num_records, const char *filename, int block_size);

#endif
