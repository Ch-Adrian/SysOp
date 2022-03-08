
extern void ** create_table(int amt_of_blocks){
	if(amt_of_blocks <= 0) return NULL;
	void** tab = calloc(amt_of_blocks, sizeof(void*));
	return tab;
}

extern void exec_wc(int amt, char** files){
	
	if(amt <= 0) return;
	if(system("touch tmp.txt") == -1) return;

	int cnt = 0;


}
