/* C version of the .pdf renamer for Congleton Chronicle. */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* Control string for sprintf calls. */
#define BASE "Con%d%s%sP0%s.pdf"

/* Current correct lengths for incoming and outgoing fnames. */
#define LIN 12
#define LOUT 18


/* Function prototypes. */
void exErr(char *msg);
int isDigit(char *str);
int fMove(char *source, char *dest);
void slice(char *dest, char *src, int l);

int main(int argc, char *argv[])
{
    /* Prepare to time execution */
    clock_t dur = clock();

    if (argc != 2)
        exErr("Please supply exactly one argument.");
    
    /* Declarations. */
    char fname[LIN], new[LOUT];
    char pathOut[100], pathIn[100], curr[100];
    int i, lenIn, lenOut, lCurr, e;
    char page[3], day[3], month[4] = "";
    char *eds = "CBSA"; 
    char *ptr;

    /* Find the month. */
    char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    struct tm *t;
    time_t now;
    time(&now);
    t = localtime(&now);
    int m = t ->tm_mon;
    int d = t ->tm_mday;

    /* Start main part of the program. */
    printf("*** Renamer ***\n");

    /* Prepare path variables. */
    strcpy(pathIn, argv[1]);    
    strcpy(pathOut, argv[1]);
    strcpy(curr, argv[1]);
    strcat(pathIn, "/CToSend/");
    strcat(pathOut, "/CSent/");
    strcat(curr, "/CToSend/");
    lenIn = strlen(pathIn);
    lenOut = strlen(pathOut);
    lCurr = strlen(curr);

    /* Open the directory and look for files */
    DIR *dhandle;
    struct dirent *drecord;
    dhandle = opendir(pathIn);
    if (dhandle == NULL)
        exErr("Could not open directory.\n");

    while ((drecord = readdir(dhandle)) != NULL)
    {
        /* Prepare the path strings */
        pathIn[lenIn] = '\0';
        pathOut[lenOut] = '\0';
        curr[lCurr] = '\0';

        /* Check it is a .pdf and correct length. */
        if (strlen(drecord -> d_name) != LIN || strcasecmp(".pdf", &drecord -> d_name[LIN - 4]) != 0)
            continue;
        strcpy(fname, drecord -> d_name);
    
        /* Get the edition number */   
        ptr = strchr(eds, fname[0]);
        e = (int)(ptr - eds);
        e++;
        if (e < 1 || e > 4)
           continue; 

        /* Check if month is set, if not, set it. */
        if (!strcmp(month, ""))
        {
            slice(day, &fname[4],2);
            if (!isDigit(day))
                continue;
            if (atoi(day) < d)
                m++;  
            strcpy(month, months[m%12]);
        }

        /* Check page number is digit and add to new filename. */
        slice(page, &fname[6],2);
        if (!isDigit(page))
            continue;
       
        /* Add the generated details to the base string. */ 
        sprintf(new, BASE, e, month, day, page);  
        printf("%s -> %s\n", fname, new);
        
        /* Prepare new path strings. */
        strcat(pathIn, fname);
        strcat(pathOut, fname);
        strcat(curr, new);

        /* Do the move first. */
        fMove(pathIn, pathOut);

        /* Following lines do the renaming. */ 
        if(rename(pathIn, curr) != 0)
           exErr("Rename error.\n");
    }

    closedir(dhandle);
    printf("Execution time: %f seconds. \n", (double)(clock() - dur) / CLOCKS_PER_SEC);
}


/* Slice: returns a substring. Dest must be big enough. 
 * Pass dest, start address in src and length. */
void slice(char *dest, char *src, int l) 
{
    int i;
    for (i = 0; i < l; i++)
       *dest++ = *src++;
    *dest = '\0'; 
 
}

/* Do a file move. */
int fMove(char *source, char *dest)
{
    signed char buf[4096];
    void *ptr = (void *) buf;
    size_t n;
    FILE *org, *dup;
    
    /* Copy the file. */
    org = fopen(source, "rb");
    dup = fopen(dest, "wb");
   
    if( org == NULL || dup == NULL) 
        return 1;

   while ((n = fread(ptr, sizeof(char), 100, org)) == 100)
       fwrite(ptr, sizeof(char), 100, dup);

   fwrite(ptr, sizeof(char), n, dup);
   fclose(org);
   fclose(dup);
   return 0;
}

/* Simple error message and quit function. */
void exErr(char *msg)
{
    printf("ERROR: %s\n", msg);
    exit(1);
}

/* Tests if a string is all digits. */
int isDigit(char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    return 1;
}
 
