
#define EXIT_OUT_OF_RANGE 3
#define EXIT_NULL 2
#define EXIT_OK 1
#define EXIT_ERROR 0

#define free_mem(ptr) free(ptr); ptr = NULL;

// typedef struct WordCountResult{
//     int lines;
//     int words;
//     int chars;
// } WCResult;

typedef struct MemoryBlock{
    char* results;
    int size;
} MBlock;

typedef struct Table{
    MBlock** blocks;
    int size;
} Tab;


// int get_length_memory_block(MBlock* block);
// void add_block_to_table(Tab* table);
// void add_result_to_block(MBlock* block);

Tab* create_table(int amt_of_blocks);
char* exec_cmd_wc_on_files(int amt, char** files);
short create_block_from_file(char* temporary_file, Tab* tab);
short remove_block_from_table(int block_idx, Tab* tab);

short deallocate_table(Tab* tab);
// short create_block_i(int amt_of_results, int block_idx, Tab* tab);
short create_block_b(int amt_of_bytes, int block_idx, Tab* tab);
// short create_result(int l, int w, int c, int result_idx, int block_idx, Tab* tab);
// short deallocate_result(int result_idx, int block_idx, Tab* tab);