#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ext2_fs.h"

const int SUPER_BLOCK_OFFSET = 1024;

int fileSystemDescriptor;
int blockCount;
int blocksPerGroup;
int inodesPerGroup;
int numberOfGroups;
int blockSize;
int inodeCount;
int inodeSize;

struct ext2_group_desc* groupDescriptor;

void printSuperblocks()
{
  struct ext2_super_block superblock;

  // Superblock is always located at byte offset 1024
  pread(fileSystemDescriptor, &superblock, sizeof(struct ext2_super_block), SUPER_BLOCK_OFFSET);

  // Make these global variables because we'll need them in other functions
  blockCount = superblock.s_blocks_count;
  inodeCount = superblock.s_inodes_count;
  blocksPerGroup = superblock.s_blocks_per_group;
  inodesPerGroup = superblock.s_inodes_per_group;
  blockSize = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;
  inodeSize = superblock.s_inode_size;

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
      inodeCount,
      blockSize,
      inodeSize,
      blocksPerGroup,
      inodesPerGroup,
      superblock.s_first_ino);
}

void printGroups()
{
  /* Group summary */
  numberOfGroups = blockCount / blocksPerGroup;

  groupDescriptor = malloc(sizeof(struct ext2_group_desc) * (numberOfGroups + 1));
  int groupOffset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);

  for (int i = 0; i <= numberOfGroups; i++) {
    pread(fileSystemDescriptor, &groupDescriptor[i], sizeof(struct ext2_group_desc), groupOffset + i * sizeof(struct ext2_group_desc));

    /*
    1. GROUP
    2. group number (decimal, starting from zero)
    3. total number of blocks in this group (decimal)
    4. total number of i-nodes in this group (decimal)
    5. number of free blocks (decimal)
    6. number of free i-nodes (decimal)
    7. block number of free block bitmap for this group (decimal)
    8. block number of free i-node bitmap for this group (decimal)
    9. block number of first block of i-nodes in this group (decimal)
    */
    printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",
        i,
        blocksPerGroup,
        inodesPerGroup,
        groupDescriptor[i].bg_free_blocks_count,
        groupDescriptor[i].bg_free_inodes_count,
        groupDescriptor[i].bg_block_bitmap,
        groupDescriptor[i].bg_inode_bitmap,
        groupDescriptor[i].bg_inode_table);
  }
}

void printFreeBlockEntries()
{
  /*
  1. BFREE
  2. number of the free block (decimal)
  */

  struct ext2_group_desc* groupDescriptor;
  groupDescriptor = malloc(sizeof(struct ext2_group_desc) * (numberOfGroups + 1));
  int groupOffset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);

  //for each groups
  for (int i = 0; i <= numberOfGroups; i++) {
    pread(fileSystemDescriptor, &groupDescriptor[i], sizeof(struct ext2_group_desc), groupOffset + i * sizeof(struct ext2_group_desc));

    __u32 bitmap = groupDescriptor[i].bg_block_bitmap;
    char buffer;

    //for each bits in bitmap
    for (int j = 0; j < blockSize; j++) {
      pread(fileSystemDescriptor, &buffer, 1, bitmap * blockSize + j);

      char compare = 1;
      for (int k = 0; k < 8; k++) {
        if ((buffer & compare) == 0) {
          printf("BFREE,%d\n", (i * blocksPerGroup) + (j * 8) + k + 1);
        }

        compare = compare << 1;
      }
    }
  }
}

void printFreeInodeEntries()
{

  //for each groups
  for (int i = 0; i <= numberOfGroups; i++) {

    __u32 bitmap = groupDescriptor[i].bg_inode_bitmap;
    char buffer;

    //for each bits in bitmap
    for (int j = 0; j < inodeSize; j++) {
      pread(fileSystemDescriptor, &buffer, 1, bitmap * blockSize + j);

      char compare = 1;
      for (int k = 0; k < 8; k++) {
        if ((buffer & compare) == 0) {
          printf("IFREE,%d\n", (i * inodesPerGroup) + (j * 8) + k + 1);
        }

        compare = compare << 1;
      }
    }
  }
}

void formatInodeTime(__u32 time, char* timeString)
{
  struct tm GMTTime = *gmtime(&(const time_t)time);
  strftime(timeString, 80, "%m/%d/%y %H:%M:%S", &GMTTime);
}

void printInodes()
{
  struct ext2_group_desc* groupDescriptor;
  groupDescriptor = malloc(sizeof(struct ext2_group_desc) * (numberOfGroups + 1));
  int groupOffset = SUPER_BLOCK_OFFSET + sizeof(struct ext2_super_block);

  for (int i = 0; i < numberOfGroups; i++) {
    __u32 inodeTable = groupDescriptor[i].bg_inode_table;
    struct ext2_inode inode;

    for (int inodeNumber = 2; inodeNumber < inodesPerGroup; inodeNumber++) {
      off_t offset = BLOCK_OFFSET(gds[i].bg_inode_table) + (inodeNumber - 1) * sizeof(struct ext2_inode);

      pread(fileSystemDescriptor, &inode, sizeof(struct ext2_super_block), SUPER_BLOCK_OFFSET);

      inode.i_mode;

      // File Type
      char fileType = '?';
      if (inode.i_mode == EXT2_S_IFDIR) { // Directory
        fileType = 'd';
      } else if (inode.i_mode == EXT2_S_IFLNK) {
        fileType = 's';
      } else if (inode.i_mode == EXT2_S_IFREG) {
        fileType = 'f';
      }

      uint16_t mode = inode.i_mode;

      // Times
      char changeTime[20];
      char modificationTime[20];
      char lastAccessTime[20];

      formatInodeTime(inode.i_ctime, changeTime);
      formatInodeTime(inode.i_mtime, modificationTime);
      formatInodeTime(inode.i_atime, lastAccessTime);

      /*
      1. INODE
      2. inode number (decimal)
      3. file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
      4. mode (low order 12-bits, octal ... suggested format "0%o")
      5. owner (decimal)
      6. group (decimal)
      7. link count (decimal)
      8. time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
      9. modification time (mm/dd/yy hh:mm:ss, GMT)
      10. time of last access (mm/dd/yy hh:mm:ss, GMT)
      11. file size (decimal)
      12. number of blocks (decimal)
      */
      printf("INODE,%d,%c,0%o,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
          inodeNumber,
          fileType,
          mode,
          inode.i_uid,
          inode.i_gid,
          inode.i_links_count,
          changeTime,
          modificationTime,
          lastAccessTime,
          inode.i_size,
          inode.i_blocks);
    }
  }

  /*
  struct ext2_group_desc* groupDescriptor;
  groupDescriptor = malloc(sizeof(struct ext2_group_desc) * (numberOfGroups + 1));
  

 

    

    for (int j = 0; j <)
  }
  */

  // go through groups
  // get inode table for each group
  // get each inode from table

  /*
  1. INODE
  2. inode number (decimal)
  3. file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
  4. mode (low order 12-bits, octal ... suggested format "0%o")
  5. owner (decimal)
  6. group (decimal)
  7. link count (decimal)
  8. time of last I-node change (mm/dd/yy hh:mm:ss, GMT)
  9. modification time (mm/dd/yy hh:mm:ss, GMT)
  10. time of last access (mm/dd/yy hh:mm:ss, GMT)
  11. file size (decimal)
  12. number of blocks (decimal)
  */
}

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
  printGroups();

  printFreeBlockEntries();
  printFreeInodeEntries();
  /*printInodes();
  printDirectoryEntries();
  printIndirectBlockReferences();
  */

  exit(0);
}
