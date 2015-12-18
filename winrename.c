/* 
 * C version of the .pdf renamer for Congleton Chronicle. 
 * Faster than Java!
 * By Mark Tranter (tranter.m@sky.com)
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* Control string for sprintf calls. */
#define BASE "Con%c%s%sP0%s.pdf"

/* Current correct lengths (plus one) for incoming and outgoing fnames.
    Must be len + 1 to allow for terminator. */
#define LIN 13
#define LOUT 18

/* Function prototypes. */
void exErr(char *msg);
int isDigit(char *str);
int fMove(char *source, char *dest);

int main(int argc, char *argv[])
{

    if (argc != 2)
        exErr("Usage: winrename.exe PATH\n(eg. D:\\FTP\\)\n");

    /* Declarations. */
    char fname[LIN], new[LOUT];
    char pathOut[100], pathIn[100], curr[100];
    int j, k, lenIn, lenOut, lCurr, m, d;
    char page[3], day[3], month[4] = "";
    char *eds = "CBSA", *edn = "1234", *ptr;
    char e;

    /* Find the month. */
    char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    struct tm *t;
    time_t now;
    time(&now);
    t = localtime(&now);
    m = t ->tm_mon;
    d = t ->tm_mday;

    /* Start main part of the program. */
    printf("*** Renamer ***\n");

    /* Prepare path variables. */
    sprintf(pathIn, "%sC To Send\\", argv[1]);
    strcpy(curr, pathIn);
    sprintf(pathOut, "%sC Sent\\", argv[1]);

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
        /* Prepare the path strings at start of each iteration.*/
        pathIn[lenIn] = '\0';
        pathOut[lenOut] = '\0';
        curr[lCurr] = '\0';

        /* Prepare to work on the filename. */
        strcpy(fname, drecord -> d_name);
        k = strlen(fname);

        /* Check it is a .pdf and correct length. */
        if (k != (LIN - 1) || strcasecmp(".pdf", &fname[k - 4]) != 0)
            continue;

        /* Get the edition number */
        ptr = strchr(eds, fname[0]);
        j = (int)(ptr - eds);
        if (j < 0 || j > 3)
           continue;
        e = edn[j];

        /* Check if month is set, if not, set it. */
        if (!strcmp(month, ""))
        {
            sprintf(day, "%.2s", &fname[4]);
            if (!isDigit(day))
                continue;
            if (atoi(day) < d)
                m++;
	    /* Must use modulus to avoid overflowing months array. */
            strcpy(month, months[m%12]);
        }

        /* Check page number is digit and add to new filename. */
        sprintf(page, "%.2s", &fname[6]);
        if (!isDigit(page))
            continue;

        /* Add the generated details to new. */
        sprintf(new, BASE, e, month, day, page);
        printf("%s -> %s\n", fname, new);

        /* Prepare new path strings. */
        strcat(pathIn, fname);
        strcat(pathOut, fname);
        strcat(curr, new);

        /* Do the move first. */
        if (fMove(pathIn, pathOut) != 0)
		exErr("Move failed.\n");

        /* Following lines do the renaming. */
        if(rename(pathIn, curr) != 0)
            exErr("Rename failed.\n");
    }

    closedir(dhandle);

    return 0;
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

   while ((n = fread(ptr, 1, 4096, org)) == 4096)
	   fwrite(ptr, 1, 4096, dup);  

   fwrite(ptr, 1, n, dup);
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

