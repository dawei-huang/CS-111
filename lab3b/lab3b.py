import sys
import csv

def valid_block(indir_str, block_address, inode_number, starting_block, ending_block, offset):
    if block_address > ending_block or block_address < 0:
        print("INVALID " + indir_str + " "  + block_address + " IN INODE " + inode_number + " AT OFFSET " + offset)
        

def block_audit(file_list):

    block_bitmap = {}
    indirection_level = 0
    for line in file_list:
        if line[0] == "SUPERBLOCK":
            block_size = int(line[3])
            inode_size = int(line[4])

        if line[0] == "GROUP":
            num_of_blocks_in_this_group = int(line[2])
            num_of_inodes_in_this_group = int(line[3])
            first_block_inode = int(line[8])
            first_non_reserved_num = first_block_inode + inode_size*num_of_inodes_in_this_group/block_size

        if line[0] == "BFREE":
            block_num = line[1]
            block_bitmap[block_num] = True

        if line[0] == "INODE":
            starting_block = first_non_reserved_num
            ending_block = num_of_blocks_in_this_group
            block_addresses = line[12:]
            inode_number = line[1]
            len_ = len(block_addresses)
            for index, block_address in enumerate(block_addresses):
                block_address = int(block_address)
                if block_address == 0:
                    continue

                if block_address in block_bitmap:
                    print("ALLOCATED BLOCK " + block_address + " ON FREELIST")

                indir_str = ""
                if index == len_ - 3:
                    indir_str = "INDIRECT"
                if index == len_ - 2:
                    indir_str = "DOUBLE INDIRECT"
                if index == len_ - 1:
                    indir_str = "TRIPLE INDIRECT"
                indir_str = indir_str + " BLOCK"
                    
                if index < len_ - 3:
                    valid_block(indir_str, block_address, inode_number, starting_block, ending_block, 0)
                        
                elif index == len_ - 3:
                    valid_block(indir_str, block_address, inode_number, starting_block, ending_block, 12)
                    
                elif index == len_ - 2:
                    valid_block(indir_str, block_address, inode_number, starting_block, ending_block, 12 + 256)
                        
                elif index == len_ - 1:
                    valid_block(indir_str, block_address, inode_number, starting_block, ending_block, 12 + 256 + 256*256)

                    

        

        
        
        
    
def main():
    
    if len(sys.argv) != 2:
        sys.stderr.write("Must have one arguments\n")
        exit(1)

    file_system = sys.argv[1]
    with open(file_system, 'r') as file:
        file_list = csv.reader(file)
        block_audit(file_list)
    

if __name__ == '__main__':
    main()
