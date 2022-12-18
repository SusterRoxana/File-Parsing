#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <math.h>

typedef struct __attribute__ ((packed)) {
    char sect_name[14]; 
    int sect_type;
    int sect_offset;
    int sect_size;
}sections;

void listD(const char *path, int* OK)
{
    DIR* dir = NULL;
    struct dirent *entry =NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL)
    {
        perror("Could not open directory");
        return;
    }
    if(*OK == 0)
    {
        printf("SUCCESS\n");
        *OK=1;
    }
    while((entry = readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath,512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0)
            {
                printf("%s\n", fullPath);
            }
        }
    }
    closedir(dir);
}



void listRec (const char *path, int *OK)
{
    DIR* dir; 
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir=opendir(path);
    if(dir == NULL)
    {
        perror("could not open the directory");
        return; 
    }

    if(*OK == 0)
    {
        printf("SUCCESS\n");
        *OK=1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry ->d_name, ".")!= 0 && strcmp(entry->d_name,"..")!=0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0)
            {
                printf("%s\n", fullPath);
                *OK=1;
                if(S_ISDIR(statbuf.st_mode))
                {
                    listRec(fullPath,OK);
                }
            }
        }
    }
    closedir(dir);
}
 

 

void listOnName(const char *path, char* string, int recursive, int *OK)
{
    DIR* dir =NULL;
    dir = opendir(path);
    struct dirent *entry =NULL;
    char fullPath[512];
    struct stat statbuf;

    if(dir == NULL)
    {
        perror("Could not open the directory");
        return ;
    }

    if(*OK == 0)
    {
        printf("SUCCESS\n");
        *OK=1;
    }

    while((entry = readdir(dir)) !=  NULL)
    {
        if(strcmp(entry->d_name, ".")!= 0 && strcmp(entry->d_name, "..") != 0 )
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            
                if(lstat(fullPath,&statbuf) == 0){

                //reverse(entry->d_name);
                char * new;
                new= entry->d_name+ strlen(entry->d_name) - strlen(string);
                //if(strstr(entry->d_name,string)==entry->d_name)
                if(strcmp(new,string)==0)
                {
                    printf("%s\n", fullPath);
                }
                if(S_ISDIR(statbuf.st_mode)&&recursive ==1)
                {
                    listOnName(fullPath,string,recursive,OK);
                }


            }
    }
    }
    closedir(dir);
}

void listOnFile (const char *path,int size, int recursive, int *OK)
{
    DIR* dir= NULL;
    dir = opendir(path);
    struct dirent *entry= NULL;
    char fullPath[512];
    struct stat statbuf;

    if(dir == NULL)
    {
        perror("Could not open directory");
        return;
    }

    if(*OK == 0 )
    {
        printf("SUCCESS\n");
        *OK = 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") !=0)
        {
            snprintf(fullPath, 512,"%s/%s", path, entry->d_name);
            if(lstat (fullPath, &statbuf) == 0)
            {
                   if(S_ISDIR(statbuf.st_mode) && recursive ==1 ) 
                 {
                   listOnFile(fullPath,size,recursive,OK);
                  }
                   if(S_ISREG(statbuf.st_mode))
                   {
                       if(statbuf.st_size < size)
                    printf("%s\n", fullPath);
                   }
            }}}
    closedir(dir);
}


void verifyFile(char *path)
{
    int fd= open(path, O_RDONLY);
    if(fd ==-1)
    {
        perror("could not open the file");
        return;
    }
    off_t size =lseek(fd,0,SEEK_END); //goes to the end of the file 
    lseek(fd,size-4,SEEK_SET); //magic has 4 bytes
    char* myMagic= (char* )calloc(5, sizeof(char));
    if(read(fd,myMagic,4)!=4)
    {
        printf("ERROR\nwrong magic\n");
        return;
    }
    if(strcmp(myMagic,"Q2B7") == 0 )
    {
        unsigned headerSize =0;
        lseek(fd,size-6,SEEK_SET);
        if(read(fd,&headerSize,2)!=2)
        {
            free(myMagic);
            printf("ERROR\nwrong size of header\n");
            return;
        }
        lseek(fd,size-headerSize,SEEK_SET);
        unsigned version=0;
        if(read(fd,&version,1)!= 1)
        {
            free(myMagic);
            printf("ERROR\nwrong version\n");
            return;
        }
        if(version<13 || version>58)
        {
            free(myMagic);
            printf("ERROR\nwrong version\n");
            return;
        }
        unsigned noSections =0;
        if(read(fd,&noSections,1)!=1)
        {
            free(myMagic);
            printf("ERROR\nwrong sect_nr\n");
            return;
        }
        if(noSections<8 || noSections>10)
        {
            free(myMagic);
            printf("ERROR\nwrong sect_nr\n");
            return;
        }

        sections *fileSections = (sections*) malloc(noSections * sizeof(sections));
        for(int i=0; i<noSections; i++)
        {
            if(read(fd,(fileSections+i), sizeof(sections)) !=sizeof(sections))
            {
                perror("error");
                free(fileSections);
                free(myMagic);
                return;
            }
            if(!((fileSections+i)->sect_type == 63 || (fileSections+i)->sect_type == 52))
            {
                printf("ERROR\nwrong sect_types\n");
                free(fileSections);
                free(myMagic);
                return;   
            }
     }

     printf("SUCCESS\nversion=%d\nnr_sections=%d\n", version, noSections);
     for(int j=0; j<noSections;j++)
     {
         char name[15];
         memcpy(name,(fileSections+j)->sect_name,14);
         name[14]=0;
         printf("section%d: %s %d %d\n", j+1, name, (fileSections+j)->sect_type, (fileSections+j)->sect_size);
     }
     free(fileSections);
    }
    else
    {
        printf("ERROR\nmagic wrong\n");
        return;
    }
    free(myMagic);
    
}

sections* verifyForLine(char *path)
{
    int fd = open(path, O_RDONLY);
    if(fd == -1)
    {
        return NULL;
    }

    off_t size = lseek(fd,0,SEEK_END);
    lseek(fd,size-4,SEEK_SET);
    char* myMagic = (char*)calloc(5,sizeof(char));
    if(read(fd,myMagic,4) != 4 )
    {
        free(myMagic);
        return NULL;
    }
     if(strcmp(myMagic,"Q2B7") == 0 )
    {
        unsigned headerSize = 0;
        lseek(fd,size-6,SEEK_SET);
        if(read(fd, &headerSize,2) != 2)
        {
            free(myMagic);
            return NULL;
        }
        lseek(fd, size-headerSize, SEEK_SET);
        unsigned version =0;
        if(read(fd,&version,1)!=1)
        {
            free(myMagic);
            return NULL;
        }
        if(version<13 || version>58)
        {
            free(myMagic);
            return NULL;
        }
        unsigned noSections =0;
        if(read(fd,&noSections,1)!=1)
        {
           free(myMagic);
            return NULL;
        }
        if(noSections < 8 || noSections>10)
        {
            free(myMagic);
            return NULL;
        }
        sections* fileSections = (sections*)calloc((noSections+1),sizeof(sections));
        for(int i=0;i<noSections;i++)
        {
            if(read(fd,(fileSections+i),sizeof(sections)) != sizeof(sections) )
            {
                free(myMagic);
                free(fileSections);
                return NULL;
            }

            if(!((fileSections+i)->sect_type==63 || (fileSections+i)->sect_type==52))
            {
               free(myMagic);
                free(fileSections);
                return NULL;
            }
        }

        (fileSections+noSections)->sect_size=-1;
        sections* a = fileSections;
        free(myMagic);
        free(fileSections);
        return a;
    }
    else
    {
   free(myMagic);
        return NULL;
    }
    free(myMagic);

}

void searchLine (char *path, int section, int line)
{
    sections* secFile = verifyForLine(path);
    int fd = open(path,O_RDONLY);
    if(secFile == NULL || fd==-1)
    {
        free(secFile);
        perror("ERROR\ninvalid file");
        return;
    }
    int d=0; //how many  sections I do have 
    while((secFile+d)->sect_size != -1)
    {
        d++;
    }

    if(section>d)
    {
       free(secFile);
        printf("ERROR\ninvalid section");
        return;
    }

    sections searchedSection = *(secFile+section-1);
    lseek(fd,searchedSection.sect_offset+searchedSection.sect_size-1,SEEK_SET);
    char *string = (char*)calloc(searchedSection.sect_size,sizeof(char));
    char c=0;
    int j=0;
    for(int i=searchedSection.sect_offset+searchedSection.sect_size-1; i>=0; i--)
    {
        lseek(fd,i,SEEK_SET);
        if(read(fd,&c,1)!=1)
        {
        free(string);
          free(secFile);
            printf("ERROR\n");
            return;
        }
        if(c == 0x0A)
        {
            line--;
            i--;
            lseek(fd,i,SEEK_SET);
            if(read(fd,&c,1)!=1)
            {
               free(string);
                free(secFile);
                printf("ERROR\n");
                return;
            }

        }
        if(line ==1)
        {
            *(string+j) = c;
            j++;
        }

    }
    if(line>1)
    {
        free(secFile);
       free(string);
        printf("invalid line");
        return;
    }

    printf("SUCCESS\n%s\n",string);
    free(string);
    free(secFile);
}

void findFile (char * path, int* OK)
{
    DIR *dir =NULL;
    dir = opendir(path);
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    if(dir ==NULL)
    {
        perror("ERROR\ninvalid directory path");
        return;
    }
    if(*OK == 0)
    {
        printf("SUCCESS\n");
        *OK=1;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") !=0)
        {
            snprintf(fullPath,512,"%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0)
            {
                if(S_ISDIR(statbuf.st_mode))
                {
                    findFile(fullPath,OK);
                }
                if(S_ISREG(statbuf.st_mode))
                {
                    sections* secFile = verifyForLine(fullPath);
		            if(secFile != NULL)
                    {
                        int fd=open(fullPath,O_RDONLY);
                        if(fd==-1)
                        {
                            perror("1");
                            printf("1");
                            return;
                        }
                        int d=0; // nr fo sections i do have 
                        while((secFile+d)->sect_size != -1)
                        { 
                            d++;
                        }
                        for(int i=0;i<d;i++) //go for  each section 
                        {
                            sections section = *(secFile+i);
                            
                            int line=0; //nr of lines
                            char c=0;
                            lseek(fd,section.sect_offset-1,SEEK_SET);
                             for(int j=section.sect_offset; j<section.sect_offset+section.sect_size && line <13; j++)
                             {
                                if(read(fd,&c,1)!=1)
                                {
                                    return;
                                }
                                //printf("%c", c);
                                if(c == 0x0A)
                                {
                                     line++;
                                 }
                             }
                           // printf("%d\n",line); 
                            if(line +1 >= 13 )
                              {
                                 printf("%s\n ",fullPath);
                                 break;
                             }
                        }

                    }   
                
                } 
            }  
        
        }

    }



    closedir(dir);
}

int main(int argc, char **argv){

    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0)
        {
            printf("25176\n");
            
        }

        if(argc <=5)
        {
            int ok=0;
            if(strcmp(argv[1],"list") == 0)
            {
                if(argc == 3)
                {
                    if(strstr(argv[2], "path=") != NULL)
                    {
                        int n = strlen(argv[2]);
                        char *path = (char*) calloc (n-4, sizeof(char));
                        strncpy(path, argv[2]+5 ,n-5);
                        listD(path,&ok);
                        free(path);
                    }
                    else
                    {
                        printf("invalid input\n");
                        return -1;
                    }
    
                }

                if(argc ==4)
                {
                    if(strstr(argv[3],"path=") != NULL)
                    {
                        int n = strlen(argv[3]);
                        char * path = (char *) calloc (n-4, sizeof(char));
                        strncpy(path, argv[3]+5, n-5);
                        if(strstr(argv[2], "recursive") != NULL)
                        {
                           listRec(path,&ok);
                        }
                        else
                        {
                            if(strstr(argv[2], "name_ends_with=") != NULL)
                            {
                                int n= strlen(argv[2]);
                                char *name = (char* ) calloc(n-14, sizeof(char));
                                strncpy(name,argv[2]+15,n-15);
                                listOnName(path,name,0,&ok);
                                 free(name);
                            }
                           else
                            {
                               if(strstr(argv[2],"size_smaller=") != NULL)
                                {
                                  char n = strlen(argv[2]);
                                  char *size = (char *) calloc (n-12, sizeof(char));
                                  strncpy(size, argv[2]+13,n-13);
                                 
                                  listOnFile(path,atoi(size),0,&ok );
                                   free(size);
                                }
                            }
                        }
                         free(path);
                    }

                    
                }
                if(argc == 5)
                {
                    if(strstr(argv[4], "path=") != NULL )
                    {
                        int n = strlen(argv[4]);
                        char *path = (char * ) calloc (n-4, sizeof(char));
                        strncpy(path,argv[4]+5,n-5);
                        if(strstr(argv[3],"recursive") != NULL)
                        {
                            if(strstr(argv[2], "name_ends_with=") != NULL)
                            {
                                int n= strlen(argv[2]);
                                char *name = (char* ) calloc(n-14, sizeof(char));
                                strncpy(name,argv[2]+15,n-15);
                                listOnName(path,name,1,&ok);
                                free(name);
                           }
                           if(strstr(argv[2],"size_smaller=") != NULL)
                           {
                                char n = strlen(argv[2]);
                                char *size = (char *) calloc (n-12, sizeof(char));
                                strncpy(size, argv[2]+13,n-13);
                                listOnFile(path,atoi(size),1,&ok );
                                free(size);
                           }
                        }
                        else 
                        if(strstr(argv[2],"recursive") != NULL)
                        {
                             if(strstr(argv[3],"size_smaller=") != NULL)
                           {
                                char n = strlen(argv[3]);
                                char *size = (char *) calloc (n-12, sizeof(char));
                                strncpy(size, argv[3]+13,n-13);
                                listOnFile(path,atoi(size),1,&ok );
                                free(size);
                           }
                        }
                        free(path);
                     
                    }
                }
             
            }  

        } 
            
     }

if(argc<=3){
     if(strcmp(argv[1],"parse") == 0)
    {
        if(strstr(argv[2],"path=") != NULL)
        {
           int n = strlen(argv[2]);
           char * path = (char *) calloc (n-4, sizeof(char));
           strncpy(path, argv[2]+5, n-5);
           verifyFile(path);
           free(path);
    
        }
    }
}

if(argc<=5)
{
    if(strcmp(argv[1],"extract") ==0)
    {
        if(strstr(argv[2],"path=") !=NULL)
        {
            char* path = argv[2] +5;
            if(strstr(argv[3], "section=") != NULL)
            {
                char *section = argv[3] +8;
                int nrSection= atoi(section);
                if(strstr(argv[4],"line=") != NULL)
                {
                    char* line = argv[4]+5;
                    int lineNumber = atoi(line);
                    searchLine(path,nrSection,lineNumber);
    
                }
                else
                {
                
                    printf("invalid input");
                    return -1;
                } 
            }
                else
                {
                    printf("invalid input");
                    return -1;
                }
        
        }
        else{
            printf("invalid input");
            return-1;
        }
    
    }
}
    if(argc >=3)
{
    if(strcmp(argv[1], "findall") ==0)
    {
    
        if(strstr(argv[2], "path=") != NULL)
        {
            char *path = argv[2]+5;
            int ok=0;
            findFile(path,&ok);
        }
    }
}
}
                    



 
