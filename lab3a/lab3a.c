
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <math.h>

int main(int argc, char **argv) {

  if (argc != 2) {
    perror("Please enter one and only one argument.\n");
    exit(1);
  }

  int fs_des;

  if ((fs_des = open(argv[1], O_RDONLY)) == -1) {
    perror("file system does not exist.\n");
    exit(1);
  }

  int summary_output = creat("summary.csv", S_IRWXU);

  /*Superblock summary*/
  struct ext2_super_block super_block;
  const int SUPER_BLOCK_OFFSET = 1024;
  pread(fs_des, &super_block, sizeof(struct ext2_super_block), SUPER_BLOCK_OFFSET);
  int total_num_of_inodes = super_block.s_inodes_count;
  int total_num_of_blocks = super_block.s_blocks_count;
  int block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
  int inode_size = super_block.s_inode_size;
  int blocks_per_group = super_block.s_blocks_per_group;
  int inodes_per_group = super_block.s_inodes_per_group;
  int first_unreserved_i = super_block.s_first_ino;
  dprintf(summary_output, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", total_num_of_inodes, total_num_of_blocks, block_size, inode_size, blocks_per_group, inodes_per_group, first_unreserved_i);


  /*Group summary*/
  int group_size = total_num_of_blocks/blocks_per_group;
  int last_group_blocks = total_num_of_blocks%blocks_per_group;
  int last_group_inodes = total_num_of_inodes%inodes_per_group;
  
  struct ext2_group_desc* group_d;
  group_d = malloc(sizeof(struct ext2_group_desc)*(group_size+1));
  int group_offset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);
  int group_num = -1;
  int total_num_of_blocks_in_group = -1;
  int total_num_of_inodes_in_group = -1;
  int free_blocks_in_group = -1;
  int free_inodes_in_group = -1;
  int block_bitmap_in_group = -1;
  int inodes_bitmap_in_group = -1;
  int first_inode_in_group = -1;
  fprintf(stderr, "group size:%d\n", group_size);
  int i = 0;
  for(; i < group_size; i++) {
    pread(fs_des, &group_d[i], sizeof(struct ext2_group_desc), group_offset + i*sizeof(struct ext2_group_desc));
    group_num = i;
    total_num_of_blocks_in_group = blocks_per_group;
    total_num_of_inodes_in_group = inodes_per_group;
    free_blocks_in_group = group_d[i].bg_free_blocks_count;
    free_inodes_in_group = group_d[i].bg_free_inodes_count;
    block_bitmap_in_group = group_d[i].bg_block_bitmap;
    inodes_bitmap_in_group = group_d[i].bg_inode_bitmap;
    first_inode_in_group = group_d[i].bg_inode_table;
    dprintf(summary_output, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", group_num, total_num_of_blocks_in_group, total_num_of_inodes_in_group, free_blocks_in_group, free_inodes_in_group, block_bitmap_in_group, inodes_bitmap_in_group, first_inode_in_group); 
  }

  //for the last remaining group_desc
  pread(fs_des, &group_d[i], sizeof(struct ext2_group_desc), group_offset + i*sizeof(struct ext2_group_desc));
  group_num = i;
  total_num_of_blocks_in_group = (last_group_blocks == 0)? blocks_per_group:last_group_blocks;
  total_num_of_inodes_in_group = (last_group_inodes == 0)? inodes_per_group: last_group_inodes;
  free_blocks_in_group = group_d[i].bg_free_blocks_count;
  free_inodes_in_group = group_d[i].bg_free_inodes_count;
  block_bitmap_in_group = group_d[i].bg_block_bitmap;
  inodes_bitmap_in_group = group_d[i].bg_inode_bitmap;
  first_inode_in_group = group_d[i].bg_inode_table;
  dprintf(summary_output, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", group_num, total_num_of_blocks_in_group, total_num_of_inodes_in_group, free_blocks_in_group, free_inodes_in_group, block_bitmap_in_group, inodes_bitmap_in_group, first_inode_in_group);

  /*Free block entries*/  
  
  //for each group
  i = 0;
  perror("foo\n");
  for(; i < group_size; i++) {
    char* bitmap_buffer = malloc(block_size);
    pread(fs_des, bitmap_buffer,block_size, group_d[i].bg_block_bitmap * (block_size));
	
    //char* compare_bitmap = malloc(block_size);
    //bzero(compare_bitmap, block_size);
    //compare_bitmap[block_size-1] = 1;
	
    //for each block
    int j = 0;
    for(; j < block_size; j++) {
      //if (*(bitmap_buffer) & *(compare_bitmap)) {
      //dprintf(;
			
      //for each bit
      int k = 0;
      int bitmap_cmp = 1;
      for (; k < 8; k++) {
	if ((bitmap_buffer[j] & bitmap_cmp) == 0) {
	  ///
	  dprintf (summary_output, "BFREE,%d\n", (i*blocks_per_group) + (j*8) + k + 1);
	}
	bitmap_cmp = bitmap_cmp << 1;
      }
    }    
  }

  /*I-node entries*/
  i = 0;
  for(; i < group_size; i++) {
    char* bitmap_buffer = malloc(block_size);
    pread(fs_des, bitmap_buffer,block_size, group_d[i].bg_inode_bitmap * (block_size));
	
    //char* compare_bitmap = malloc(block_size);
    //bzero(compare_bitmap, block_size);
    //compare_bitmap[block_size-1] = 1;
	
    //for each inodes
    int j = 0;
    for(; j < block_size; j++) {
      //if (*(bitmap_buffer) & *(compare_bitmap)) {
      //dprintf(;
			
      //for each bit
      int k = 0;
      int bitmap_cmp = 1;
      for (; k < 8; k++) {
	if ((bitmap_buffer[j] & bitmap_cmp) == 0) {
	  ///
	  dprintf (summary_output, "BFREE,%d\n", (i*inodes_per_group) + (j*8) + k + 1);
	}
	bitmap_cmp = bitmap_cmp << 1;
      }
    }    
  }
  
  /*Inode summary*/
  int inode_offset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block) + group_size*sizeof(struct ext2_group_desc);
  
  
  return 0;
}
