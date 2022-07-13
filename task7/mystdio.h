#include "myfs.h"
#include <stdarg.h>

typedef struct myfile{
	char 	 		name[MAX_FILE_NAME];	//file's name
	off_t			offset;					//offset value (0..file size). Set to 0 with opening/closing the file or by mylseek()
	int 			fd;		//fd f
	unsigned char	flags;	//file flags
} myFILE;

myFILE *myfopen(const char *pathname, const char *mode);
int myfclose(myFILE *stream);
size_t myfread(void *ptr, size_t size, size_t nmemb, myFILE *stream);
size_t myfwrite(const void *ptr, size_t size, size_t nmemb, myFILE *stream);
int myfseek(myFILE *stream, long offset, int whence);
int myfscanf(myFILE *stream,const char *format, ...);
int myfprintf(myFILE *stream, const char *format, ...);

int debugioctl;
//a list of  open myFILE-s
myFILE	 myOpen_myFILEs[MAX_FILES];