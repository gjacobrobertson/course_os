#include <stdint>
#include "../../include/linked_list.h"
#include "../../include/bitvector.h"
#include "../../include/open_table.h"
#include "../../include/mmci.h" //TO BE MADE
#include "../../include/klibc.h"

//NOTES:

//STRUCTS:
struct superblock
{
	char* fs_name; // 32 bytes (max length for this field abides by MAX_NAME_LENGTH)
	int fs_version; // 36 bytes
	int magic_num; // 40 bytes
	int sd_card_capacity; // 44 bytes
	int blocksize; // 48 bytes
	int root_inum; // 52 bytes
	int max_inodes; // 56 bytes
	int max_data_blocks; // 66 bytes
	int inode_bitmap_loc; // 70 bytes
	int data_bitmap_loc; // 74 bytes
	int start_inode_table_loc; // 78 bytes
	int start_data_blocks_loc; // 82 bytes
	// the rest of the superblock will be empty for now (BLOCKSIZE - 82 = 512 - 82 = 430 free/wasted bytes)
};

struct metadata
{
	char* fname;
	list inode;
	int size;
	bitvector perms; // a bitvector of length three to track: read, write, execute
	time creation_time; // need to look up CourseOS specific data type	
};


struct directory
{
	struct metadata meta;
};

struct file
{
	struct metadata meta;
	void* data_blocks[];
};

//CONSTANTS:
const int SUPERBLOCK = 1;
const int MAX_NAME_LENGTH = 32;
const int MAX_BLOCKS;
conts int BLOCKSIZE = 512;

struct superblock FS;

// const char* FS_NAME;
// const int FS_VERSION;
// const int MAGIG_NUM;
// const int SD_CARD_CAPACITY;
// const int BLOCKSIZE;
// const int ROOT_INUM;
// const int MAX_INODES;
// const int MAX_DATA_BLOCKS;
// const int ROOT_INUM;
// const int INODE_BITMAP_LOC;
// const int DATA_BITMAP_LOC;
// const int START_INDOE_TABLE_LOC;
// const int START_DATA_BLOCKS_LOC;

// initialize the filesystem:
int kfs_init(){
	//initialize the SD Card driver:
	init_sd();
	//read in the super block from disk and store in memory:
	void* superblock_spaceholder = kmalloc(BLOCKSIZE);
	receive(superblock_spaceholder, (SUPERBLOCK*BLOCKSIZE));
	os_size_t size_of_superblock = sizeof(struct superblock);
	void* fs_superblock_temp = kmalloc(size_of_superblock);
	// os_memcpy(superblock_spaceholder, fs_superblock_temp, size_of_superblock);
	struct superblock* fs_superblock = (struct superblock*) fs_superblock_temp;
	FS = *fs_superblock;

	//initialize the free list by grabbing it from the SD Card:
	
	FS->inode_bitmap_loc


}


int kopen(char* filepath, char mode){
	int fd;

	// TODO reach filepath & return pointer to struct file
	//struct file* f = returned file
	bitvector *p = f->perms;
	switch mode{
		case 'r':
			if(get(0, p) == 0){
				os_printf("File Cannot Be Read\n");
				
				return -1;
			}
			break;
		case 'w':
			if(get(1, p) == 0){
				os_printf("File Cannot Be Written\n");
				return -1;
			}
			break;
		case 'a':
			if(get(2, p) == 0){
				os_printf("File Cannot Be Appeneded To\n");
				return -1;
			}	
			break;
		case 'b':
			if(get(3, p) == 0){
				os_printf("File Cannot Be Read and Written\n");
				return -1;
			}	
			break;
		default:
			os_printf("File permission passed\n");
	}

	fd = add_to_opentable(f, mode);

	return fd;
}//end kopen()


// Helper function for kread():
int read_partial_block(int bytesLeft, void* buf_offset, file_descriptor* fd, void* transferSpace) {
	int local_offset = fd->offset % BLOCKSIZE; // local_offset is the leftmost point in the block

	// Actually get the data for 1 block (the SD Driver will put it in transferSpace for us)
	int blockNum = fd->offset / BLOCKSIZE;
	int success = recieve(transferSpace, blockNum);
	if(success < 0){
	 	// failed on a block receive, therefore the whole kread fails; return failure error
	 	os_printf("failed to receive block number %d\n", numBytes);
	 	return -1;
	}//end if

	if((local_offset == 0) && (bytesLeft < BLOCKSIZE)) { 
	/*	 ___________________
		|~~~~~~~~~|			|
		--------------------- */
		// Actually move the data to the user's specified buffer...must first cast void pointers to uint32_t* pointers:
		// source is transferSpace
		// dest is users buffer

		 os_memcpy(transferSpace, buf_offset, (os_size_t) bytesLeft); 	// note, this updates the buf_offset pointer as it transfer the data
		 																// os_memcpy takes uint32_t* as arguments
		 fd->offset += bytesLeft; // update the file descriptors file offset
		 // reset transferSpace pointer
		 transferSpace -= bytesLeft;
		 return bytesLeft; // note, we are returning the number of bytes that were successfully transferred

	} else if((local_offset > 0) && (bytesLeft >= (BLOCKSIZE - local_offset)) {
	/*	_____________________
		|           |~~~~~~~~|
		---------------------- */
		// Actually move the data to the user's specified buffer...must first cast void pointers to uint32_t* pointers:
		// source is transferSpace
		// dest is users buffer

		 os_memcpy((transferSpace + local_offset), buf_offset, (os_size_t) (BLOCKSIZE - local_offset)); 	// note, this updates the buf_offset pointer as it transfer the data
		 																// os_memcpy takes uint32_t* as arguments
		 fd->offset += (BLOCKSIZE - local_offset); // update the file descriptors file offset
		 // reset transferSpace pointer
		 transferSpace -= BLOCKSIZE;
		 return (BLOCKSIZE - local_offset); // note, we are returning the number of bytes that were successfully transferred

	} else if((local_offset > 0) && (bytesLeft < (BLOCKSIZE - local_offset)){
	/*	______________________
		|      |~~~~|         |
		----------------------- */
		// Actually move the data to the user's specified buffer...must first cast void pointers to uint32_t* pointers:
		// source is transferSpace
		// dest is users buffer

		 os_memcpy((transferSpace + local_offset), buf_offset, (os_size_t) bytesLeft); 	// note, this updates the buf_offset pointer as it transfer the data
		 																// os_memcpy takes uint32_t* as arguments
		 fd->offset += bytesLeft; // update the file descriptors file offset
		 // reset transferSpace pointer
		 transferSpace -= (local_offset + bytesLeft);
		 return bytesLeft; // note, we are returning the number of bytes that were successfully transferred

	} else{
		//this should never happen...print for debugging. TODO: remove after debugged
		os_printf("Error! In f1() in kread()...this should never happend!");
		return 0;
	}//end if else block
}//end of read_partial_block() helper function


// Helper function for kread():
int read_full_block(int bytesLeft, void* buf_offset, file_descriptor* fd, void* transferSpace) {
	// read BLOCKSIZE
	// Actually get the data for 1 block (the SD Driver will put it in transferSpace for us)
	int blockNum = fd->offset / BLOCKSIZE;
	int success = recieve(transferSpace, blockNum);
	if(success < 0){
	 	// failed on a block receive, therefore the whole kread fails; return failure error
	 	os_printf("failed to receive block number %d\n", blockNum);
	 	return -1;
	}//end if
	/*	______________________
		|~~~~~~~~~~~~~~~~~~~~~|
		----------------------- */
	// Actually move the data to the user's specified buffer...must first cast void pointers to uint32_t* pointers:
	// source is transferSpace
	// dest is users buffer
	 os_memcpy(transferSpace, buf_offset, (os_size_t) BLOCKSIZE); 	// note, this updates the buf_offset pointer as it transfer the data
	 																// os_memcpy takes uint32_t* as arguments
	 fd->offset += BLOCKSIZE; // update the file descriptors file offset
	 // reset transferSpace pointer
	 transferSpace -= BLOCKSIZE;
	 return BLOCKSIZE; // note, we are returning the number of bytes that were successfully transferred
}//end read_full_block() helper function


/* read from fd, put it in buf, then return the number of bytes read in numBytes */
int kread(int fd_int, void* buf, int numBytes) {
	int bytes_read = 0;
	uint32_t* buf_offset = buf; //this allows us to move data incrementally to user's buf via buf_offset
	//while retaining the original pointer to return back to the user
	file_descriptor* fd = get_file_descriptor(fd_int); // note, get_file_descriptor() function has not yet been implemented...will be in open_table.c

	if (fd->permission != 'r' || fd->permission != 'b') {
		os_printf("no permission \n");
		return -1;
	}

	// Allocate space for and create a bitvector to be used repeatedly to transfer the data:
	uint32_t* transferSpace = kmalloc(BLOCKSIZE);
	int blockNum = 0;

	// start of higher-level algo:
	int bytes_read = 0;
	if(numBytes < BLOCKSIZE) {
		while(bytes_read < numBytes) {
			bytes_read += read_partial_block((numBytes-bytes_read), buf_offset, fd, transferSpace);
		}
	} else if(numBytes >= BLOCKSIZE) {
		bytes_read += read_partial_block((numBytes-bytes_read), buf_offset, fd, transferSpace);
		while((numBytes - bytes_read) > BLOCKSIZE) {
			bytes_read += read_full_block((numBytes-bytes_read), buf_offset, fd, transferSpace);
		}
		if(bytes_read < numBytes) {
			bytes_read += read_partial_block((numBytes-bytes_read), buf_offset, fd, transferSpace);
		}
	}//end else if
	if(bytes_read != numBytes){
		return bytes_read;
	}else{
		return -1;
	}	
} // end kread();



/* write from fd, put it in buf, then return the number of bytes written in numBytes */
int kwrite(int fd_int, void* buf, int num_bytes) {
	int bytes_written;
        file_descriptor* fd = get_file_descriptor(fd_int);
        if (fd->permission != 'w' || fd->permission != 'b') {
                os_printf("no permission \n");
                return -1;
        }

    int total_bytes_left = num_bytes;
    int bytes_written = 0;

    uint32_t* buf_offset = buf;
    uint32_t* transferSpace = kmalloc(BLOCKSIZE);
    // os_memcpy(transferSpace, buf_offset, (os_size_t) BLOCKSIZE); 

    // have offset in the file already, just need to move it and copy.
    // fd->offset is the offset in the file. 
    int blockNum;
    int bytes_left_in_block;
    while(bytes_written < total_bytes_left) {
		blockNum = fd->offset / BLOCKSIZE;
		// need to put things in transferSpace, move pointer back when done
		bytes_left_in_block = BLOCKSIZE - (fd->offset % BLOCKSIZE);
		if(total_bytes_left <= bytes_left_in_block){
			/*	--------------- 			-----------------				
				|~~~~~~~|      |	 OR		|     |~~~~~|   |
				----------------			-----------------
			*/ 
			// write total_bytes_left
			os_memcpy(buf_offset, transferSpace, (os_size_t) total_bytes_left);
			transferSpace -= total_bytes_left;
			// pointer to start, blockNum, where we are in file, length of write
			transmit(transferSpace, blockNum, fd->offset, total_bytes_left);

			bytes_written += total_bytes_left;
			total_bytes_left -= total_bytes_left;
			fd->offset += total_bytes_left;
		}
		else if(bytes_left_in_block <= total_bytes_left) {
			/* 
				------------
				|	    |~~~|
				------------
				read to the end of the block
			*/
			os_memcpy(buf_offset, transferSpace, (os_size_t) bytes_left_in_block);
			transferSpace -= bytes_left_in_block;
			// pointer to start, blockNum, where we are in file, lengh of write
			transmit(transferSpace, blockNum, fd->offset, bytes_left_in_block);

			bytes_written += bytes_left_in_block;
			total_bytes_left -= tbytes_left_in_block;
			fd->offset += bytes_left_in_block;
		} else {
			os_memcpy(buf_offset, transferSpace, (os_size_t) BLOCKSIZE);
			transferSpace -= BLOCKSIZE;
			// pointer to start, blockNum, where we are in file, lengh of write
			transmit(transferSpace, blockNum, fd->offset, BLOCKSIZE);

			bytes_written += BLOCKSIZE;
			total_bytes_left -= BLOCKSIZE;
			fd->offset += BLOCKSIZE;
		}
	}
	return bytes_written;
} // end kwrite();



/* close the file fd, return 1 if the close was successful */
int kclose(int fd) {
	int error;
	error = delete_from_opentable(fd);
	return error;
} // end kclose();



/* seek within the file, return an error if you are outside the boundaries */
int kseek(int fd_int, int num_bytes) {
	int error;
        file_descriptor* fd = get_file_descriptor(fd_int);
        if (fd->permission != 'r' || fd->permission != 'w' || fd->permission != 'b') {
                os_printf("no permission \n");
                return -1;
        }
	fd->offset += num_bytes;	
	return 0;
} // end kseek();



/* create a new file, if we are unsuccessful return -1 */
int kcreate(char* filepath, int mode) {
	int error;

	return error;
} // end kcreate();



/* delete the file with the path filepath. Return -1 if the file does not excist */
int kdelete(char* filepath) {
	int error;
	
	return error;
} // end kdelete();




