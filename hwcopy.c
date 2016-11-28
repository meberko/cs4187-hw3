#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define BUFSIZE 1024
#define ASIZE   256
#define OK          0
#define INV_CONFIG  1
#define NO_INPUT    1
#define TOO_LONG    2

char doc_root[BUFSIZE], out_area[BUFSIZE];
char *alias[ASIZE][ASIZE];
int i, j;

static void cleanAlias() {
    for(i=0; i<ASIZE; i++) {
        for(j=0; j<ASIZE; j++) free(alias[i][j]);
    }
}

static int configure(char *config_file) {
    if(strcmp(config_file, ".config") == 0) printf("Using default config file\n");
    else printf("Using config file: %s\n", config_file);
    if(access(config_file, R_OK) == -1) return INV_CONFIG;
    for(i=0; i<ASIZE; i++) {
        for(j=0; j<ASIZE; j++) alias[i][j] = (char *)(malloc(BUFSIZE*sizeof(char)));
    }
    return OK;
}

// Static method for getting lines from stdin
static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL) return NO_INPUT;

    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    buff[strlen(buff)-1] = '\0';
    return OK;
}

int main(int argc, char *argv[]) {
    int stat;
    char config[BUFSIZE], input[BUFSIZE];
    if(argc == 1) {
        strncpy(config, ".config", BUFSIZE);
    }
    else if(argc == 3) {
        if(strcmp(argv[1], "-c") != 0) {
            fprintf(stderr, "Usage: hwcopy [-c <config_file>]\n");
            exit(1);
        }
        strncpy(config, argv[2], BUFSIZE);
    }
    else {
        fprintf(stderr, "Usage: hwcopy [-c <config_file>]\n");
        exit(1);
    }

    // Configure
    if(configure(config) == INV_CONFIG) {
        fprintf(stderr, "Error: invalid config file given\n");
        exit(1);
    }

    // Ask user for a filenames and read it into input
    while(!(stat = (getLine("Please enter command lines of the form:\n\t> <input_file> <output_file>\n>", input, BUFSIZE)))){
        printf("%s\n", input);
        // If there's no input, exit w/ error
        if(stat == NO_INPUT){
            printf("Error: no input!\n");
            cleanAlias();
            return stat;
        }
        // If input is too long, exit w/ error
        if(stat == TOO_LONG){
            printf("Error: input too long, please limit to %d characters\n", BUFSIZE);
            cleanAlias();
            return stat;
        }
        
    }
    cleanAlias();
    return OK;
}
