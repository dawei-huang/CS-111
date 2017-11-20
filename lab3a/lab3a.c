

#include <math.h>

//process program's arguments
void process_args(int argc, char** argv, char* file_system_name) {
  if (argc != 2) {
    perror("Please enter one and only one argument.\n");
    exit(1);
  }
  else {
    file_system_name = malloc(strlen(argv[1]) + 1);
    file_system_name = argv[1];
  }
}

int main(int argc, char **argv) {

  char* file_system_name;
  
  process_args(argc, argv, file_system_name);

  int fs_des;

  if ((fs_des = open(file_system_name, O_RDONLY)) == -1) {
    perror("file system does not exist.\n");
    exit(1);
  }

  int summary_output = creat("summary.csv", S_IRWXU);

  /*Superblock summary*/
  struct ext2_super_block super_block;

  pread(fs_des, &super_block, sizeof(struct ext2_super_block), SUPER_BLOCK_OFFSET);
  int total_num_of_inodes = super_block.s_inodes_count;
  int total_num_of_blocks = super_block.s_blocks_count;
  int block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
  int inode_size = super_block.s_inode_size;
  int blocks_per_group = super_block.s_blocks_per_group;
  int inodes_per_group = super_block.s_inodes_per_group;
  int first_unreserved_i = super_block.s_first_ino;
  dprintf(fs_des, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d", total_num_of_inodes, total_num_of_blocks, block_size, inode_size, blocks_per_group, inodes_per_group, first_unreserved_i);


  /*Group summary*/
  int group_size = total_num_of_blocks/blocks_per_group;
  int last_group_blocks = total_num_of_blocks%blocks_per_group;
  int last_group_inodes = total_num_of_inodes%inodes_per_group;
  
  struct ext2_group_desc* group_d;
  group_d = malloc((struct ext2_group_desc)*(group_size+1));
  int group_offset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);
  int group_num = -1;
  int total_num_of_blocks_in_group = -1;
  int total_num_of_inodes_in_group = -1;
  int free_blocks_in_group = -1;
  int free_inodes_in_group = -1;
  int block_bitmap_in_group = -1;
  int inodes_bitmap_in_group = -1;
  int first_inode_in_group = -1;
  
  int i = 0;
  for(; i < group_size; i++) {
    pread(fs_des, &group_d[i], group_offset + i*sizeof(struct ext2_group_desc));
    group_num = i;
    total_num_of_blocks_in_group = blocks_per_group;
    total_num_of_inodes_in_group = inodes_per_group;
    free_blocks_in_group = group_d[i].bg_free_blocks_count;
    free_inodes_in_group = group_d[i].bg_free_inodes_count;
    block_bitmap_in_group = group_d[i].bg_block_bitmap;
    inodes_bitmap_in_group = group_d[i].bg_inode_bitmap;
    first_inode_in_group = group_d[i].bg_inode_table;
    dprintf(fs_des, "GROUP,%d,%d,%d,%d,%d,%d,%d", group_num, total_num_of_blocks_in_group, total_num_of_inodes_in_group, free_blocks_in_group, free_inodes_in_group, block_bitmap_in_group, inodes_bitmap_in_group, first_inode_in_group); 
  }

  //for the last remaining group_desc
  pread(fs_des, &group_d[i], group_offset + i*sizeof(struct ext2_group_desc));
  group_num = i;
  total_num_of_blocks_in_group = last_group_blocks;
  total_num_of_inodes_in_group = last_group_inodes;
  free_blocks_in_group = group_d[i].bg_free_blocks_count;
  free_inodes_in_group = group_d[i].bg_free_inodes_count;
  block_bitmap_in_group = group_d[i].bg_block_bitmap;
  inodes_bitmap_in_group = group_d[i].bg_inode_bitmap;
  first_inode_in_group = group_d[i].bg_inode_table;
  dprintf(fs_des, "GROUP,%d,%d,%d,%d,%d,%d,%d", group_num, total_num_of_blocks_in_group, total_num_of_inodes_in_group, free_blocks_in_group, free_inodes_in_group, block_bitmap_in_group, inodes_bitmap_in_group, first_inode_in_group);

  /*Free block entries*/

  //for each group
  i = 0;
  for(; i < group_size; i++) {
    //for each block
    void* bitmap_buffer = malloc(block_size);
    pread(fs_des, bitmap_buffer,block_size, group_d[i].bg_block_bitmap * (block_size));
    
    int j = 0;
    for(; j < block_size; j++) {
      
    }
    
  }

  
  return 0;
}
