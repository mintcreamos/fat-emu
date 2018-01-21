#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<ctype.h>
//#include<stdio_ext.h>
#include<time.h>

char* imgfile;
int imgsize;
int partition_begin;
int partition_size;
int sector_per_cluster;
int boot;
int fat_num;
int dir_num;
int fat_sec;
int at_fat;
int at_root;
int at_data;
void* fat_buff;
char* dir_buff;
FILE* handle;
int first_cluster;
int last_fat;
char* title;
int is_dirty;
int dll_dirty;
char* dll_buff;
int dll_cluster;
int dll_size;
int dll_num;
int is_new;
char cmd[20];
char arg[256];
int i;
char filename[13];
int fat_buff_size;
int dir_buff_size;
char cache[512];

void get_parameter();
void do_insert(char* path);
void my_seek(int);
void close_file();
int create_file(char*,int,char,int,int*,char*);
//int create_file(char*,int);
int alloc_fat();
void formatName(char*);
void my_read(void*,int);
void my_write(void*,int);
void do_dir(int,char*);
void insert_boot();
void write_sector();
void make_time(int);

int get_short(char* buff,int index){
		return (int)*(short*)(buff+index);
	}
int get_int(char* buff,int index){
		return (int)*(int*)(buff+index);
	}
int get_byte(char* buff,int index){
		return (int)*(unsigned char*)(buff+index);
	}
void put_short(char* buff,int index,short value){
		*(short*)(buff+index)=value;
	}
void put_int(char* buff,int index,int value){
		*(int*)(buff+index)=value;
	}
void put_byte(char* buff,int index,char value){
		*(char*)(buff+index)=value;
	}

void do_erase();

int check_handle(){
      if(!handle){
	handle=fopen(imgfile,"rb+");
	if(!handle){
	  printf("cannot open image\n");
	  return 0;
	}
      }
      return 1;
}
int main(int argc,char** argv){
	int i,j;
	if(argc<2)
		imgfile="disk.img";
	else{
		imgfile=argv[1];
	}
	handle=fopen(imgfile,"rb+");
	if(!handle){
		printf("cannot open %s\n",imgfile);
		exit(1);
	}
	i=fseek(handle,0,SEEK_END);
	if(i!=0){
	    printf("seek error\n");
	    exit(1);
	}
	imgsize=ftell(handle);
	if(imgsize==0){
		printf("zero image file\n");
		exit(1);
	}	
	get_parameter();
	
	last_fat=2;
	is_dirty=0;
	dll_dirty=0;
	dll_buff=0;
	cache[0]=0;
	while(1){
		printf(":>");
		fflush(stdin);
		i=scanf("%s",cmd);
		if(i<1){
			printf("byte-byte\n");
			close_file();
			exit(1);
		}
		j=strcmp(cmd,"i");
		if(j==0){
			i=scanf("%s",arg);
			if(i<1){
			  close_file();
			   printf("byte-byte\n");
			  exit(1);
			 }
			if(!handle){
				printf("image not open yet\n");
				continue;
			}
			i=strlen(arg);
			if(i<512)
			  strncpy(cache,arg,i+1);
			do_insert(arg);
			continue;
		}

		j=strcmp(cmd,"id");
		if(j==0){
			i=scanf("%s",arg);
			if(i<1){
			  close_file();
			   printf("byte-byte\n");
			  exit(1);
			 }
			if(!handle){
				printf("image not open yet\n");
				continue;
			}
			insert_dll(arg);
			continue;
		}

		
		i=strcmp(cmd,"x");
		if(i==0){
			close_file();
			printf("byte-byte\n");
			exit(1);
		}
		i=strcmp(cmd,"c");
		if(i==0){
			if(handle){
				printf("not close yet\n");
				continue;
			}
			handle=fopen(imgfile,"rb+");
			if(!handle){
				printf("cannot open image\n");
				continue;
			}
			printf("capture ok\n");
			continue;
		}
		i=strcmp(cmd,"r");
		if(i==0){
			if(!handle){
				printf("image not open yet\n");
				continue;
			}
			printf("release ok\n");
			close_file();		
			continue;
		}
		i=strcmp(cmd,"d");
		if(i==0){
		  if(!handle){
				printf("image not open yet\n");
				continue;
		  }
		  do_dir(dir_num,dir_buff);
		  continue;
		}

		i=strcmp(cmd,"f");
		if(i==0){
		  if(!handle){
		    printf("image not open\n");
		    continue;
		  }
		  if(dll_buff)
		    do_dir(dll_num,dll_buff);
		  else
		    if(prepare_dll()==0)
		      do_dir(dll_num,dll_buff);
		  continue;
		}
		i=strcmp(cmd,"p");
		if(i==0){
		  if(!handle){
		    printf("image not open yet\n");
				continue;
		  }
		  if(cache[0]==0){
		    printf("no cache path\n");
		    continue;
		  }
		  printf("insert %s\n",cache);
		  do_insert(cache);
		  continue;
		}
		i=strcmp(cmd,"b");
		if(i==0){
		  if(!handle){
		    printf("image not open yet\n");
		    continue;
		  }
		  i=scanf("%s",arg);
			if(i<1){
			   printf("miss boot file\n");
			  continue;
			}
		  insert_boot();
		  continue;
		}
		i=strcmp(cmd,"h");
		if(i==0){
			printf("b boot\np last action\nx quit\ni insert\nd list\nr release\nc capture\nk insert kernel\nl insert loader\nw flush\ne erase\n");
			continue;
		}

		i=strcmp(cmd,"e");
		  if(i==0)
		    if(check_handle()){
		      do_erase();
		      continue;
		    }
		i=strcmp(cmd,"k");
		if(i==0){
			if(!handle){
		    	handle=fopen(imgfile,"rb+");
					if(!handle){
						printf("cannot open image\n");
						continue;
					}
				printf("capture ok\n");
		  	}
		  		printf("insert kernel...\n");
		  		do_insert("../kernel/kernel.sys");
				close_file();
			  continue;
		}
		i=strcmp(cmd,"l");
		if(i==0){
			if(!handle){
		    	handle=fopen(imgfile,"rb+");
					if(!handle){
						printf("cannot open image\n");
						continue;
					}
				printf("capture ok\n");
		  	}
			  printf("insert loader...\n");
			  do_insert("../loader/loader.sys");
				close_file();
			  continue;
		}
		i=strcmp(cmd,"w");
		if(i==0){
			if(!handle){
		    	handle=fopen(imgfile,"rb+");
					if(!handle){
						printf("cannot open image\n");
						continue;
					}
				printf("capture ok\n");
		  	}
			  printf("write sector...\n");
			write_sector();
			continue;
		}
		printf("unknow command\n");
	}
}

 void my_write(void* buff,int size){
    i=fwrite(buff,1,size,handle);
    if(i!=size){
     printf("write error\n");
     exit(1);
   }
 }

void do_erase(){
  for(i=0; i<dir_num; i++){
    if(dir_buff[i*32]==0 || dir_buff[i*32]==0xe5)
      continue;
    else
      dir_buff[i*32]=0;
  }
  int fat_count=(fat_sec<<9)/2;
  dll_dirty=0;
  for(i=0; i<fat_count; i++)
    put_short((char*)fat_buff,i*2,(short)0);

        my_seek(at_fat);
		      my_write(fat_buff,fat_buff_size);
		      my_seek(at_root);
		      my_write(dir_buff,dir_buff_size);
		      printf("erase ok\n");
		      is_dirty=0;
		
}
 void insert_boot(){
      int file_size;
      FILE* file;
      unsigned char code[512];
      file=fopen(arg,"r");
      if(!file){	
	printf("boot file not open\n");
	return;
      }
      fseek(file,0,SEEK_END);
      file_size=ftell(file);
      if(file_size>512){
	printf("boot file too large\n");
	fclose(file);
	return;
      }
      fseek(file,0,SEEK_SET);
      my_seek((partition_begin<<9)+0x3e);
      i=fread(code,1,file_size,file);
      if(i!=file_size){
	printf("read boot file error\n");
	fclose(file);
	return;
      }
      i=fwrite(code+0x3e,1,file_size-0x3e,handle);
      if(i!=file_size-0x3e){
	printf("write boot error\n");
	exit(1);
      }
      printf("boot size %d\n",file_size);
      fclose(file);
}


void close_file(){
	if(handle){
	    if(is_dirty){
	      my_seek(at_fat);
	      my_write(fat_buff,fat_buff_size);
	      my_seek(at_root);
	      my_write(dir_buff,dir_buff_size);
	      printf("flush cache\n");
	      is_dirty=0;
	    }

	    if(dll_dirty && dll_buff){
	      my_seek(at_data+(dll_cluster-2)*(sector_per_cluster<<9));
	      my_write(dll_buff,dll_size);
	      dll_dirty=0;
	      printf("flush dll\n");
	    }
	    fclose(handle);
	    handle=NULL;

	}
}


void do_insert(char* arg) {
	int sector_size;
	int fats;
	FILE* file;
	void* file_buff;
	int block_size;
	int m;
	int current_fat;
	int file_size;
	int k;
	file=fopen(arg,"r");
	if(!file){	
		printf("inserted file not exist\n");
		return;
	}
	fseek(file,0,SEEK_END);
	file_size=ftell(file);
	fseek(file,0,SEEK_SET);
	sector_size=sector_per_cluster<<9;
	m=file_size+sector_size-1;
	fats=m/sector_size;
	if(fats==0){
		printf("file is zero size\n");
		return;
	}
	file_buff=malloc(sector_size);
	if(file_buff==NULL){
		printf("cannot alloc memory\n");
		return;
	}
	i=create_file(arg,file_size,0x20,dir_num,&is_dirty,dir_buff);
	if(i!=0){
	  printf("cannot create file %d\n",i);
	  exit(1);
	}
		
	for(k=0; k<fats; k++){
		current_fat=(first_cluster-2)*sector_size;
		my_seek(at_data+current_fat);
		if(file_size>sector_size)
			block_size=sector_size;
		else
			block_size=file_size;
		m=fread(file_buff,1,block_size,file);
			
		if(m!=block_size){
			printf("read file error\n");
			goto final;
		}	
		m=fwrite(file_buff,1,block_size,handle);
		if(m!=block_size){
			printf("write file error\n");
			exit(1);
		}
		file_size-=block_size;
		if(k<fats-1){
			first_cluster=alloc_fat();
			if(first_cluster==-1){
			  printf("alloc fat error\n");
			  exit(1);
			}
	       }
	}
	final:
	fclose(file);
	free(file_buff);
}




void insert_dll(char* arg) {
	int sector_size;
	int fats;
	FILE* file;
	void* file_buff;
	int block_size;
	int m;
	int current_fat;
	int file_size;
	int k;
	file=fopen(arg,"r");
	if(!file){	
		printf("inserted file not exist\n");
		return;
	}
	fseek(file,0,SEEK_END);
	file_size=ftell(file);
	fseek(file,0,SEEK_SET);
	sector_size=sector_per_cluster<<9;
	m=file_size+sector_size-1;
	fats=m/sector_size;
	if(fats==0){
		printf("file is zero size\n");
		return;
	}
	file_buff=malloc(sector_size);
	if(file_buff==NULL){
		printf("cannot alloc memory\n");
		return;
	}
	i=create_dll(arg,file_size);		
	if(i!=0){
	  printf("cannot create file %d\n",i);
	  exit(1);
	}
		
	for(k=0; k<fats; k++){
		current_fat=(first_cluster-2)*sector_size;
		my_seek(at_data+current_fat);
		if(file_size>sector_size)
			block_size=sector_size;
		else
			block_size=file_size;
		m=fread(file_buff,1,block_size,file);
			
		if(m!=block_size){
			printf("read file error\n");
			goto final;
		}	
		m=fwrite(file_buff,1,block_size,handle);
		if(m!=block_size){
			printf("write file error\n");
			exit(1);
		}
		file_size-=block_size;
		if(k<fats-1){
			first_cluster=alloc_fat();
			if(first_cluster==-1){
			  printf("alloc fat error\n");
			  exit(1);
			}
	       }
	}
	final:
	fclose(file);
	free(file_buff);
}




void do_dir(int num,char* buff){
	  int k;
	  int m;
	  int size;
	  int cluster;

	  for(i=0; i<num; i++){
	    for(k=0; k<13; k++)
	      filename[k]=0;

	    if(buff[i*32]==0xe5)
			continue;
	  	if(buff[i*32]==0)
			return;
	  	m=0;
	    for(k=0; k<11; k++){
	      if(buff[i*32+k]==0x20){
			if(k==7)
			  filename[m++]='.';
			continue;
		  }else
			  filename[m++]=buff[i*32+k];
	      if(k==7)
			filename[m++]='.';
	    }
	    filename[k]=0;
		m--;
		if(filename[m]=='.')
			filename[m]=0;
	    size=get_int(buff,i*32+0x1c);
	    cluster=get_short(buff,i*32+0x1a);
	    printf("%s size:%d \t\tfirst:%d\n",filename,size,cluster);
	  }
}

int create_file(char* arg,int file_size, char t,int num,int* dirty,char* buff){
	first_cluster=0;
	int empty_dir=-1;
	int j,k;
	j=0;
	is_new=1;

	formatName(arg);
	
	for(i=0; i<num; i++){
		k=0;
		
		for(; k<11; k++){
			if(buff[j+k]!=filename[k])
			  break;
	  
		}
		if(k>=11){
			first_cluster=get_short(buff,j+0x1a);
			is_new=0;
			break;
		}
		if(buff[j]==0xe5)
			empty_dir=j;
		else if(buff[j]==0 ){
			empty_dir=j;
			break;
		}
		j+=32;
	}
	if(i>=num || empty_dir!=-1){
		j=empty_dir;
		if(buff[j]==0){
			for(i=0; i<11; i++)
				buff[j++]=filename[i];
		}else{
			return 1;
		}
	}
	j=j&0xfffffe0;
	if(first_cluster==0){
		first_cluster=alloc_fat();
		if(first_cluster==-1){
		  buff[j]=0;
		  return 2;
		}
	}

	put_short(buff,j+0x1a,(short) first_cluster);
	put_byte(buff,j+0xb, t);
	put_int(buff,j+0x1c,(int)file_size);
		
	make_time(j);
	*dirty=1;
	return 0;		
}

int prepare_dll(){
  dll_buff=(char*)calloc(dll_size,1);
    if(!dll_buff){
      printf("cannot alloc buff for dll");
      return -1;
    }

    i=create_file("lib",0,0x10,dir_num,&is_dirty,dir_buff);
    if(i!=0){
      printf("create dll fail");
      free(dll_buff);
      dll_buff=0;
      return -1;
    }

    if(!is_new){
      my_seek(at_data+(first_cluster-2)*(sector_per_cluster<<9));
      my_read(dll_buff,dll_size);
    }else
      dll_dirty=1;
    dll_cluster=first_cluster;
    return 0;
}
int create_dll(char* arg,int size){
  if(!dll_buff){
    dll_buff=(char*)calloc(dll_size,1);
    if(!dll_buff){
      printf("cannot alloc buff for dll");
      return -1;
    }

    i=create_file("lib",0,0x10,dir_num,&is_dirty,dir_buff);
    if(i!=0){
      printf("create dll fail");
      free(dll_buff);
      dll_buff=0;
      return -1;
    }
     if(!is_new){
       my_seek(at_data+(first_cluster-2)*(sector_per_cluster<<9));
       my_read(dll_buff,dll_size);
    }else
       dll_dirty=1;
    dll_cluster=first_cluster;
    return 0;
  }
  else{
    i=create_file(arg, size, 0x20, dll_num,&dll_dirty,dll_buff);
    if(i!=0){
      printf("create dll fail");
      return -1;
    }
  }

  return 0;
}

	void make_time(int j){
		short t=0;
		time_t timer=time(NULL);
		
		struct tm* p=localtime(&timer);
		t|=((*p).tm_hour & 0x1f) << 11;
		t|=((*p).tm_min & 0x3f) << 5;
		t|=((*p).tm_sec & 0x1f);
		put_short(dir_buff,j+14,t); //create time
		put_short(dir_buff,j+22,t); //mktime
		
		t=0;
		t|=(((*p).tm_year - 2000) & 0x7f) << 9;
		t|=((*p).tm_mon & 0xf) << 5;
		t|=((*p).tm_mday & 0x1f);
		put_short(dir_buff,j+16,t);
		put_short(dir_buff,j+18,t);
		put_short(dir_buff,j+24,t);
	}
	int alloc_fat(){
		int fat_count=(fat_sec<<9)/2;
		int fat;
		if(last_fat>=fat_count)
		  last_fat=2;
		
		if(first_cluster!=0){
			fat=get_short((char*)fat_buff,first_cluster*2);
			if(fat>0){
				return fat;
			}
		}
		for(i=last_fat; i<fat_count;i++){
			fat=get_short((char*)fat_buff,i*2);
			if(fat==0){
				put_short((char*)fat_buff,i*2, (short) -8);
				last_fat=i+1;
				if(first_cluster!=0)
					put_short((char*)fat_buff,first_cluster*2, (short) i);
				is_dirty=1;
				return i;
			}
		}
		if(last_fat>=2)
			for(i=2; i<last_fat;i++){
				fat=get_short((char*)fat_buff,i*2);
				if(fat==0){
					put_short((char*)fat_buff,i*2, (short) 0xfff8);
					last_fat=i+1;
					put_short((char*)fat_buff,first_cluster*2, (short) i);
					is_dirty=1;
					return i;
				}
			}
		last_fat=2;
		return -1;
	}

	void write_sector(){
		char buff[512];
		int fat;
		int pos;
		int fat_count=(fat_sec<<9)/2;
		int m;
		int sector_size=sector_per_cluster<<9;
		for(i=0; i<512; i++){
			buff[i]=0x55;
			i++;
			buff[i]=0xaa;
		}
		for(i=0; i<fat_count;i++){
			fat=get_short((char*)fat_buff,i*2);
			if(fat==0)
				break;
		}
		if(i>=fat_count){
			printf("no free fat\n");
			return;
		}	
		m=(fat-2)*sector_size;
		my_seek(at_data+m);
		pos=ftell(handle);
		m=fwrite(buff,1,512,handle);		
		if(m!=512){
			printf("write sector error\n");
			return;
		}
		m=pos;
		m=(m-partition_begin)>>9;
		printf("write sector at 0x%x 0x%x\n",pos,m);
		is_dirty=0;
		
	}
	void formatName(char* name){
	  int k;
		char* p=name;
	    for(k=0; k<13; k++)
	      filename[k]=0;
	    k=0;

	    for(i=0; i<strlen(name); i++){
		  if(name[i]=='/')
		    p=name+i+1;
		}
		for(i=0; i<11; i++)
			filename[i]=0x20;
		for(i=0; i<11 && k<strlen(p); i++){
			if(p[k]=='.' && i<=8){
				i=7;
				k++;
				continue;
			}
			filename[i]=toupper(p[k++]);
		}
	}
	void my_seek(int index){
		fseek(handle,index,SEEK_SET);
		i=ftell(handle);
		if(i!=index){
			printf("fseek error\n");
			exit(1);
		}
	}
	
	void my_read(void* buff,int size){
		i=fread(buff,1,size,handle);
		if(i!=size){
			printf("read error\n");
			exit(1);
		}
	}
void get_parameter(){
	char mbr[64];
	char dbr[512];
	unsigned char type;
	my_seek(0x1be);
	my_read(mbr,sizeof(mbr));
	for(i=0; i<4; i++){
	  type=mbr[i*16+4];
	  if(type==4 || type==6 || type==14){
	    partition_begin=*(int*)(mbr+i*16+8);
	    partition_size=*(int*)(mbr+i*16+12);
	    if((partition_begin+partition_size)<<9 > imgsize){
		printf("invalid partition\n");
		exit(1);
	      }
	    break;
	  }
			//printf("%d ",type);
	}
	  if(i==4){
	    printf("partition not found\n");
	    exit(1);
	  }
	  my_seek(partition_begin<<9);
	  my_read(dbr,sizeof(dbr));
		
	  sector_per_cluster=(int)(*(unsigned char*)(dbr+13));
	  dll_size = sector_per_cluster<<9;
	  dll_num = dll_size >> 5;
	  boot=(int)*(short*)(dbr+14);
	  fat_num=(int)*(unsigned char*)(dbr+16);
	  dir_num=(int)*(short*)(dbr+17);
	  fat_sec=(int)*(short*)(dbr+22);
	  at_fat=(partition_begin+boot)<<9;
	  at_root=(partition_begin+boot+fat_sec*fat_num)<<9;
	  at_data=at_root +((dir_num*32+511) & 0xFFFFFE00);
			
	  fat_buff=malloc(fat_sec<<9);
	  fat_buff_size=fat_sec<<9;
	  if(!fat_buff){
		printf("alloc memory error\n");	
		exit(1);
	  }
	  my_seek(at_fat);
	  my_read(fat_buff,fat_sec<<9);

	  dir_buff=(char*)malloc(dir_num*32);
	  dir_buff_size=dir_num*32;
	  if(!dir_buff){
		printf("alloc memory error\n");	
		exit(1);
	  }
			
	  my_seek(at_root);
	  my_read(dir_buff,dir_num*32);
}
