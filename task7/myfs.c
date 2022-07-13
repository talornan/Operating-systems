#include "myfs.h"

//Disc , buffer operations
//chack parameters are valis
int checkDiscOp( int blockNum, const void *buffer )
{
	if(blockNum<0 || blockNum>=myDisc.blocksNmuber) {
		printf("checkDiscOp-ERROR blockNum (%d) is wrong!\n",blockNum);
		return 0;
	}
	if(!buffer) {
		printf("checkDiscOp-ERROR: null buffer pointer!\n");
		return 0;
	}
	return 1;
}
//read block from disc
int readBlocDisc(int blockNum, char *buffer){
	size_t rc;
	if( !checkDiscOp(blockNum, buffer) ){
		return 0;
	}
	//seek to block's position
	fseek(myDisc.file,blockNum*myDisc.blockSize,SEEK_SET);
	//read block, check return code
	if(debug){printf("debug:readBlocDisc: %d Bytes: blockNum=%d,offset=%d,file=%p\n",myDisc.blockSize, blockNum,blockNum*myDisc.blockSize,myDisc.file);}
	if( (rc = fread(buffer,1,myDisc.blockSize,myDisc.file))!=myDisc.blockSize) {
		printf("readBlocDisc:ERROR of reading %d Bytes: rc=%ld(%s),blockNum=%d,offset=%d,file=%p\n",myDisc.blockSize, rc,  strerror(errno),blockNum,blockNum*myDisc.blockSize,myDisc.file);
		return 0;
	}
	return myDisc.blockSize;
}
//write block to disk
int writeBlocDisc(int blockNum, const char *buffer ){
	size_t rc;
	if( !checkDiscOp(blockNum, buffer) ){
		return 0;
	}
	//seek to block's position
	fseek(myDisc.file,blockNum*myDisc.blockSize,SEEK_SET);
	//write block, check return code
	if(debug){printf("debug:writeBlocDisc: %d Bytes: blockNum=%d,offset=%d,file=%p\n",myDisc.blockSize, blockNum,blockNum*myDisc.blockSize,myDisc.file);}
	if((rc=fwrite(buffer,1,myDisc.blockSize,myDisc.file))!=myDisc.blockSize) {
		printf("writeBlocDisc:ERROR of writing %d Bytes: ret=%ld(%s)\n",myDisc.blockSize, rc, strerror(errno));
		return 0;
	}
	return myDisc.blockSize;
}
//write buffer to specific block by read block, update it and write back
int writeBuffer(int block, int offset, const char *buffer, int size){
	unsigned char buff[BLOCK_SIZE];
	int rc=0;
	if(debug){printf("debug:writeBuffer: write block %d, offset=%d, size=%d\n",block, offset, size);}
	if( size > BLOCK_SIZE){
		printf("writeBuffer:Error - attempt to write %d bytes - more than block size\n",size);
		return 0;
	}
	if( !(rc = readBlocDisc(block, (char*) buff )) ){
		return 0;
	}
	memcpy(buff + offset, buffer, size);
	if( !(rc=writeBlocDisc(block,(char*) buff )) ){
		return 0;
	}	
	return rc;
}
//read buffer from block
int readBuffer(int block, int offset, char *buffer, int size){
	unsigned char buff[BLOCK_SIZE];
	int rc=0;
	if( size > BLOCK_SIZE){
		printf("readBuffer:Error - attempt to read %d bytes - more than block size\n",size);
		return 0;
	}
	if( !(rc = readBlocDisc(block,(char*) buff )) ){
		return 0;
	}
	//extract buffer from block
	memcpy(buffer, buff + offset, size);
	if(debug){printf("debug:readBuffer: read block %d, offset=%d, size=%d\n",block, offset, size);}
	return rc;
}
//mark block in blocks memory map as used = 0/1 = free/used
int markBlock(int block, unsigned char used){
	if(debug){printf("debug:markBlock-write to block %d, offset %d, size=%d from %p\n",myFS.firstBlockUsedByBlocksMap,0, myFS.disc.blocksNmuber,myFS.disc.blocksMap);}
	myFS.disc.blocksMap[block] = used;
	if(debug){printf("debug:markBlock-write to block %d, offset %d, size=%d from %p\n",myFS.firstBlockUsedByBlocksMap,0, myFS.disc.blocksNmuber,myFS.disc.blocksMap);}
	int rc;
	if( !(rc = writeBuffer(myFS.firstBlockUsedByBlocksMap, 0, (const char *)myFS.disc.blocksMap, myFS.disc.blocksNmuber) ) ){
		printf("markBlock: Error - failed to wrtite map to block %d\n",myFS.firstBlockUsedByBlocksMap);
		return 0;
	}
	if( !used ){
		//in case block removed - write all 0 to block
		unsigned char buff[BLOCK_SIZE];
		memset(buff,0,sizeof(buff));
		writeBlocDisc(block,(char*) buff );
	}
	return rc;
}
//mark block in blocks memory map as used = 0/1 = free/used
int readFromDiskBlockMap(){
	int rc;
	if( !(rc = readBuffer(myFS.firstBlockUsedByBlocksMap, 0, (char *)myFS.disc.blocksMap, myFS.disc.blocksNmuber) ) ){
		printf("readFromDiskBlockMap: Error - failed to read blocks map from block %d\n",myFS.firstBlockUsedByBlocksMap);
		return 0;
	}

	return rc;
}
//look in blocks map for first free block
int lookForEmptyBlock(){
	for( int block = 0; block < myFS.disc.blocksNmuber; block++){
		if( !myFS.disc.blocksMap[block] ){
			return block;
		}
	}
	return -1;
}
//init Disc
int initDisk(const char *name, int size){
	printf("initDisk: Initializing Disc from %s, size = %d\n",name,size);
	struct stat st;
	int rc = stat(name, &st);
	if( !rc && st.st_size && size > 0 ){
		printf("initDisk:Disc file %s existing. Using FS from Disc file\n", name);
		oldDisc = 1;
	}
	else if( rc || (!rc && !st.st_size) ){
		printf("initDisk:Disc file %s not existing or 0 size. Will create new FS\n", name);
	}
	if( rc ){
		if( (myDisc.file = fopen(name, "wb+")) == NULL ){
		printf("initDisk:Failed to create file %s\n", name);
		return 0;
		}
	}
	else {
		if( (myDisc.file = fopen(name, "rb+")) == NULL ){
		printf("initDisk:Failed to create file %s\n", name);
		return 0;
		}
	}
	if(debug){printf("debug:initDisk- myDisc.file = %p\n",myDisc.file);}
	myDisc.size = size;
	myDisc.blockSize = BLOCK_SIZE;
	myDisc.blocksNmuber = (size / BLOCK_SIZE);
	if(myDisc.blocksNmuber <3){
		printf("initDisk:Error - too small Disc Size\n");
		fclose(myDisc.file);
		return 0;
	}
	myDisc.mounted = 0;
	return 1;
}

// inode operations
//write inode to it's position on disk
int writeInode(int inode, struct Inode *inodeBuff){
	int rc = 0;
	int block = myFS.firstBlockUsedByInodes + (inode / myFS.numOfInodesPerBlock);
	int inodeoffset = (inode % myFS.numOfInodesPerBlock) * myFS.inodeSize;	
	if(debug){printf("debug:writeInode: wrtite Inode %d to block %d, offset=%d\n",inode,block,inodeoffset);	}
	if( !(rc = writeBuffer(block, inodeoffset, (const char *)inodeBuff, sizeof(struct Inode)) ) ){
		printf("writeInode: Error - failed to wrtite Inode %d to block %d, offset=%d\n",inode,block,inodeoffset);
		return 0;
	}
	memcpy(&myFS.myInodes[inode], inodeBuff, sizeof(struct Inode));
	return rc;
}
//read inode from it's position on disk
int readInode(int inode, struct Inode *inodeBuff){
	int rc = 0;
	int block = myFS.firstBlockUsedByInodes + (inode / myFS.numOfInodesPerBlock);
	int inodeoffset = (inode % myFS.numOfInodesPerBlock) * myFS.inodeSize;	
	if(debug){printf("debug:readInode: read Inode %d from block %d, offset=%d\n",inode,block,inodeoffset);	}
	if( !(rc = readBuffer(block, inodeoffset, (char *)inodeBuff, sizeof(struct Inode)) )){
		printf("writeInode: Error - failed to wrtite Inode %d to block %d, offset=%d\n",inode,block,inodeoffset);
		return 0;
	}	
	memcpy(&myFS.myInodes[inode], inodeBuff, sizeof(struct Inode));
	if(debug){printf("debug:readInode: read inode %d from block %d, offset=%d\n",inode, block, inodeoffset);}
	return rc;
}
//look for empty inode in inodes list
int lookForEmptyInode(){
	struct Inode inodeBuff;
	for(int inodeNum=0; inodeNum < myFS.inodesNumber;inodeNum++){
		if( !readInode(inodeNum, &inodeBuff) ){
			printf("myLookForEmptyInode: Error reading inode %d\n",inodeNum);
			return -1;
		}
		if( !inodeBuff.used ){
			return inodeNum;
		}
	}
	return -1;
}


//dir/file operations
myDIR *lookForEmptyDirSlot(){
myDIR *dir = NULL;
    for(int i=0; i < MAX_FILES; i++){
        if( !strlen(myFS.myOpenDirs[i].File.files[0].name) ){
            dir = &myFS.myOpenDirs[i];
			return dir;
        }
    }
    printf("lookForEmptyDirSlot: Erorr no empty solt in myFS.myOpenDir\n");
	return NULL;
}
int isOpenedDir(const char *name){
    for(int i=0; i < MAX_FILES; i++){
        if( !strlen(myFS.myOpenDirs[i].File.files[0].name) && !strcmp(myFS.myOpenDirs[i].File.files[0].name, name)){
			printf("lookForOpenedDirSlot: Erorr no empty solt in myFS.myOpenDir\n");
			return 1;
        }
    }
    return 0;
}
int emptyDirSlot(myDIR *dir){
	if(dir == NULL){
		printf("emptyDirSlot: Error - NULL dir pointer\n");
		return 0;
	}
	memset(dir,0,sizeof(myDIR));
	return 1;
 }

int lookForEmptyFileSlot(){
    for(int i=0; i < MAX_FILES; i++){
        if(!strlen(myFS.myOpenFiles[i].files[0].name)){
            return i;
        }
    }
    printf("lookForEmptyFileSlot: Erorr no empty solt in myFS.myOpenFiles\n");
	return -1;
}
int updateFileSlot(int desc, struct file *file){
	if(desc < 0 || desc > MAX_FILES){
		printf("updateFileSlot: Error - wrong desc given %d\n", desc);
		return 0;
	}
	struct file *slot = &myFS.myOpenFiles[desc];
	memcpy(slot,(char *)file,sizeof(struct file));
	return 1;
}
int getFileSlot(int desc, struct file *file){
	if(desc < 0 || desc > MAX_FILES){
		printf("updateFileSlot: Error - wrong desc given %d\n", desc);
		return 0;
	}
	struct file *slot = &myFS.myOpenFiles[desc];
	memcpy((char *)file,slot,sizeof(struct file));
	return 1;
}
int emptyFileSlot(int desc){
	if(desc < 0 || desc > MAX_FILES){
		printf("emptyFileSlot: Error - wrong desc given %d\n", desc);
		return 0;
	}
	struct file *slot = &myFS.myOpenFiles[desc];
	memset(slot,0,sizeof(struct file));
	return 1;
}

int getDirEntryByInode(int inodeNum, myDIR *dir, bool onlyUsed){
	struct Inode inode;
	int rc;
	if( !readInode(inodeNum, &inode) ){
		printf("getDirEntryByInode: Error reading inode %d\n",inodeNum);
		return 0;
	}
	if( onlyUsed && !inode.used ){ return 0; } //not used iblock - continue with next inode
	int block = inode.directs[0];
	if( !(rc = readBuffer(block, 0, (char *)dir, sizeof(myDIR)) )){
		printf("getDirEntryByInode: Error reading dir inode %d Data block %d\n",inodeNum,block);
		return 0;
	}
	return rc;
}
int writeDirEntry_InodeByDirInode(myDIR *dir, struct Inode *inode){
	int rc;
	
	if( !(rc = writeBuffer(inode->directs[0], 0, (const char *)dir, sizeof(myDIR)) )){
		printf("writeDirEntry_InodeByDirInode: Error writing dir Data block %d\n",inode->directs[0]);
		return 0;
	}
	if( !writeInode(dir->File.files[0].inode, inode) ){
		printf("writeDirEntry_InodeByDirInode: Error writing inode %d\n",dir->File.files[0].inode);
		return 0;
	}
	return 1;
}


int getFileEntryByInode(int inodeNum, struct file *file, bool onlyUsed){
	struct Inode inode;
	int rc;
	if( !readInode(inodeNum, &inode) ){
		printf("getFileEntryByInode: Error reading inode %d\n",inodeNum);
		return 0;
	}
	if( onlyUsed && !inode.used ){ return 0; } //not used iblock - continue with next inode
	int block = inode.directs[0];
	if( !(rc = readBuffer(block, 0, (char *)file, sizeof(struct file)) )){
		printf("getFileEntryByInode: Error reading file inode %d Data block %d\n",inodeNum,block);
		return 0;
	}
	if(debug){printf("debug:getFileEntryByInode - got by inodeNum=%d: name=%s inode = %d\n",inodeNum,file->files[0].name, file->files[0].inode);}
	return rc;
}
int writeFileEntry_InodeByFileInode(struct file *file, struct Inode *inode){
	int rc;
	
	if( !(rc = writeBuffer(inode->directs[0], 0, (const char *)file, sizeof(struct file)) )){
		printf("writeFileEntry_InodeByFileInode: Error writing file Data block %d\n",inode->directs[0]);
		return 0;
	}
	if( !writeInode(file->files[0].inode, inode) ){
		printf("writeFileEntry_InodeByFileInode: Error writing inode %d\n",file->files[0].inode);
		return 0;
	}
	return 1;
}



//look for Dir name. Return myDIR*
myDIR *lookDir(char *name){
    myDIR *dir = NULL;
	char path[MAX_PATH_DEPTH][MAX_FILE_NAME];	//array will include path dir-s names
	if(debug){printf("debug:lookdir-name=%s\n",name);}
	if(name == NULL || !strlen(name)){
		printf("lookDir: Error - empty name given\n");
		return NULL;
	}
    if( !(dir = lookForEmptyDirSlot()) ){
        printf("lookDir: Erorr no empty solt in myFS.myOpenDir\n");
        return NULL;
    }
	//convert name path string to array of dir-s
	int pathindex=0;
	int fromInode=0;
	memset(path,0,sizeof(path));
	if(name[0] == '/'){
		strcpy(path[0],"/");
		pathindex=1;
	} else {
		printf("lookDir: Error - Relative path search not implemented\n");
		emptyDirSlot(dir);
		return NULL;		
	}
	//go over tokens of given path - build path array with dir name in each entry
	char *token = strtok(name , "/");
    while(token != NULL){
		if(debug){printf("debug:lookdir-token=%s out of name=%s\n",token,name);}
		if(pathindex >= MAX_PATH_DEPTH ){
			printf("lookDir: Error-path given too long (max=%d), given=%s\n",MAX_PATH_DEPTH,name);
			emptyDirSlot(dir);
			return NULL;		
		}
		strcpy(path[pathindex++],token);
		token = strtok(NULL,"/");
	}
	bool found = false;
	//look starting from / rood dir
	for(pathindex=0; pathindex < MAX_PATH_DEPTH; pathindex++){
		if(debug){printf("debug:lookdir- pattern = [%s]\n",path[pathindex]);}
		if( !strlen(path[pathindex]) ){
			printf("lookDir: Error - path %s not found\n",name);
			emptyDirSlot(dir);
			return NULL;					
		}
		found = false;
		//go over all inodes, read their Data blocks , look for same namy and DIR type
		for(int inodeNum=fromInode; inodeNum < myFS.inodesNumber;){
			if(debug){printf("debug:lookDir: look for %s in inode %d\n",path[pathindex],inodeNum);}
			if( !getDirEntryByInode(inodeNum,dir,true) ){
				//printf("lookDir: Error reading inode %d\n",inodeNum);
				emptyDirSlot(dir);
				return NULL;
			}
			//look for non-empty and non-root names and list them
			int i;
			for (i = 0; i < MAX_DIR_FILE_NUMBER; i++){
				int inodeNum = dir->File.files[i].inode;
				if(strlen(dir->File.files[i].name) && !strcmp(dir->File.files[i].name,path[pathindex]) && myFS.myInodes[inodeNum].type == DIR_TYPE){ 
					if(debug){printf("debug: lookDir: Found name %s in index %d,inode %d\n" , dir->File.files[i].name, i, inodeNum);}
					//check if we found last item in path list - return found dir
					if( pathindex + 1 < MAX_PATH_DEPTH && !strlen(path[pathindex+1])){
						if(debug){printf("debug: lookDir: Found all path %s\n" , name);}
						//fetch dound dir's block
						if( !getDirEntryByInode(dir->File.files[i].inode,dir,true) ){
							//printf("lookDir: Error reading inode %d\n",inodeNum);
							emptyDirSlot(dir);
							return NULL;
						}
						return dir;
					} else {	//else - continue to look for next item in path						
						fromInode = dir->File.files[i].inode;	
					}
					//break the look
					found = true;
					break;
				}
			}
			//all list of dir-s checked - no same one found
			if(i == MAX_DIR_FILE_NUMBER){
				if(debug){printf("debug:lookDir: Error - name %s not found - end of files list reached\n",path[pathindex]);}
				emptyDirSlot(dir);
				return NULL;
			} else { break; }
		}
		if( !found ){
			if(debug){printf("lookDir: Error - name %s not found in all inodes. last inode checked %d\n",path[pathindex],fromInode);}
			emptyDirSlot(dir);
			return NULL;
		}
    }
	printf("lookDir: Error - name %s not found adter all path checked\n",name);
	emptyDirSlot(dir);
	return NULL;
}
//User interface functions
// mount the disc to FS
int mymount(const char *source, const char *target,const char *filesystemtype, unsigned long mountflags, const void *data){
	unsigned char zeroBlock[BLOCK_SIZE];
	
	if( !initDisk(source, myFS.fileSystemSize) ){
		return 0;
	}
	myDisc.mounted = true;
	if(!oldDisc){
		memset(zeroBlock, 0, sizeof(zeroBlock));
		for (int blockNum = 0; blockNum < myDisc.blocksNmuber; blockNum++){
			if( !writeBlocDisc(blockNum, (char *) zeroBlock ) ){
				return 0;
			}
		}
	}
	//prepare blocks map
	myDisc.blocksNumber_forBlocksMap = (myDisc.blocksNmuber / BLOCK_SIZE) + 1;
	memset(&myDisc.blocksMap, 0, sizeof(myDisc.blocksNmuber));
	
	memcpy(&myFS.disc, &myDisc, sizeof(myDisc));
	myFS.fileSystemPhysicalSize = myFS.fileSystemSize;
    myFS.blocksNmuber = myFS.fileSystemSize / myDisc.blockSize;
    myFS.inodesNumber = ((myFS.blocksNmuber / 10) * myDisc.blockSize) / sizeof(struct Inode); 
	myFS.firstBlockUsedByInodes = 1;	//Inodes starts at second block - index 1

	struct SuperBlock superBlock;
	if( !oldDisc ){
		superBlock.blocksNmuber = myFS.blocksNmuber;
		superBlock.inodesNumber = myFS.inodesNumber;
		superBlock.usedInodesNumber = 0;
		superBlock.blocksUsedByInodes = myFS.blocksNmuber / 10;
		superBlock.usedBlocks = 1 + superBlock.blocksUsedByInodes;

		myFS.firstBlockUsedByInodes = 1;	//Inodes starts at second block - index 1
		myFS.blocksUsedByInodes = superBlock.blocksUsedByInodes;
		myFS.blocksNumber_forBlocksMap = myFS.disc.blocksNumber_forBlocksMap;
		myFS.firstBlockUsedByBlocksMap =  myFS.blocksUsedByInodes + 1;	//Blocks Map starts right after inodes
		myFS.firstBlockUsedByDataBlock =  myFS.blocksUsedByInodes + 1 + myFS.firstBlockUsedByBlocksMap;	//Data blocks starts right after Blocks Map
		myFS.blockSize = myDisc.blockSize; 
		myFS.inodeSize = sizeof(struct Inode);
		myFS.numOfInodesPerBlock = myFS.blockSize/myFS.inodeSize;

		if(debug){printf("debug:mymskfs builds ufs size = %d, blocks number = %d, inodes number = %d, blocksUsedByInodes = %d,blocksNumber_forBlocksMap=%d\n",
		myFS.fileSystemSize, myFS.blocksNmuber, myFS.inodesNumber, myFS.blocksUsedByInodes,myFS.blocksNumber_forBlocksMap);}
		if(debug){printf("debug:my FS : first data block = %d , first inode block = %d, first maps block= %d\n" , 
		myFS.firstBlockUsedByDataBlock,myFS.firstBlockUsedByInodes,myFS.firstBlockUsedByBlocksMap);}
		//fill super block
		if( !writeBlocDisc(0, (const char *)&superBlock ) ){
			printf("mymskfs Error writing Super Block\n");
			return 0;
		}
		struct SuperBlock superBlock2;
		if( !readBlocDisc(0, (char *)&superBlock2 ) ){
			printf("debug:mymskfs Error reading Super Block\n");
			return 0;
		}
		if( memcmp(&superBlock,&superBlock2,sizeof(struct SuperBlock)) ){
			printf("debug:mymskfs reading Super Block not same as original\n");
			return 0;
		}
		if(debug){printf("debug:mymkfs: mark super blocks from %d as %d\n",0,1);}
		//mark super block as used in block map
		markBlock(0, (unsigned char)1);		//mark block as used
		if(debug){printf("debug:mymkfs: mark indes blocks from %d to %d as %d\n",myFS.firstBlockUsedByInodes,myFS.blocksUsedByInodes,1);}
		//mark inode-s blocks as used in block map
		for(int i = myFS.firstBlockUsedByInodes; i<=myFS.blocksUsedByInodes; i++){
			markBlock(i, (unsigned char)1);		//mark block as used
		}
		if(debug){printf("debug:mymkfs: mark blocks of map from %d to %d as %d\n",myFS.firstBlockUsedByBlocksMap,myFS.blocksUsedByInodes+myFS.blocksNumber_forBlocksMap,1);}
		//mark blocks map blocks as used in block map
		for(int i = myFS.firstBlockUsedByBlocksMap; i<=(myFS.blocksUsedByInodes+myFS.blocksNumber_forBlocksMap); i++){
			markBlock(i, (unsigned char)1);		//mark block as used
		}

		myDIR *dir;
		if( !(dir = myopendir_completeRootcreation("/")) ){
			printf("mymkfs: Error creating root /\n");
			return 0;
		}
		myclosedir(dir);
		
	}
	else {
			//read super block
		if( !readBlocDisc(0, (char *)&superBlock ) ){
			printf("debug:mymskfs Error reading Super Block from old disc\n");
			return 0;
		}
		superBlock.blocksNmuber = myFS.blocksNmuber;
		superBlock.inodesNumber = myFS.inodesNumber;
		superBlock.usedInodesNumber = 0;
		superBlock.blocksUsedByInodes = myFS.blocksNmuber / 10;
		superBlock.usedBlocks = 1 + superBlock.blocksUsedByInodes;
		myFS.blocksUsedByInodes = superBlock.blocksUsedByInodes;
		myFS.blocksNumber_forBlocksMap = myFS.disc.blocksNumber_forBlocksMap;
		myFS.firstBlockUsedByBlocksMap =  myFS.blocksUsedByInodes + 1;	//Blocks Map starts right after inodes
		myFS.firstBlockUsedByDataBlock =  myFS.blocksUsedByInodes + 1 + myFS.firstBlockUsedByBlocksMap;	//Data blocks starts right after Blocks Map
		myFS.blockSize = myDisc.blockSize; 
		myFS.inodeSize = sizeof(struct Inode);
		myFS.numOfInodesPerBlock = myFS.blockSize/myFS.inodeSize;
		if(debug){printf("debug:mymskfs read ufs size = %d, blocks number = %d, inodes number = %d, blocksUsedByInodes = %d,blocksNumber_forBlocksMap=%d\n",
		myFS.fileSystemSize, myFS.blocksNmuber, myFS.inodesNumber, myFS.blocksUsedByInodes,myFS.blocksNumber_forBlocksMap);}
		if(debug){printf("debug:my FS : first data block = %d , first inode block = %d, first maps block= %d\n" , 
		myFS.firstBlockUsedByDataBlock,myFS.firstBlockUsedByInodes,myFS.firstBlockUsedByBlocksMap);}
		struct Inode inodeBuff;
		for(int inode = 0; inode < superBlock.inodesNumber; inode++){
			if( !readInode(inode, &inodeBuff) ){
				printf("debug:mymskfs Error reading Inodes from old disc\n");
				return 0;
			}
		}
		if( !readFromDiskBlockMap() ){
			printf("debug:mymskfs Error reading Blocks Map from old disc\n");
			return 0;
		}

	}

	return 1;
}
//make FS
int mymkfs(int size){
//	if( !myDisc.mounted ){
//		printf("mymkfs: Error - no disc mounted\n");
//		return 0;
//	}
	//initialize parameters
    memset(&myFS,0,sizeof(myFS));
//	memcpy(&myFS.disc, &myDisc, sizeof(myDisc));
	memset(&myFS.disc, 0, sizeof(myDisc));
    myFS.fileSystemSize = (size / BLOCK_SIZE) * BLOCK_SIZE;//myDisc.size;
	if(myFS.fileSystemSize < size){
		myFS.fileSystemSize += BLOCK_SIZE;
	}
	printf("mymkfs = Initialize FS size %d\n",myFS.fileSystemSize);
	myFS.fileSystemPhysicalSize = 0;
//    myFS.blocksNmuber = myFS.fileSystemSize / myDisc.blockSize;
//    myFS.inodesNumber = ((myFS.blocksNmuber / 10) * myDisc.blockSize) / sizeof(struct Inode); 

	memset(myFS.myOpenDirs,0,sizeof(myFS.myOpenDirs));
	memset(myFS.myOpenFiles,0,sizeof(myFS.myOpenFiles));
	memset(myFS.myInodes,0,sizeof(myFS.myInodes));
	myFS.initialized = true;
//	myDIR *dir;
//    if( !(dir = myopendir("/")) ){
//		printf("mymkfs: Error creating root /\n");
//		return 0;
//	}
//	myclosedir(dir);
	return 1;
}


// To do: support directory file size more than 1 block size
myDIR *myopendir(const char *name){
    myDIR *dir = NULL;
    myDIR *dirParent = NULL;
	char dirName[MAX_FILE_NAME];		//the dir name without path
	char tmpName[MAX_FILE_NAME];
	char parentPath[MAX_FILE_NAME];		//the string with parent dir path
    
	if( myDisc.mounted == false ){
		printf("Error: myopendir - Disc File Simulation isn't mounted yet\n");
		return NULL;
	}

	if(name == NULL){
		printf("Error: myopendir - NULL dir name given in input\n");
		return NULL;
	}
	if(isOpenedDir(name)){
		printf("Error: myopendir - the dir %s is already opened\n", name);
		return NULL;
	}
	memset(parentPath,0,sizeof(parentPath));
	//handle only absolute path - starts from /
    if(name[0] == '/'){
		
		//check if dir already existing
		memset(tmpName,0,sizeof(tmpName));
		strcpy(tmpName,name);
		//check is the dir already exists
		if( !(dir = lookDir(tmpName)) ){
			//the 'name' dir doesn't exists
			if(debug){printf("debug:myopendir- no %s found\n",name);}
			
			//in case of root - dirName=/, no parent
			if( name != NULL && strlen(name) && !strcmp(name,"/") ){
				strcpy(dirName, name);
				strcpy(parentPath,"");
			}
			//look for parent in case of non-root dir
			else if( name != NULL && strlen(name) && strcmp(name,"/") ){
				char path[MAX_PATH_DEPTH][MAX_FILE_NAME];	//array will include path dir-s names
				//convert name path string to array of dir-s
				int pathindex=0;
				strcpy(path[0],"/");
				memset(tmpName,0,sizeof(tmpName));
				strcpy(tmpName,name);
				char *token = strtok(tmpName , "/");
				while(token != NULL){
					if(++pathindex >= MAX_PATH_DEPTH ){
						printf("myopendir: Error-path given too long (max=%d), given=%s\n",MAX_PATH_DEPTH,name);
						return NULL;		
					}
					if( pathindex > 1 ){
						strcat(parentPath,"/");
					}
					strcat(parentPath,path[pathindex-1]);
					strcpy(path[pathindex],token);
					token = strtok(NULL,"/");
				}
				strcpy(dirName, path[pathindex]);
				if(debug){printf("debug:myopendir- new dir %s,look for parent path %s\n",dirName,parentPath);}
				memset(tmpName,0,sizeof(tmpName));
				strcpy(tmpName,parentPath);
				//check if parent dir exists
				if( !(dirParent = lookDir(tmpName)) ){
					//no parent dir found - exit with error
					if(debug){printf("debug:myopendir- no parent %s found\n",parentPath);}
					return NULL;
				}
				if(debug){printf("debug:myopendir- new dir %s,found parent path %s\n",dirName,parentPath);}
			}
			
			if(debug){printf("debug:myopendir- Update/set new dir %s,,parent path %s\n",dirName,parentPath);}
			//get empty slot in myOpenDirs
			if( !(dir = lookForEmptyDirSlot()) ){
				printf("myopendir: Erorr no empty dir solt in myFS.myOpenDir\n");
				return NULL;
			}			
			memset(dir, 0, sizeof(myDIR));
			//dir->File.files[0].type = DIR_TYPE;
			//fill dir and inode parameters
			strcpy (dir->File.files[0].name,dirName);
//			if( !strcmp(dirName,"/") && !myFS.fileSystemPhysicalSize){
//				return dir;
//			}
			//get empty inode
			if( (dir->File.files[0].inode = lookForEmptyInode()) == -1 ){
				printf("myopendir: Error of looking for empty Inode\n");
				emptyDirSlot(dir);
				return NULL;
			}
			struct Inode inode;
			memset(&inode,0,sizeof(inode));
			inode.used = true;
			inode.size = BLOCK_SIZE;
			inode.blocks = 1;
			inode.type = DIR_TYPE;
			//get empty Data block
			if( (inode.directs[0] = lookForEmptyBlock()) == -1){
				printf("myopendir: Error of looking for empty Block\n");
				emptyDirSlot(dir);
				return NULL;
			}
			if(debug){printf("debug:myopendir - store new dir %s in inode %d, block = %d\n",dir->File.files[0].name,dir->File.files[0].inode,inode.directs[0]);}
			//in case of parent exists - update parent's Data block with new chield dir and parent's descriptor to new dir
			bool freeSlotFoundInParentFilesList = false;
			if( strlen(parentPath) && dirParent != NULL ){
				//and parent's descriptor to new dir
				strcpy(dir->File.parent.name,dirParent->File.files[0].name);
				dir->File.parent.inode = dirParent->File.files[0].inode;
				//look for empty slot in parent's dir list of files
				for(int i = 1; i < MAX_DIR_FILE_NUMBER; i++){
					//if empty foujnd - update parent's Data block with new chield dir
					if( !strlen(dirParent->File.files[i].name) ){
						strcpy( dirParent->File.files[i].name, dirName);
						//dirParent->File.files[i].type = dir->File.files[0].type;
						dirParent->File.files[i].inode = dir->File.files[0].inode;
						freeSlotFoundInParentFilesList = true;
						if(debug){printf("debug:myopendir update parentdir %s list entry %d, val = [%s,i=%d]\n",
						dirParent->File.files[0].name,i,dirParent->File.files[i].name,dirParent->File.files[i].inode);}
						break;
					}
				}
				//the dir already full
				if( !freeSlotFoundInParentFilesList ){
					printf("myopendir: Error of looking for empty slot in parent %s (path %s) file list\n",dirParent->File.files[0].name,parentPath);
					emptyDirSlot(dir);
					return NULL;
				}
				//update parent's Dir data block
				struct Inode parentInode;
				if( !readInode(dirParent->File.files[0].inode, &parentInode) ){
					printf("myopendir: Error of reading parent's inode %d\n",dirParent->File.files[0].inode);
					emptyDirSlot(dir);
					return NULL;					
				}
				if( !writeDirEntry_InodeByDirInode(dirParent,&parentInode) ){
					printf("myopendir: Error writing parent's dir %s Data block %d, inode %d\n",parentPath,parentInode.directs[0],dirParent->File.files[0].inode);
					emptyDirSlot(dir);
					return NULL;
				}
				emptyDirSlot(dirParent);
			}
			if(debug){printf("debug:myopendir: taken inode=%d, block=%d\n",dir->File.files[0].inode,inode.directs[0]);}
			if( !writeDirEntry_InodeByDirInode(dir,&inode) ){
				printf("myopendir: Error writing dir %s Data block %d, inode %d\n",dirName,inode.directs[0],dir->File.files[0].inode);
				emptyDirSlot(dir);
				return NULL;
			}
			markBlock(inode.directs[0], 1);		//mark block as used
		}		
    } else {
		printf("Error - relative dir path not supported\n");
	}
    return dir;
}

// To do: support directory file size more than 1 block size
myDIR *myopendir_completeRootcreation(char *name){
    myDIR *dir = NULL;
	char dirName[MAX_FILE_NAME];		//the dir name without path
	char tmpName[MAX_FILE_NAME];
    
	if(name == NULL){
		printf("Error: myopendir_completeRootcreation - NULL dir name given in input\n");
		return NULL;
	}
	if(isOpenedDir(name) || strcmp(name,"/")){
		printf("Error: myopendir_completeRootcreation - the dir %s is already opened or not root / dir given in input\n", name);
		return NULL;
	}
	if(!myFS.fileSystemPhysicalSize){
		printf("Error: myopendir_completeRootcreation - disc not mounted yet\n");
		return NULL;
	}

	memset(tmpName,0,sizeof(tmpName));
	strcpy(tmpName,name);
	//check is the dir already exists
	if( !(dir = lookDir(tmpName)) ){
		if( !(dir = lookForEmptyDirSlot()) ){
			printf("lookDir: Erorr no empty solt in myFS.myOpenDir\n");
			return NULL;
		}
	}
	strcpy(dirName, name);
	if(debug){printf("debug:myopendir_completeRootcreation- Update/set new dir %s\n",dirName);}
	//get empty slot in myOpenDirs
	if( !(dir = lookForEmptyDirSlot()) ){
		printf("myopendir_completeRootcreation: Erorr no empty dir solt in myFS.myOpenDir\n");
		return NULL;
	}			
	memset(dir, 0, sizeof(myDIR));
	strcpy (dir->File.files[0].name,dirName);

	strcpy(dirName, name);
		//get empty inode
	if( (dir->File.files[0].inode = lookForEmptyInode()) == -1 ){
		printf("myopendir_completeRootcreation: Error of looking for empty Inode\n");
		emptyDirSlot(dir);
		return NULL;
	}
	struct Inode inode;
	memset(&inode,0,sizeof(inode));
	inode.used = true;
	inode.size = BLOCK_SIZE;
	inode.blocks = 1;
	inode.type = DIR_TYPE;
	//get empty Data block
	if( (inode.directs[0] = lookForEmptyBlock()) == -1){
		printf("myopendir_completeRootcreation: Error of looking for empty Block\n");
		emptyDirSlot(dir);
		return NULL;
	}
	if(debug){printf("debug:myopendir_completeRootcreation - store new dir %s in inode %d, block = %d\n",dir->File.files[0].name,dir->File.files[0].inode,inode.directs[0]);}
	//in case of parent exists - update parent's Data block with new chield dir and parent's descriptor to new dir
	if(debug){printf("debug:myopendir_completeRootcreation: taken inode=%d, block=%d\n",dir->File.files[0].inode,inode.directs[0]);}
	if( !writeDirEntry_InodeByDirInode(dir,&inode) ){
		printf("myopendir_completeRootcreation: Error writing dir %s Data block %d, inode %d\n",dirName,inode.directs[0],dir->File.files[0].inode);
		emptyDirSlot(dir);
		return NULL;
	}
	markBlock(inode.directs[0], 1);		//mark block as used
    return dir;
}

int myclosedir(myDIR *dirp){
	if(!dirp){
		printf("myclosedir: Error - null Dir descriptor given\n");
		return 0;
	}
	dirp->dirent_index = 0;
	emptyDirSlot(dirp);
	return 1;
}
struct mydirent *myreaddir(myDIR *dirp){
	if(!dirp){
		printf("myreaddir: Error - null Dir descriptor given\n");
		return NULL;
	}
	//check if disk file mounted
	if( !strcmp(dirp->File.files[0].name, "/") && !myFS.fileSystemPhysicalSize ){ 
		return NULL;
	}
	//go over all dir's files and return entry of dirent_index. 
	for(;dirp->dirent_index < MAX_DIR_FILE_NUMBER;){
		if(debug){printf("debug: myreaddir - dirp->dirent_index=%d,name=%s\n",dirp->dirent_index,dirp->File.files[dirp->dirent_index].name);}
		if( !dirp->dirent_index || !strlen(dirp->File.files[dirp->dirent_index].name) ){ //check for empty slot
			dirp->dirent_index ++; 
			continue; 
		}
		//before return - update dirent_index
		return (struct mydirent *)&dirp->File.files[dirp->dirent_index++];
	}
	return NULL;
}
/*
The open() system call opens the file specified by pathname.  If
       the specified file does not exist, it may optionally (if O_CREAT
       is specified in flags) be created by open()

The return value of open() is a file descriptor, a small,
       nonnegative integer that is an index to an entry in the 
A call to open() creates a new open file description, an entry in
       the system-wide table of open files.  The open file description
       records the file offset and the file status flags 
	   
Flags:
#define FREAD		O_RDONLY	
#define FWRITE		O_WRONLY
#define FREADWRITE	O_RDWR
#define FCREATE		O_CREAT
#define FAPPEND		O_APPEND

*/
int myopen(const char *name, int flags){
    myDIR *dir = NULL;
	char fileName[MAX_FILE_NAME];		// a file name without all path prefix
	char tmpName[MAX_FILE_NAME];
	char parentPath[MAX_FILE_NAME];		// a path of parent's dir
	struct file myfile;
	struct Inode inode;
    //check for valid parameters
	if( name == NULL || !strlen(name) ){
		printf("myopen: wrong file path given in input\n");
		return -1;
	}
	//check the path is absolete - starts from root dir /
	if(name[0] != '/'){
		printf("myopen: relative path not supported\n");
		return -1;
	}
	memset(&myfile,0,sizeof(myfile));
	//extract path of parent dir and file name
	memset(parentPath,0,sizeof(parentPath));
	strcat(parentPath,"/");				
	memset(tmpName,0,sizeof(tmpName));
	strcpy(tmpName,name);
	char tokens[MAX_PATH_DEPTH][MAX_FILE_NAME];
	memset(tokens,0,sizeof(tokens));
	int numtokens=0;
	char *token = strtok(tmpName , "/");
	while(token != NULL){
		strcpy(tokens[numtokens++],token);
		memset(fileName,0,sizeof(fileName));
		strcpy(fileName,token);		//file name is a last name in path
		token = strtok(NULL,"/");
	}
	for(int i=0; i<numtokens-1;i++){
		strcat(parentPath, tokens[i]);
		if( i+1 < numtokens ){
			strcat(parentPath, "/");
		}
	}
	if(debug){printf("debug:myopen-look for parent dir %s\n",parentPath);}
	//get parent's dir info
	if( !(dir = myopendir(parentPath)) ){
		printf("myopen: Error opening parend dir %s\n",parentPath);
		return -1;
	}
	int i;
	//got over a list of parent's files/dirs to look for the file - check if exists
	for (i = 0; i < MAX_DIR_FILE_NUMBER; i++){
		if(debug){printf("debug:myopen - check dir names: %d=[%s], comp with [%s]\n",i,dir->File.files[i].name,fileName);}
		int inodeNum = dir->File.files[i].inode;
		if(strlen(dir->File.files[i].name) && !strcmp(dir->File.files[i].name,fileName) && myFS.myInodes[inodeNum].type == FILE_TYPE){ 
			//file found in a list of parent's dir
			if(debug){printf("debug: myopen: Found name %s in index %d,inode %d\n" , dir->File.files[i].name, i, dir->File.files[i].inode);}
			if( !getFileEntryByInode(dir->File.files[i].inode,&myfile,true) ){
				printf("myopen: Error reading inode %d\n",dir->File.files[i].inode);
				myclosedir(dir);
				return -1;
			}
			break;
		}
	}
	//we can't find the file - need to create it
	if(debug){printf("debug: myopen: i=%d,max=%d,flags = %d\n",i,MAX_DIR_FILE_NUMBER,flags);}
	if( i == MAX_DIR_FILE_NUMBER ){	
		//firstly - check if FCREATE flag is set
		if( !(flags & FCREATE) ){
			if(debug){printf("debug:myopen: Error of Creating new file - the flag FCREATE not set\n");}
			return -1;
		}
	
		//update fields of file and inode
		memset(&myfile, 0, sizeof(myfile));
		//myfile.files[0].type = FILE_TYPE;
		strcpy (myfile.files[0].name,fileName);
		if( (myfile.files[0].inode = lookForEmptyInode()) == -1 ){
			printf("myopen: Error of looking for empty Inode\n");
			return -1;
		}
		
		memset(&inode,0,sizeof(inode));
		inode.used = true;
		inode.size = 0;
		inode.blocks = 1;
		inode.type = FILE_TYPE;
		inode.flags = flags;
		if( (inode.directs[0] = lookForEmptyBlock()) == -1){
			printf("myopen: Error of looking for empty Block\n");
			myclosedir(dir);
			return -1;
		}
		//and parent's descriptor to new file
		strcpy(myfile.parent.name,dir->File.files[0].name);
		myfile.parent.inode = dir->File.files[0].inode;

		//now update parent's files list to add this new file
		bool freeSlotFoundInParentFilesList = false;
		//look for empty slot in parent's dir list of files
		for(int i = 1; i < MAX_DIR_FILE_NUMBER; i++){
			//if empty foujnd - update parent's Data block with new chield dir
			if( !strlen(dir->File.files[i].name) ){
				strcpy( dir->File.files[i].name, fileName);
				//dir->File.files[i].type = myfile.files[0].type;
				dir->File.files[i].inode = myfile.files[0].inode;
				freeSlotFoundInParentFilesList = true;
				if(debug){printf("debug:myopen update parentdir %s list entry %d, val = [%s,i=%d]\n",
					dir->File.files[0].name,i,dir->File.files[i].name,dir->File.files[i].inode);}
				break;
			}
		}
		if( !freeSlotFoundInParentFilesList ){
			printf("myopen: Error of looking for empty slot in parent %s (path %s) file list\n",dir->File.files[0].name,parentPath);
			myclosedir(dir);
			return -1;
		}
		//update parent's Dir data block
		struct Inode parentInode;
		if( !readInode(dir->File.files[0].inode, &parentInode) ){
			printf("myopen: Error of reading parent's inode %d\n",dir->File.files[0].inode);
			myclosedir(dir);
			return -1;					
		}
		if( !writeDirEntry_InodeByDirInode(dir,&parentInode) ){
			printf("myopen: Error writing parent's dir %s Data block %d, inode %d\n",parentPath,parentInode.directs[0],dir->File.files[0].inode);
			myclosedir(dir);
			return -1;
		}
		if(debug){printf("debug:myopen - store new file %s in inode %d, block = %d\n",myfile.files[0].name,myfile.files[0].inode,inode.directs[0]);}
		if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
			printf("myopen: Error writing file %s Data block %d, inode %d\n",fileName,inode.directs[0],myfile.files[0].inode);
			return -1;
		}
		markBlock(inode.directs[0], 1);		//mark block as used	
	}
	myclosedir(dir);
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("myread: Error reading inode %d\n",myfile.files[0].inode);
		return -1;
	}
	inode.flags = flags;
	//check if file opened with FAPPEND flag - so - write will be from seek = file size
	if( (flags & FAPPEND) ){
		myfile.files[0].offset = inode.size;
	}
	if(debug){printf("debug:myopen:offset=%ld\n",myfile.files[0].offset);}
	if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
		if(debug){printf("debug:myopen: Error writing file %s Data block %d, inode %d\n",myfile.files[0].name,inode.directs[0],myfile.files[0].inode);}
		return 0;
	}


	//allocate file descriptor and fill open files slot in list
	int desc=-1;
	if( (desc = lookForEmptyFileSlot()) == -1 ){
		printf("myopen: Error - no free File descriptors in system\n");
		return -1;
	}
	if( !(updateFileSlot(desc, &myfile)) ){
		printf("myopen: Error - can't update File's descriptot in descriptors list in system\n");
		emptyFileSlot(desc);
		return -1;
	}
	return desc;
}
int getFileSizeByDesc(int myfd){
	struct file myfile;
	struct Inode inode;
	if(myfd<0){
		printf("getFileSizeByDesc: Error - null file descriptor given\n");
		return 0;
	}
	//get file's info by file descriptor
	if( !(getFileSlot(myfd, &myfile)) ){
		printf("getFileSizeByDesc: Error - can't get File's descriptot %d info from descriptors list in system\n",myfd);
		return -1;
	}
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("getFileSizeByDesc: Error reading inode %d\n",myfile.files[0].inode);
		return -1;
	}
	return inode.size;
}
int set0FileSizeByDesc(int myfd){
	struct file myfile;
	struct Inode inode;
	if(myfd<0){
		printf("setFileSizeByDesc: Error - null file descriptor given\n");
		return 0;
	}
	//get file's info by file descriptor
	if( !(getFileSlot(myfd, &myfile)) ){
		printf("setFileSizeByDesc: Error - can't get File's descriptot %d info from descriptors list in system\n",myfd);
		return -1;
	}
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("setFileSizeByDesc: Error reading inode %d\n",myfile.files[0].inode);
		return -1;
	}
	int block = -1;
	int direct = 1;
	//remove Data blocks
	if(debug){printf("debug: set0FileSizeByDesc - remove Data blocks - inode.blocks=%d\n",inode.blocks) ;}
	while( inode.blocks > 1 && direct < NUM_OF_DIRECT_BLOCKS ){ 
		//check if block really used
		if( (block = inode.directs[direct]) == 0){ break; }
		if(debug){printf("debug: set0FileSizeByDesc direct=%d,block=%d\n",direct,block );}
		inode.blocks--;	//decrease number of used blocks
		if(debug){printf("debug: set0FileSizeByDesc remove block %d direct = %d nowblocks = %d\n",block,direct,inode.blocks);}
		markBlock(block, 0);
		inode.size -= BLOCK_SIZE;
	}
	inode.size = 0;
	if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
		printf("set0FileSizeByDesc: Error writing file %s Data block %d, inode %d\n",myfile.files[0].name,inode.directs[0],myfile.files[0].inode);
		return 0;
	}
	return 1;
}

int myclose(int myfd){
	struct file myfile;
	struct Inode inode;
	if(myfd<0){
		printf("myclose: Error - null file descriptor given\n");
		return 0;
	}
	//now - need to change offset to 0
	//get file's info by file descriptor
	if( !(getFileSlot(myfd, &myfile)) ){
		printf("myclose: Error - can't get File's descriptot %d info from descriptors list in system\n",myfd);
		emptyFileSlot(myfd);
		return -1;
	}
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("myclose: Error reading inode %d\n",myfile.files[0].inode);
		return -1;
	}
	//update current offset
	myfile.files[0].offset = 0;
	//update file Block and descriptor
	if( !(updateFileSlot(myfd, &myfile)) ){
		printf("myclose: Error - can't update File's descriptot in descriptors list in system\n");
		emptyFileSlot(myfd);
		return 0;
	}
	if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
		printf("myclose: Error writing file %s Data block %d, inode %d\n",myfile.files[0].name,inode.directs[0],myfile.files[0].inode);
		return 0;
	}

	emptyFileSlot(myfd);

	return 1;
}
ssize_t mywrite(int myfd, const void *buf, size_t count){
	struct file myfile;
	struct Inode inode;
	int rc;

	if(debug){printf("debug: mywrite desc %d,sz=%ld\n",myfd,count);}
	if( myfd == -1 || buf == NULL || !count ){
		printf("mywrite: Error - Wrong parameters given: fd = %d, buf = %p, count=%ld\n",myfd,buf,count);
		return -1;
	}
	//get file's info by file descriptor
	if( !(getFileSlot(myfd, &myfile)) ){
		printf("mywrite: Error - can't get File's descriptot %d info from descriptors list in system\n",myfd);
		emptyFileSlot(myfd);
		return -1;
	}
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("mywrite: Error reading inode %d\n",myfile.files[0].inode);
		return -1;
	}
	if( !(inode.flags & (FWRITE | FREADWRITE | FAPPEND)) ){
		printf("mywrite: Can't write - file is Read Only \n");
		return -1;
	}

	int block = -1;
	int fileDiscSize = ((inode.blocks - 1) * BLOCK_SIZE);
	int direct = 1;
	//check & assign enough Data blocks
	if(debug){printf("debug: mywrite - check & assign enough Data blocks - fileDiscSize=%d, required=%ld\n",fileDiscSize,(myfile.files[0].offset + count) );}
	while( fileDiscSize < (myfile.files[0].offset + count)){ 
		//look for empty direct block slot
		while( direct < NUM_OF_DIRECT_BLOCKS ){
			if(inode.directs[direct] == 0){ break; }
			else{ direct++; }
		}
		if(debug){printf("debug: mywrite fileDiscSize=%d, direct=%d\n",fileDiscSize,direct );}
		if( direct == NUM_OF_DIRECT_BLOCKS ){
			printf("mywrite: Error - no more empty slot for direct blocks can be used\n");
			emptyFileSlot(myfd);
			return -1;
		}
		
		if( (block = inode.directs[direct] = lookForEmptyBlock()) == -1){
			printf("mywrite: Error of looking for empty Block\n");
			return -1;
		}
		inode.blocks++;	//increase number of used blocks
		if(debug){printf("debug: mywrite alloc new block %d ind = %d nowblocks = %d\n",block,direct,inode.blocks);}
		if(debug){printf("debug: mywrite fileDiscSize=%d, direct=%d, block = %d,inode.blocks=%d\n",fileDiscSize,direct,block,inode.blocks );}
		//update inode with new attached data block
		if( !writeInode(myfile.files[0].inode, &inode) ){
			printf("mywrite: Error of writing inode %d\n",myfile.files[0].inode);
			return -1;
		}
		markBlock(block, 1);
		fileDiscSize += BLOCK_SIZE;
		
	}
	char tmpbuf[BLOCK_SIZE];
	char *buforig = (char *)malloc(count);
	memcpy(buforig,buf,count);
	char *bufp = buforig;
	int bytesToWrite=0;
	off_t blockOffsetToWrite = (myfile.files[0].offset % BLOCK_SIZE);;
	int countRemaining = count;
	if(debug){printf("debug: mywrite write cycle: countRemaining=%d inode.blocks=%d\n",countRemaining,inode.blocks );}
	for(direct = 1; direct < inode.blocks; direct++){
		if( !countRemaining ){ break; }
		block = inode.directs[direct];
		if(debug){printf("debug: mywrite direct = %d, block=%d,countRemaining=%d\n",direct, block,countRemaining);}
		//check if need to write from buf to the block
		if( myfile.files[0].offset >= ((direct-1) * BLOCK_SIZE) && myfile.files[0].offset < ((direct) * BLOCK_SIZE)){
			if((blockOffsetToWrite + countRemaining) <= BLOCK_SIZE){	//write last chunk less than block size
				bytesToWrite = countRemaining;
				countRemaining = 0;
			} 
			else if((blockOffsetToWrite + countRemaining) > BLOCK_SIZE){	//write full block or from the inner position of block
				bytesToWrite = (BLOCK_SIZE - blockOffsetToWrite);
				countRemaining -= bytesToWrite;
				blockOffsetToWrite = 0;
			}
			memset(tmpbuf, 0, sizeof(tmpbuf));
			memcpy(tmpbuf, bufp, bytesToWrite);
			bufp += bytesToWrite;
			if(debug){printf("debug: mywrite wrtite buf %s to fd %d to block %d, offset=%ld, count=%d\n",tmpbuf,myfd,block,blockOffsetToWrite,bytesToWrite);}
			//write the data block
			if( !(rc = writeBuffer(block, blockOffsetToWrite, (const char *)tmpbuf, bytesToWrite) ) ){
				printf("mywrite: Error - failed to wrtite buf %s to fd %d to block %d, offset=%ld, count=%d\n",tmpbuf,myfd,block,blockOffsetToWrite,bytesToWrite);
				free(buforig);
				return -1;
			}
		}
	}
	free(buforig);
	//check if need to update file's size :
	if( myfile.files[0].offset + count > inode.size ){
		inode.size = myfile.files[0].offset + count;
		if( !writeInode(myfile.files[0].inode, &inode) ){
			printf("mywrite: Error writing inode %d\n",myfile.files[0].inode);
			return -1;
		}
	}
	//update current offset
	myfile.files[0].offset = myfile.files[0].offset + count;
	//update file Block and descriptor
	if( !(updateFileSlot(myfd, &myfile)) ){
		printf("mywrite: Error - can't update File's descriptot in descriptors list in system\n");
		emptyFileSlot(myfd);
		return 0;
	}
	if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
		printf("mywrite: Error writing file %s Data block %d, inode %d\n",myfile.files[0].name,inode.directs[0],myfile.files[0].inode);
		return 0;
	}
	return (ssize_t)count;
}
ssize_t myread(int myfd, void *buf, size_t count){
	struct file myfile;
	struct Inode inode;
	int rc;

	if(debug){printf("debug: myread desc %d,sz=%ld\n",myfd,count);}
	//get file's info by file descriptor
	if( !(getFileSlot(myfd, &myfile)) ){
		printf("myread: Error - can't get File's descriptot %d info from descriptors list in system\n",myfd);
		emptyFileSlot(myfd);
		return -1;
	}
	if(debug){printf("debug: myfd=%d,myread myfile.files[0].inode=%d\n",myfd,myfile.files[0].inode);}
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("myread: Error reading inode %d\n",myfile.files[0].inode);
		return -1;
	}
	if( FREAD == 0 && inode.flags && !(inode.flags & FREADWRITE) ){
		printf("myread: Can't read - the file is Write Only : flags = %d\n",inode.flags );
		return -1;
	}
	else if( FREAD != 0 && (!(inode.flags & FREAD) || !(inode.flags & FREADWRITE)) ){
		printf("myread: Can't read - the file is Write Only : flags=%d\n",inode.flags );
		return -1;
	}
	if( inode.size < myfile.files[0].offset + count ){
		if(debug){printf("myread: can't read %ld bytes from offset %ld - file size %d bytes\n",count,myfile.files[0].offset,inode.size);}
		return 0;
	}
	int blockInFile = (myfile.files[0].offset / BLOCK_SIZE) + 1;		//the num of flock inside the file
	if( blockInFile >= NUM_OF_DIRECT_BLOCKS ){
		printf("myread: Error - can't access blockInFile index = %d, Max is %d\n",blockInFile,NUM_OF_DIRECT_BLOCKS);
	}
	int block = 0;
	int blockOffset = myfile.files[0].offset % BLOCK_SIZE;	//offset inside block
	int gotBytes = 0;
	int getBytes = 0;
	while( gotBytes < count ){
		block = inode.directs[blockInFile];				//real block num
		if(debug){printf("debug: myread - try read count=%ld from offset %ld: blockInFile-%d block = %d,blockOffset=%d\n",count, myfile.files[0].offset,blockInFile,block,blockOffset);}
		if( !block ){ //no Data blocks were allocated for the file until now 
			printf("myread: no more Data blocks were allocated for the file until now\n");
			return 0;
		}
		if( count - gotBytes <= BLOCK_SIZE ){
			getBytes = count - gotBytes;
		}
		else {
			getBytes = BLOCK_SIZE;
		}
		if(debug){printf("debug: myread - read count=%ld from offset %ld: blockInFile-%d block = %d,blockOffset=%d,getBytes=%d\n",count, myfile.files[0].offset,blockInFile,block,blockOffset,getBytes);}
		if( !(rc = readBuffer(block, blockOffset, buf + gotBytes, getBytes) ) ){
			if(debug){printf("myread: Error - failed to read fd %d from block %d, offset=%ld\n",myfd,block,myfile.files[0].offset);}
			return -1;
		}
		gotBytes += getBytes;	//increase obtained bytes
		blockInFile++;			//next read from next Data block of file
		blockOffset = 0;		//read next block from offset 0;
	}
	if(debug){printf("debug: myread done: buf=%s,block=%d,offset=%ld\n",(char *)buf,block,myfile.files[0].offset);}
	//update current offset
	myfile.files[0].offset = myfile.files[0].offset + count;
	//update file Block and descriptor
	if( !(updateFileSlot(myfd, &myfile)) ){
		printf("myread: Error - can't update File's descriptot in descriptors list in system\n");
		emptyFileSlot(myfd);
		return 0;
	}
	if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
		printf("myread: Error writing file %s Data block %d, inode %d\n",myfile.files[0].name,inode.directs[0],myfile.files[0].inode);
		return 0;
	}
	
	return (ssize_t)count;
}
/*
lseek() repositions the file offset of the open file description
       associated with the file descriptor fd to the argument offset
       according to the directive whence as follows:

       SEEK_SET
              The file offset is set to offset bytes.

       SEEK_CUR
              The file offset is set to its current location plus offset
              bytes.

       SEEK_END
              The file offset is set to the size of the file plus offset
              bytes.

       lseek() allows the file offset to be set beyond the end of the
       file (but this does not change the size of the file).  If data is
       later written at this point, subsequent reads of the data in the
       gap (a "hole") return null bytes ('\0') until data is actually
       written into the gap.
*/
off_t mylseek(int myfd, off_t offset, int whence){
	struct file myfile;
	struct Inode inode;

	if(debug){printf("debug: mylseek desc %d,offset=%ld, whence=%d\n",myfd,offset,whence);}
	//get file's info by file descriptor
	if( !(getFileSlot(myfd, &myfile)) ){
		printf("mylseek: Error - can't get File's descriptot %d info from descriptors list in system\n",myfd);
		emptyFileSlot(myfd);
		return 0;
	}
	//get file's inode
	if( !readInode(myfile.files[0].inode, &inode) ){
		printf("mylseek: Error reading inode %d\n",myfile.files[0].inode);
		return 0;
	}
/*	if( !block ){ //no Data blocks were allocated for the file until now 
		printf("mylseek: no Data blocks were allocated for the file until now\n");
		return 0;
	}
*/
	if(debug){printf("debug: mylseek file size=%d, current offset=%ld\n",inode.size,myfile.files[0].offset);}
	if( whence == MYSEEK_SET ){
/*		if( inode.size < offset ){
			printf("myread: can't seek to offset %ld from start of file - file size %d bytes\n",offset,inode.size);
			return 0;
		}	
*/
		myfile.files[0].offset = offset;
	}
	else if( whence == MYSEEK_END ){
/*		if( offset > 0 ){
			printf("myread: can't seek to offset %ld from end of file - file size %d bytes\n",offset,inode.size);
			return 0;
		}	
*/
		myfile.files[0].offset = inode.size + offset;
	}
	else if( whence == MYSEEK_CUR ){
/*		if( inode.size < myfile.files[0].offset + offset ){
			printf("myread: can't seek to offset %ld from current position %ld - file size %d bytes\n",offset,myfile.files[0].offset,inode.size);
			return 0;
		}	
*/
		myfile.files[0].offset += offset;
	}
	else{
		printf("mylseek: Wrong 'whence' value given in input: %d\n",whence);
		return 0;
	}

	if(debug){printf("debug: mylseek file size=%d, new offset=%ld\n",inode.size,myfile.files[0].offset);}
	//update file Block and descriptor
	if( !(updateFileSlot(myfd, &myfile)) ){
		printf("mylseek: Error - can't update File's descriptot in descriptors list in system\n");
		emptyFileSlot(myfd);
		return 0;
	}
	if( !writeFileEntry_InodeByFileInode(&myfile,&inode) ){
		printf("mylseek: Error writing file %s Data block %d, inode %d\n",myfile.files[0].name,inode.directs[0],myfile.files[0].inode);
		return 0;
	}
	
	return (ssize_t)1;
	
}
void printDir(char *name){
    myDIR *dir;
    if(name == NULL){
        return;
    }
    if(name[0] == '/'){
		
		if( !(dir = myopendir(name)) ){
			int flags=FREAD;
			//supplied name isn't dir. check - if it's a file
			int myfd=-1;
			if( (myfd = myopen(name, flags)) == -1 ){
				printf("printDir: Error opening %s /\n",name);
				return;
			}
			printf("The %s is the file\n",name);
			myclose(myfd);
			return;
		}
		if(debug){
			printf("debug:printDir-dir %s list:",name);
			for(int i=0;i<MAX_DIR_FILE_NUMBER;i++){
				printf("[%s]",dir->File.files[i].name);
			}
			printf("\n");
		}
		struct mydirent *de = NULL;
		printf("printDir: dir=%s, parent=%s\n",dir->File.files[0].name,dir->File.parent.name);
		while ((de = myreaddir(dir)) != NULL){
			char type[10];
			//if(de->dirent.type == DIR_TYPE){ strcpy(type,"d"); }
			if(myFS.myInodes[de->dirent.inode].type == DIR_TYPE){ strcpy(type,"d"); }
			else{ strcpy(type," "); }
			int sz = myFS.myInodes[de->dirent.inode].size;

//printf("dirent: %s inode=%d, sz=%d\n",de->dirent.name,de->dirent.inode,sz);
            printf("%s %10d %s\n", type, sz, de->dirent.name);
		}
		myclosedir(dir);
	}
}


