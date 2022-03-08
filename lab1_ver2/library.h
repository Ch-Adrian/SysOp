

extern void** create_table(int amt_of_blocks);
extern void exec_wc(int amt, char** files);
extern int reserve_block(void** table);
extern void remove_block(int idx);
