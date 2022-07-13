#include "mystdio.h"
//the function performs various myopendir/myclosedir/myreaddir tests
int dirtest(){
    myDIR *dir;
	bool itsOldDisc=false;
	myFILE *stream;
	
	printf("\n\nStart Test: myopendir/myclosedir/myreaddir ...\n\n");
    if( (stream = myfopen("/file1", "r")) != NULL ){
		itsOldDisc = true;
		printf("dirtest - old disc mounted\n");
		myfclose(stream);
	} else { printf("dirtest - new disc mounted\n");}
	if( !(dir = myopendir("/dir1")) ){
		printf("create/open /dir1 failed\n");
		return 0;
	} else { printf("create/open /dir1 test passed OK\n"); }
	myclosedir(dir);
	if( !(dir = myopendir("/dir1/dir12")) ){
		printf("create/open /dir1/dir2 failed\n");
		return 0;
	} else { printf("create/open /dir1/dir12 test passed OK\n"); }
	myclosedir(dir);
	if( !(dir = myopendir("/dir1/dir12/dir123")) ){
		printf("create/open /dir1/dir2/dir123 failed\n");
		return 0;
	} else { printf("create/open /dir1/dir2/dir123 test passed OK\n"); }
	myclosedir(dir);
	if( !(dir = myopendir("/dir2")) ){
		printf("create/open /dir2 failed\n");
		return 0;
	} else { printf("create/open /dir2 test passed OK\n"); }
	myclosedir(dir);
	if( !(dir = myopendir("/dir3")) ){
		printf("create/open /dir3 failed\n");
		return 0;
	} else { printf("create/open /dir3 test passed OK\n"); }
	myclosedir(dir);
	
	if( !(dir = myopendir("/")) ){
		printf("open / failed\n");
		return 0;
	} else { printf("open / test passed OK\n"); }
	struct mydirent *de = NULL;
	int i=0;
	while ((de = myreaddir(dir)) != NULL){
		if( !strlen(de->dirent.name) ){
			printf("myreaddir / failed on reading item %d. Got: empty name string\n",i);
			return 0;
		}
		if( strcmp(de->dirent.name,"dir1") && strcmp(de->dirent.name,"dir2") && strcmp(de->dirent.name,"dir3")){
			if( itsOldDisc ){
				if( strcmp(de->dirent.name,"file1") && strcmp(de->dirent.name,"file2") && strcmp(de->dirent.name,"file3")){
					printf("myreaddir / failed on reading item %d. Got: %s\n",i,de->dirent.name);
					return 0;
				}
			}
			else {
				printf("myreaddir / failed on reading item %d. Got: %s\n",i,de->dirent.name);
				return 0;
			}
		}
		i++;
	}
	myclosedir(dir);
	printf("\n\nSummary: myopendir/myclosedir/myreaddir tests passed OK\n\n");
	return 1;
}
//the function performs various myfopen/myfclose/myfwrite/myfread/myfprintf/myfscanf/myfseek tests
int filetest(char *name){
	myFILE *stream;
	char buff[128];
	char buff2[128];
	bool itsOldDisc=false;
	ssize_t count=0;
	int rc;

	printf("\n\nStart Test: file = %s for myfopen/myfclose/myfwrite/myfread/myfprintf/myfscanf/myfseek ...\n\n",name);
	
    if( (stream = myfopen(name, "r")) != NULL ){ 
		itsOldDisc = true; 
		printf("filetest - old disc mounted\n");
		myfclose(stream);
	} else {printf("filetest - new disc mounted\n");}
		
	if( itsOldDisc == false ){
		if( (stream = myfopen(name, "w")) == NULL ){ 
			printf("myfopen %s for w failed\n",name);
			return 0;
		} else { printf("myfopen %s with w test passed OK\n",name); }
	
		memset(buff,1,sizeof(buff));
		if( (count = myfwrite(buff, 1, sizeof(buff) , stream)) != sizeof(buff) ){ 
			printf("myfwrite '111...' to %s failed\n",name);
			return 0;
		} else { printf("myfwrite '111...' to %stest passed OK\n",name); }
		
		if( myfseek(stream, 5000, MYSEEK_SET) ){ 
			printf("myfseek 5000, MYSEEK_SET %s failed\n",name);
			return 0;
		} else { printf("myfseek 5000, MYSEEK_SET to %s test passed OK\n",name); }
		
		memset(buff,2,sizeof(buff));
		if( (count = myfwrite(buff, 1, sizeof(buff) , stream)) != sizeof(buff) ){ 
			printf("myfwrite '222...' to %s failed. count = %ld\n",name,count);
			return 0;
		} else { printf("myfwrite '222...' to %s test passed OK\n",name); }
		
		myfclose(stream);
	}
	if( (stream = myfopen(name, "r+")) == NULL ){ 
		printf("myfopen %s for r+ failed\n",name);
		return 0;
	} else { printf("myfopen %s with r+ test passed OK\n",name); }

	memset(buff,0,sizeof(buff));
	memset(buff2,1,sizeof(buff));
	if( (count = myfread(buff, 1,sizeof(buff), stream)) != sizeof(buff) ){ 
		printf("myfread '111...' from %s failed\n",name);
		return 0;
	} else { 
		if( memcmp(buff,buff2,sizeof(buff)) ){ 
			printf("myfread '111...' from %s failed - different buffer obtained\n",name);
			return 0;
		}
		printf("myfread '111...' from %s test passed OK\n",name); 
	}
	if( itsOldDisc == false ){
		if( myfseek(stream, -128, MYSEEK_END) ){ 
			printf("myfseek -128, MYSEEK_END %s failed\n",name);
			return 0;
		} else { printf("myfseek -128, MYSEEK_END to %s test passed OK\n",name); }
	}
	else {
		if( myfseek(stream, 5000, MYSEEK_SET) ){ 
			printf("myfseek 5000, MYSEEK_SET %s failed\n",name);
			return 0;
		} else { printf("myfseek 5000, MYSEEK_SET to %s test passed OK\n",name); }
	}
	memset(buff,0,sizeof(buff));
	memset(buff2,2,sizeof(buff));
	if( (count = myfread(buff, 1,sizeof(buff), stream)) != sizeof(buff) ){ 
		printf("myfread '222...' from %s failed count = %ld\n",name,count);
		return 0;
	} else { 
		if( memcmp(buff,buff2,sizeof(buff)) ){ 
			printf("myfread '222...' from %s failed - different buffer obtained\n",name);
			return 0;
		}
		printf("myfread '222...' from %s test passed OK\n",name); 
	}
	myfclose(stream);
	
	if( itsOldDisc == false ){
		if( (stream = myfopen(name, "a")) == NULL ){ 
			printf("myfopen %s for a failed\n",name);
			return 0;
		} else { printf("myfopen %s with a test passed OK\n",name); }
		if( (rc = myfprintf(stream, "%d %c %f", 11, 'c', 11.11)) <= 0 ){ 
			printf("myfprintf to %s with d c f failed. ret = %d\n",name,rc);
			return 0;
		} else { printf("myfprintf to %s with d c f test passed OK\n",name); }
		myfclose(stream);			
	}

	if( (stream = myfopen(name, "r")) == NULL ){ 
		printf("myfopen %s for r failed\n",name);
		return 0;
	} else { printf("myfopen %s with r test passed OK\n",name); }
	
	if( myfseek(stream, 5000+128, MYSEEK_CUR) ){ 
		printf("myfseek 5000+128, MYSEEK_CUR %s failed\n",name);
		return 0;
	} else { printf("myfseek 5000+128, MYSEEK_CUR to %s test passed OK\n",name); }
	int num=0;
	char chr=0;
	float flt=0;
	if( (rc = myfscanf(stream, "%d %c %f", &num, &chr, &flt)) != 3 ){ 
		printf("myfscanf to %s with d c f failed. ret = %d\n",name,rc);
		return 0;
	} else { printf("myfscanf to %s with d c f test passed OK\n",name); }
	myfclose(stream);
	
	printf("\n\nSummary: myfopen/myfclose/myfwrite/myfread/myfprintf/myfscanf/myfseek tests passed OK\n\n");
	return 1;
}

//as per definition - program getting input argument -s <FS sise>
//the program will build or use it, if exists the file myFileSystem.fs
//The program will perform tests of functions in mystdiolib and readdir from libmyfs
int main(int argc, char *argv[]){
	int fd=-1;
	myFILE *stream;
	char buff[128];
	myDIR *dir;
	int ret=0;
	
	debugioctl=0;
	debug=0;

    if(argc != 3){
        printf("Wrong amount of arguments: %d\n", argc);
        return -1;  
    }
    if(strcmp(argv[1], "-s") != 0){
           printf("Wrong key: %s\n", argv[1]);
           return -1;
    }
    if(atoi(argv[2]) <= 0){
           printf("Wrong ufs size: %s\n", argv[2]);
           return -1;
    }
	int myfssize=atoi(argv[2]);
    if( !mymkfs(myfssize) ){
		printf("Error - mymkfs of size %d failed\n",myfssize);
		return -1;
	}

	if( !mymount("myFileSystem.fs", "/",NULL, 0, NULL)) {
		printf("Error - mount of %s to %s failed\n","myFileSystem.fs","/");
		return -1;
	}
	if( !dirtest() ){ return 0; }
	if( !filetest("/file1") ){ return 0; }
	if( !filetest("/file2") ){ return 0; }
	if( !filetest("/file3") ){ return 0; }
	if( !filetest("/dir1/file11") ){ return 0; }
	if( !filetest("/dir1/file12") ){ return 0; }
	if( !filetest("/dir1/file13") ){ return 0; }
	
	return 1;
}