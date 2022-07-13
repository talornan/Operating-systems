#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BLOCK_SIZE 4096
#define NUM_OF_DIRECT_BLOCKS 10
#define DIR_TYPE 1
#define FILE_TYPE 2
#define MAX_FILE_NAME 24
#define MAX_DIR_FILE_NUMBER 24
#define MAX_FILES 10000
#define MAX_PATH_DEPTH	1024
//the scructure of the Disc to be mounted
struct Disc{
	FILE *file;			//file pointed that simulating the disc
	int size;			//max size of disc(file)
	int blockSize;		//disc's block size
	int blocksNmuber;	//total number of blocks on disc
	bool mounted;		//is the disc mounted to File System?
	unsigned int blocksNumber_forBlocksMap;	//the number of blocks reserved for Blocks Map
    unsigned int firstBlockUsedByBlocksMap;	//the first block used by Blocks Map
	unsigned char blocksMap[BLOCK_SIZE];	//the blocks Map - to sign for free and used blocks
};
//the super block - 1'st block  on disc - to include general information
struct SuperBlock{
    unsigned int blocksNmuber;			//total number of blocks on disc
    unsigned int usedBlocks;			//blocls used
    unsigned int inodesNumber;			//the number of inode-sb_type
    unsigned int usedInodesNumber;		//the number of used inode-sb_type
    unsigned int blocksUsedByInodes;	//the number of blocks used inode-sb_type
};
//the inode structure
struct Inode{
	bool 			used;	//if inode used or not
    unsigned int 	size; 	//size of file/dir described by inode
    unsigned int 	blocks;	//num of blocks belongs to file/dir
	unsigned char 	type;	//file or dir
	unsigned int	flags;	//read/write
    unsigned int 	directs [NUM_OF_DIRECT_BLOCKS];	//directly pointed blocks
    unsigned int 	indirect; //indirectly pointed blocks
}; 
//file access mode
#define FREAD		O_RDONLY	
#define FWRITE		O_WRONLY
#define FREADWRITE	O_RDWR
#define FCREATE		O_CREAT
#define FAPPEND		O_APPEND
//file seek base
#define MYSEEK_SET SEEK_SET//: It denotes starting of the file.
#define MYSEEK_END SEEK_CUR//: It denotes end of the file.
#define MYSEEK_CUR SEEK_END//: It denotes file pointer’s current position.
//the file descriptor
struct Descriptor{
	char 	 		name[MAX_FILE_NAME];	//file's name
	int 	 		inode;					//inode number
	off_t			offset;					//offset value (0..file size). Set to 0 with opening/closing the file or by mylseek()
};
//the file structure
struct file{
	struct Descriptor	files[MAX_DIR_FILE_NUMBER];	//index #0 is the file/dir itself
    struct Descriptor 	parent;						//parent's descriptor
}; 

//the dir structure
typedef struct dir{
    struct file File;	
	int 		dirent_index;	//current index to directory's entry. Set to 0 by opendir/closedir. Can be changed by myreaddir()
} myDIR; 
//the dirent entry
struct mydirent{
	struct Descriptor dirent;
}; 

//the File System structure
struct  myFileSystem
{
	bool		 initialized;					//if initialized?
	struct Disc  disc;							//the disc used
    unsigned int fileSystemSize;				//the FS syze
    unsigned int fileSystemPhysicalSize;				//the FS Physical size
    unsigned int blocksNmuber;					//FS's blocks number
    unsigned int inodesNumber;					//FS's inodes number
    unsigned int blockSize;						//FS's block size
    unsigned int inodeSize;						//inode size
    unsigned int firstBlockUsedByInodes;		//the first Block number Used By Inodes
    unsigned int blocksUsedByInodes;			//the blocks number Used By Inodes 
    unsigned int numOfInodesPerBlock;			//the number Of Inodes Per Block
	unsigned int blocksNumber_forBlocksMap;		//the blocks Number for Blocks Map
    unsigned int firstBlockUsedByBlocksMap;		//the first Block number Used By Blocks Map
    unsigned int firstBlockUsedByDataBlock;		//the first Block Used By Data Blocks
    myDIR 		 currentDir;					//the current dir
    myDIR 		 myOpenDirs[MAX_FILES];			//a list of open dir-s
	struct file	 myOpenFiles[MAX_FILES];		//a list of open files
	struct Inode myInodes[MAX_FILES];			//a list of inode-s
};


int initDisk(const char *name, int size);
int readBlocDisc(int blockNum, char *buffer);
int writeBlocDisc(int blockNum, const char *buffer );
int getFileSizeByDesc(int myfd);
int set0FileSizeByDesc(int myfd);
myDIR *myopendir_completeRootcreation(char *name);

int mymkfs(int size);
int mymount(const char *source, const char *target,const char *filesystemtype, unsigned long mountflags, const void *data);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
ssize_t myread(int myfd, void *buf, size_t count);
ssize_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
int myclosedir(myDIR *dirp);
struct mydirent *myreaddir(myDIR *dirp);

void printDir(char *name);

int debug=0;
struct Disc myDisc;
struct  myFileSystem myFS;
int oldDisc=0;
