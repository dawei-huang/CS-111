#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "ext2_fs.h"

int fileSystemDescriptor;

int blockCount;
int blocksPerGroup;
int inodesPerGroup;
int numberOfGroups;
int blockSize;

void printSuperblocks()
{
  struct ext2_super_block superblock;

  // Superblock is always located at byte offset 1024
  pread(fileSystemDescriptor, &superblock, sizeof(struct ext2_super_block), 1024);

  // Make these global variables because we'll need them in other functions
  blockCount = superblock.s_blocks_count;
  blocksPerGroup = superblock.s_blocks_per_group;
  inodesPerGroup = superblock.s_inodes_per_group;
  blockSize = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;

  /*
  1. SUPERBLOCK
  2. total number of blocks (decimal) (s_blocks_count)
  3. total number of i-nodes (decimal) (s_inodes_count)
  4. block size (in bytes, decimal) (EXT2_MIN_BLOCK_SIZE << s_log_block_size)
  5. i-node size (in bytes, decimal) (s_inode_size)
  6. blocks per group (decimal) (s_blocks_per_group)
  7. i-nodes per group (decimal) (s_inodes_per_group)
  8. first non-reserved i-node (decimal) (s_first_ino)
  */
  printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
      blockCount,
      superblock.s_inodes_count,
      blockSize,
      superblock.s_inode_size,
      blocksPerGroup,
      superblock.s_inodes_per_group,
      superblock.s_first_ino);
}

// void printGroups()
// {
//   /* Group summary */
//   numberOfGroups = blockCount / blocksPerGroup;

//   struct ext2_group_desc* groupDescriptor;
//   groupDescriptor = malloc((struct ext2_group_desc) * (numberOfGroups + 1));
//   int groupOffset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);

//   for (int i = 0; i < numberOfGroups; i++) {
//     pread(fileSystemDescriptor, &groupDescriptor[i], groupOffset + i * sizeof(struct ext2_group_desc));

//     /*
//     1. GROUP
//     2. group number (decimal, starting from zero)
//     3. total number of blocks in this group (decimal)
//     4. total number of i-nodes in this group (decimal)
//     5. number of free blocks (decimal)
//     6. number of free i-nodes (decimal)
//     7. block number of free block bitmap for this group (decimal)
//     8. block number of free i-node bitmap for this group (decimal)
//     9. block number of first block of i-nodes in this group (decimal)
//     */
//     printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",
//         i,
//         blocksPerGroup,
//         inodesPerGroup,
//         groupDescriptor[i].bg_free_blocks_count,
//         groupDescriptor[i].bg_free_inodes_count,
//         groupDescriptor[i].bg_block_bitmap,
//         groupDescriptor[i].bg_inode_bitmap,
//         groupDescriptor[i].bg_inode_table);
//   }
// }

void printFreeBlockEntries()
{
  /*
  1. BFREE
  2. number of the free block (decimal)
  */

  struct ext2_group_desc* groupDescriptor;
  groupDescriptor = malloc((struct ext2_group_desc) * (numberOfGroups + 1));
  int groupOffset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);

  //for each groups
  for (int i = 0; i < numberOfGroups; i++) {
    pread(fs_des, &groupDescriptor[i], groupOffset + i * sizeof(struct ext2_group_desc));

    __u32 bitmap = groupDescriptor[i].bg_block_bitmap;
    char buffer;
    
    //for each bits in bitmap
    for (int j = 0; j < blockSize; j++) {
      pread(fs_des, &buffer, 1, bitmap*blockSize + j);

      char compare = 1;
      for (int k = 0; k < 8; k++) {
	if ((buffer & compare) == 0) {
	  printf("BFREE,%d\n", (i*blocksPerGroup) + (j*8) + k + 1);
	}

	compare = compare << 1;
      }
    }
  }
}

// void printFreeInodeEntries()
// {
//   for (int i = 0; i < group_size; i++) {
//     char* bitmap_buffer = malloc(block_size);
//     pread(fs_des, bitmap_buffer, block_size, group_d[i].bg_inode_bitmap * (block_size));

//     //char* compare_bitmap = malloc(block_size);
//     //bzero(compare_bitmap, block_size);
//     //compare_bitmap[block_size-1] = 1;

//     //for each inodes
//     int j = 0;
//     for (; j < block_size; j++) {
//       //if (*(bitmap_buffer) & *(compare_bitmap)) {
//       //dprintf(;

//       //for each bit
//       int k = 0;
//       int bitmap_cmp = 1;
//       for (; k < 8; k++) {
//         if ((bitmap_buffer[j] & bitmap_cmp) == 0) {
//           /*
//           1. IFREE
//           2. number of the free I-node (decimal)
//           */
//           dprintf(fs_des, "BFREE,%d\n", (i * inodesPerGroup) + (j * 8) + k + 1);
//         }
//         bitmap_cmp = bitmap_cmp << 1;
//       }
//     }
//   }

//   /*Inode summary*/
//   int inode_offset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block) + group_size * sizeof(struct ext2_group_desc);
// }

// void printInodes()
// {
//   /*
//   1. INODE
//   2. inode number (decimal)
//   3. file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
//   4. mode (low order 12-bits, octal ... suggested format "0%o")
//   5. owner (decimal)
//   6. group (decimal)
//   7. link count (decimal)
//   8. time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
//   9. modification time (mm/dd/yy hh:mm:ss, GMT)
//   10. time of last access (mm/dd/yy hh:mm:ss, GMT)
//   11. file size (decimal)
//   12. number of blocks (decimal)
//   */
// }

// void printDirectoryEntries()
// {
//   /*
//   1. DIRENT
//   2. parent inode number (decimal) ... the I-node number of the directory that contains this entry
//   3. logical byte offset (decimal) of this entry within the directory
//   4. inode number of the referenced file (decimal)
//   5. entry length (decimal)
//   6. name length (decimal)
//   7. name (string, surrounded by single-quotes). Don't worry about escaping, we promise there will be no single-quotes or commas in any of the file names.
//   */
// }

// void printIndirectBlockReferences()
// {
//   /*
//   1. INDIRECT
//   2. I-node number of the owning file (decimal)
//   3. (decimal) level of indirection for the block being scanned ... 1 single   indirect, 2 double indirect, 3 triple
//   4. logical block offset (decimal) represented by the referenced block. If the referenced block is a data block, this is the logical block offset of that block within the file. If the referenced block is a single- or double-indirect  block, this is the same as the logical offset of the first data block to which it refers.
//   5. block number of the (1,2,3) indirect block being scanned (decimal) ... not the highest level block (in the recursive scan), but the lower level block that contains the block reference reported by this entry.
//   6. block number of the referenced block (decimal)
//   */
// }

int main(int argc, char** argv)
{
  if (argc != 2) {
    fprintf(stderr, "Please enter one and only one argument.\n");
    exit(1);
  }

  fileSystemDescriptor = open(argv[1], O_RDONLY);
  if (fileSystemDescriptor == -1) {
    fprintf(stderr, "File system does not exist.\n");
    exit(1);
  }

  printSuperblocks();
  // printGroups();
  /*
  printFreeBlockEntries();
  printFreeInodeEntries();
  printInodes();
  printDirectoryEntries();
  printIndirectBlockReferences();
  */

  exit(0);
}
