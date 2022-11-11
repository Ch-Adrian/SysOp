
#define EXIT_OUT_OF_RANGE 3
#define EXIT_NULL 2
#define EXIT_OK 1
#define EXIT_ERROR 0

#define free_mem(ptr) free(ptr); ptr = NULL;

typedef struct MemoryBlock{
    char* results;
    int size;
} MBlock;

typedef struct Table{
    MBlock** blocks;
    int size;
} Tab;

Tab* create_table(int amt_of_blocks);
char* exec_cmd_wc_on_files(int amt, char** files);
short create_block_from_file(char* temporary_file, Tab* tab);
short remove_block_from_table(int block_idx, Tab* tab);

short deallocate_table(Tab* tab);
short create_block_b(int amt_of_bytes, int block_idx, Tab* tab);
