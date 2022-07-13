#include "mystdio.h"

//internal functions
myFILE *getEmpty_myFILE(){
	for(int i=0; i < MAX_FILES; i++){
		if( !myOpen_myFILEs[i].flags ){
			if(debugioctl){printf("debug:getEmpty_myFILE - empty slot %d p %p\n",i,&myOpen_myFILEs[i]); }
			return &myOpen_myFILEs[i];
		}
	}
	return NULL;
}

int free_myFILE(myFILE *stream){
	memset(stream,0,sizeof(myFILE));
	return 1;
}

//user functions

/*fopen
The fopen() function opens the file whose name is the string
       pointed to by pathname and associates a stream with it.

       The argument mode points to a string beginning with one of the
       following sequences (possibly followed by additional characters,
       as described below):

       r      Open text file for reading.  The stream is positioned at
              the beginning of the file.

       r+     Open for reading and writing.  The stream is positioned at
              the beginning of the file.

       w      Truncate file to zero length or create text file for
              writing.  The stream is positioned at the beginning of the
              file.

       a      Open for appending (writing at end of file).  The file is
              created if it does not exist.  The stream is positioned at
              the end of the file.
*/
myFILE *myfopen(const char *pathname, const char *mode){
	int flags = 0;
	myFILE *stream;
	int fd;
	
	if( !strcmp(mode,"r+") ){
		flags |= (FREADWRITE);
	}
	else if( !strcmp(mode,"r") ){
		flags |= FREAD;
	}
	else if( !strcmp(mode,"w") ){
		flags |= (FWRITE|FCREATE);
	}
	else if( !strcmp(mode,"w+") ){
		flags |= (FREADWRITE|FCREATE);
	}
	else if( !strcmp(mode,"a") ){
		flags |= (FAPPEND|FCREATE);
	}
	else{
		printf("myfopen: Wrong mode given in input: %s\n",mode);
		return NULL;
	}
	if( (fd = myopen(pathname, flags)) == -1 ){
		printf("myfopen: Error of opening file %s\n",pathname);
		return NULL;
	}
	//allocate myFILE 
	if( !(stream = getEmpty_myFILE()) ){
		printf("myfopen: Error - no empty FILE -s available\n");
		return NULL;
	}
	strcpy(stream->name,pathname);
	if( !strcmp(mode,"w") || !strcmp(mode,"w+")){
		stream->offset = 0;
		set0FileSizeByDesc(fd);
	}
	else if( (flags & FAPPEND) ){
		stream->offset = getFileSizeByDesc(fd);
	}
	else {
		stream->offset = 0;
	}
	stream->fd = fd;
	stream->flags = flags;
	printf("myfopen:flags = %d\n",flags);
	return stream;
}
//fclose
int myfclose(myFILE *stream){
	int rc = 0;
	if(debugioctl){printf("debug:myfclose close stream %p\n",stream);}
	if( stream == NULL ){
		printf("myfclose: NULL stream pointer given in input\n");
		rc = EOF;
	}
	else if( stream < myOpen_myFILEs || stream > (myOpen_myFILEs + sizeof(myOpen_myFILEs)) ){
		printf("myfclose: Wrong stream pointer given in input\n");
		rc = EOF;
	}
	else if( !(rc = myclose(stream->fd)) ){
		printf("myfclose: Error of closing file\n");
		rc = EOF;
	}
	free_myFILE(stream);
	return rc;
}
/*fread
The function fread() reads nmemb items of data, each size bytes
       long, from the stream pointed to by stream, storing them at the
       location given by ptr.
On success, fread() and fwrite() return the number of items read
       or written.  This number equals the number of bytes transferred
       only when size is 1.  If an error occurs, or the end of the file
       is reached, the return value is a short item count (or zero)

       The file position indicator for the stream is advanced by the
       number of bytes successfully read or written.
*/
size_t myfread(	void * ptr, 	//This is the pointer to a block of memory with a minimum size of size*nmemb bytes
				size_t size, 			//This is the size in bytes of each element to be read
				size_t nmemb, 			//This is the number of elements, each one with a size of size bytes
				myFILE * stream	//This is the pointer to a FILE object that specifies an input stream
				){
	size_t rc = 0;
	if( ptr == NULL || stream == NULL || !size || !nmemb ){
		printf("myfread: Wrong parameters given in input: %ld,%ld,%p\n",size,nmemb,stream);
		return 0;
	}
	if( stream < myOpen_myFILEs || stream > (myOpen_myFILEs + sizeof(myOpen_myFILEs)) ){
		printf("myfread: Wrong stream pointer given in input\n");
		rc = EOF;
	}

	size_t sz = (size * nmemb); 
	if( (rc = myread(stream->fd, ptr, sz)) != sz ){
		if(debug){printf("myfread: Error of reading size=%ld nmemb = %ld. myread returned %ld\n",size,nmemb,rc);}
		return 0;
	}
	if(debugioctl){printf("debug:myfread from stream %p, fd = %d, myread rc = %ld\n",stream, stream->fd, rc);}
	stream->offset += sz;
	return nmemb;
}
/*fwrite
The function fwrite() writes nmemb items of data, each size bytes
       long, to the stream pointed to by stream, obtaining them from the
       location given by ptr.
On success, fread() and fwrite() return the number of items read
       or written.  This number equals the number of bytes transferred
       only when size is 1.  If an error occurs, or the end of the file
       is reached, the return value is a short item count (or zero)

       The file position indicator for the stream is advanced by the
       number of bytes successfully read or written.
*/	   
size_t myfwrite(	const void * ptr, 	//This is the pointer to a block of memory with a minimum size of size*nmemb bytes
				size_t size, 			//This is the size in bytes of each element to be read
				size_t nmemb, 			//This is the number of elements, each one with a size of size bytes
				myFILE * stream	//This is the pointer to a FILE object that specifies an input stream
				){
	size_t rc = 0;
	if( ptr == NULL || stream == NULL || !size || !nmemb ){
		printf("myfwrite: Wrong parameters given in input: %s,%ld,%ld,%p\n",(char *)ptr,size,nmemb,stream);
		return 0;
	}
	if( stream < myOpen_myFILEs || stream > (myOpen_myFILEs + sizeof(myOpen_myFILEs)) ){
		printf("myfwrite: Wrong stream pointer given in input\n");
		rc = EOF;
	}
	size_t sz = (size * nmemb); 
	if( (rc = mywrite(stream->fd, ptr, sz)) != sz ){
		printf("myfwrite: Error of writing size=%ld nmemb = %ld. mywrite returned %ld\n",size,nmemb,rc);
		return 0;
	}
	if(debugioctl){printf("debug:myfwrite to stream %p, fd = %d, mywrite rc = %ld\n",stream, stream->fd, rc);}
	stream->offset += sz;
	return nmemb;
}
/*
The fseek() function sets the file position indicator for the
       stream pointed to by stream.  The new position, measured in
       bytes, is obtained by adding offset bytes to the position
       specified by whence.  If whence is set to SEEK_SET, SEEK_CUR, or
       SEEK_END, the offset is relative to the start of the file, the
       current position indicator, or end-of-file, respectively
	   
	   Upon successful
       completion, fseek() return 0
*/
int myfseek(myFILE *stream, long offset, int whence){
	
	if( stream == NULL ){
		printf("myfseek: NULL stream given in input:\n");
		return 1;
	}
	if( stream < myOpen_myFILEs || stream > (myOpen_myFILEs + sizeof(myOpen_myFILEs)) ){
		printf("myfseek: Wrong stream pointer given in input\n");
		return 1;
	}

		
	if(debugioctl){printf("debug:myfseek: seek in stream=%p to %ld from %d\n", stream, offset,whence);}
	if( !mylseek(stream->fd, offset, whence) ){
		printf("myfseek: Error of mylseek\n");
		return -1;
	}
	if( whence == MYSEEK_SET ){
		stream->offset = offset;
	}
	else if( whence == MYSEEK_END ){
		stream->offset = getFileSizeByDesc(stream->fd) + offset;
	}
	else if( whence == MYSEEK_CUR ){
		stream->offset += offset;
	}

	return 0;
}
/*
The fscanf() function shall read from the named input stream.
Upon successful completion, these functions shall return the
       number of successfully matched and assigned input items; this
       number can be zero in the event of an early matching failure. If
       the input ends before the first conversion (if any) has
       completed, and without a matching failure having occurred, EOF
       shall be returned. If an error occurs before the first conversion
       (if any) has completed, and without a matching failure having
       occurred, EOF shall be returned
*/
int myfscanf(myFILE *stream,const char *format, ...){
	va_list arguments;
	int numargs=0;
	char formatarr[100];
	int   num;
	char  chr;
	float flt;
	
	if( stream == NULL || format == NULL ){
		printf("myfscanf: Wrong parameters given in input: stream=%p, format=%p\n",stream,format);
		return EOF;
	}
	if( stream < myOpen_myFILEs || stream > (myOpen_myFILEs + sizeof(myOpen_myFILEs)) ){
		printf("myfscanf: Wrong stream pointer given in input\n");
		return 1;
	}
	if(debugioctl){printf("debug:myfscanf - got input format=%s\n",format);}
	char buff[100];
	memset(buff,0,sizeof(buff));
	strcpy(buff,format);
	//parse format into array of Qualifyers 
	char *token = strtok(buff , " ");
	while(token != NULL){
		if(debugioctl){printf("debug:myfscanf - token=%s,buff=%s\n",token,buff);}
		if( !strcmp(token,"%d") ){
			formatarr[numargs++] = 'd';
		}
		else if( !strcmp(token,"%c") ){
			formatarr[numargs++] = 'c';
		}
		else if( !strcmp(token,"%f") ){
			formatarr[numargs++] = 'f';
		}
		else{
			printf("myfscanf: Wrong Qualifyers Input %s given in input: stream=%p, format=%s\n",token,stream,format);
			return EOF;			
		}
		token = strtok(NULL," ");
	}
	char symbol[2];
	char param[100];
	int i=0;
	memset(param,0,sizeof(param));
	memset(symbol,0,sizeof(symbol));
	va_start ( arguments, format );
	//read file byte by byte until space or EOL or max num of Qualifyers in format
	while( (myfread(&symbol,1,1,stream) == 1) && i < numargs ){
		if(debugioctl){printf("debug:myfscanf - i= %d,read symbol [%c],param=%s,stream->offse=%ld,file sz=%d\n",i,symbol[0],param,stream->offset,getFileSizeByDesc(stream->fd));}
		//if param non empty and space or EOL - parameter reading completed
		if( strlen(param) && (symbol[0] == ' ' || symbol[0] == '\n' || stream->offset == getFileSizeByDesc(stream->fd))){
			if(formatarr[i] == 'd'){
				num = atoi(param);
				int *ptr = va_arg ( arguments, int *);	//in case of %d scanned - the int * pointer expected
				if(debugioctl){printf("debug:myfscanf - param %d = [%s] set num %d to %p\n",i,param,num,ptr);}
				*ptr = num;
			}
			else if(formatarr[i] == 'c'){
				chr = param[0];
				char *ptr = va_arg ( arguments, char *);	//in case of %c scanned - the char * pointer expected
				if(debugioctl){printf("debug:myfscanf - param %d = [%s] set char %c to %p\n",i,param,chr,ptr);}
				*ptr = chr;
			}
			else if(formatarr[i] == 'f'){
				flt = atof(param);
				float *ptr = va_arg ( arguments, float *);	//in case of %f scanned - the float * pointer expected
				if(debugioctl){printf("debug:myfscanf - param %d = [%s] set float %f to %p\n",i,param,flt,ptr);}
				*ptr = flt;
			}
			i++;
			memset(param,0,sizeof(param));
			if(symbol[0] == '\n'){ break; }		//if EOL - break the file reading
			memset(symbol,0,sizeof(symbol));
		}
		else if(symbol[0] == '\n'){ break; }		//if EOL - break the file reading
		else if(symbol[0] == ' '){ memset(symbol,0,sizeof(symbol)); } //skeep spaces before param
		else
		{	//accumulate the parameter's value from symbols
			symbol[1] = '\0';
			strcat(param,symbol);
			memset(symbol,0,sizeof(symbol));
		}
	}
	va_end ( arguments );                  

	if(i != numargs){
		printf("myfscanf: Can't process format: stream=%p, format=%s\n",stream,format);
		return EOF;			
	}
	return numargs;	
}
/*
The fprintf() function shall place output on the named output
       stream.  
	   
	   Upon successful completion, the dprintf(), fprintf(), and
       printf() functions shall return the number of bytes transmitted.
	   
	If an output error was encountered, these functions shall return
       a negative value and set errno to indicate the error.	   
*/
int myfprintf(myFILE *stream, const char *format, ...){
	va_list arguments;
	int numargs=0;
	char formatarr[100];
	int   num;
	char  chr;
	float flt;
	int rc = -1;
	
	if( stream == NULL || format == NULL ){
		printf("myfprintf: Wrong parameters given in input: stream=%p, format=%p\n",stream,format);
		return EOF;
	}
	if( stream < myOpen_myFILEs || stream > (myOpen_myFILEs + sizeof(myOpen_myFILEs)) ){
		printf("myfprintf: Wrong stream pointer given in input\n");
		return 1;
	}
	if(debugioctl){printf("debug:myfprintf - got input format=%s\n",format);}
	char buff[100];
	memset(buff,0,sizeof(buff));
	strcpy(buff,format);
	//parse format into array of Qualifyers 
	char *token = strtok(buff , " ");
	while(token != NULL){
		if(debugioctl){printf("debug:myfprintf - token=%s,buff=%s\n",token,buff);}
		if( !strcmp(token,"%d") ){
			formatarr[numargs++] = 'd';
		}
		else if( !strcmp(token,"%c") ){
			formatarr[numargs++] = 'c';
		}
		else if( !strcmp(token,"%f") ){
			formatarr[numargs++] = 'f';
		}
		else{
			printf("myfprintf: Wrong Qualifyers Input %s given in input: stream=%p, format=%s\n",token,stream,format);
			return -1;			
		}
		token = strtok(NULL," ");
	}
	char buffer[1024];	//resulting buffer with values
	char tmpbuf[100];	//buffer to include single value
	memset(buffer, 0, sizeof(buffer));
	va_start ( arguments, format );	
	//go over arguments, obtain values and build buffer with values
	for(int i = 0; i < numargs; i++){
		if(formatarr[i] == 'd'){
			num = va_arg ( arguments, int );	//in case of %d  - the int  expected
			if(debugioctl){printf("debug:myfprintf - param #%d - got int %d from va_arg\n",i,num);}
			if( strlen(buffer) ){ strcat(buffer," "); }	//add space after previous value
			memset(tmpbuf, 0, sizeof(tmpbuf));
			sprintf(tmpbuf,"%d",num);
			strcat(buffer, tmpbuf);
		}
		else if(formatarr[i] == 'c'){
			chr = va_arg ( arguments, int );	//in case of %c  - the char  expected
			if(debugioctl){printf("debug:myfprintf - param #%d - got char %c from va_arg\n",i,chr);}
			if( strlen(buffer) ){ strcat(buffer," "); }	//add space after previous value
			memset(tmpbuf, 0, sizeof(tmpbuf));
			sprintf(tmpbuf,"%c",chr);
			strcat(buffer, tmpbuf);
		}
		else if(formatarr[i] == 'f'){
			flt = va_arg ( arguments, double );	//in case of %f  - the float  expected
			if(debugioctl){printf("debug:myfprintf - param #%d - got float %f from va_arg\n",i,flt);}
			if( strlen(buffer) ){ strcat(buffer," "); }	//add space after previous value
			memset(tmpbuf, 0, sizeof(tmpbuf));
			sprintf(tmpbuf,"%f",flt);
			strcat(buffer, tmpbuf);
		}		
	}
	va_end ( arguments );                  
	if( !(rc = myfwrite( buffer, 1, strlen(buffer), stream )) ){		
		printf("myfprintf: Error writing buffer %s to stream=%p, format=%p\n",buffer,stream,format);
		return EOF;
	}
	return rc;		
}