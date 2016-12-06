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
static char *alias[ASIZE][2];
int i, j, k, ai=0;

static void cleanAlias() {
    for(i=0; i<ai; i++) {
        for(j=0; j<2; j++) free(alias[i][j]);
    }
}

static int configure(char *config_file) {
    char *line = NULL, *line_tokens[5];
    size_t len = 0;
    ssize_t read;
    if(strcmp(config_file, ".config") == 0) printf("Using default config file\n");
    else printf("Using config file: %s\n", config_file);
    if(access(config_file, R_OK) == -1) return INV_CONFIG;
    FILE *fp = fopen(config_file, "r");
    if(fp == NULL) return INV_CONFIG;
    for(i=0; i<ASIZE; i++) {
        for(j=0; j<ASIZE; j++) alias[i][j] = (char *)(malloc(BUFSIZE*sizeof(char)));
    }
    while((read = getline(&line, &len, fp)) != -1) {
        i=0;
        while((line_tokens[i] = strtok(line, " ")) != NULL) {
            line = NULL;
            i++;
        }
        line_tokens[i-1][strlen(line_tokens[i-1])-1] = 0;
        if(strncmp(line_tokens[0], "DocumentRoot", BUFSIZE) == 0) {
            if(i == 2) strncpy(doc_root, line_tokens[1], BUFSIZE);
            else return INV_CONFIG;
        }
        else if(strncmp(line_tokens[0], "OutputArea", BUFSIZE) == 0) {
            if(i == 2) strncpy(out_area, line_tokens[1], BUFSIZE);
            else return INV_CONFIG;
        }
        else if(strncmp(line_tokens[0], "Alias", BUFSIZE) == 0) {
            if(i == 3 && ai < ASIZE) {
                strncpy(alias[ai][0], line_tokens[1], strlen(line_tokens[1])-1);
                strncpy(alias[ai][1], line_tokens[2], BUFSIZE);
                ai++;
            }
            else return INV_CONFIG;
        }
        else return INV_CONFIG;
    }
    if(doc_root == NULL || !strcmp(doc_root, "\0") || out_area == NULL || !strcmp(out_area, "\0")) return INV_CONFIG;
    printf("\n----------------------------------------------\nDocument Root: %s\nOutput Area: %s\n", doc_root, out_area);
    for(i = 0; i < ai; i++) printf("Alias: %s/-->%s\n", alias[i][0], alias[i][1]);
    printf("----------------------------------------------\n\n");
    fclose(fp);
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
    char config[BUFSIZE], input[BUFSIZE], dum_inf[BUFSIZE], *mod_inf_tokens[BUFSIZE], full_inf[BUFSIZE], full_outf[BUFSIZE], dummy_inf[BUFSIZE], *inf_tokens[BUFSIZE], *inf, *outf;
    // Check argc
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
        fprintf(stderr, "\nError: invalid config file given\n");
        exit(1);
    }

    // Ask user for a commands and read it into input
    while(!(stat = (getLine("Please enter command lines of the form:\n\t> <input_file> <output_file>\n>", input, BUFSIZE)))){
        int cont = 1;
        // If there's no input, exit w/ error
        if(stat == NO_INPUT){
            fprintf(stderr, "\nError: no input!\n");
            cleanAlias();
            return stat;
        }
        // If input is too long, exit w/ error
        if(stat == TOO_LONG){
            fprintf(stderr, "\nError: input too long, please limit to %d characters\n", BUFSIZE);
            cleanAlias();
            return stat;
        }
        // Tokenize input into inf(ile) and outf(ile)
        inf = strtok(input, " ");
        outf = strtok(NULL, " ");
        if(inf == NULL || outf == NULL || strtok(NULL, " ") != NULL) {
            fprintf(stderr, "\nError: input not of requested form\n");
            exit(1);
        }
        // Remove any left over alias symlinks
        for(i=0; i<ai; i++) {
            char rm_cmd[BUFSIZE];
            snprintf(rm_cmd, BUFSIZE, "rm -f %s/%s", doc_root, alias[i][0]+1);
            system(rm_cmd);
        }
        // Add all alias symlinks
        for(i=0; i < ai; i++) {
            char sym_cmd[BUFSIZE];
            snprintf(sym_cmd, BUFSIZE, "ln -s %s %s/%s", alias[i][1], doc_root, alias[i][0]+1);
            system(sym_cmd);
        }
        // Iterate through aliases and search output files for each
        // If it's there, report an error. No aliases in output values
        for(i=0; i<ai; i++) {
            char *c;
            if((c = strstr(outf, alias[i][0])) != NULL) {
                fprintf(stderr, "\nError: aliases not allowed in output file\n");
                cont = 0;
            }
        }

        // Add document root and output area to full input file path and full
        // output file paths respectively
        strncpy(full_inf, doc_root, BUFSIZE);
        strncpy(full_outf, out_area, BUFSIZE);
        strncat(full_inf, "/", BUFSIZE);
        strncat(full_outf, "/", BUFSIZE);
        /*
         *  Add the modified input file name and output file name with the
         *  aliases replaced accordingly to the full input file path and full
         *  output file path respectively
         */
        strncat(full_inf, inf, BUFSIZE);
        strncat(full_outf, outf, BUFSIZE);
        // Need dummy_inf to tokenize
        strncpy(dummy_inf, full_inf, BUFSIZE);
        // Tokenize full input file path by "/"
        inf_tokens[0] = strtok(dummy_inf, "/");
        i = 1;
        while((inf_tokens[i] = strtok(NULL, "/")) != NULL) i++;
        k = 0;
        /*
         *  Search out "..", use it to remove the preceeding directory.
         *  Do this for input file paths, output handled by chroot. We will then
         *  search for the document root in the final result. If they are not
         *  found, we have determined an attempt to get out of the document root
         */
        for(j = 0; j < i; j++) {
            if(!strcmp(inf_tokens[j], "..") && k!=0) k--;
            else if(!strcmp(inf_tokens[j], "..") && k==0) {
                fprintf(stderr, "\nError: you are trying to get out of document root! Stop that!\n");
                cont = 0;
            }
            else {
              mod_inf_tokens[k] = inf_tokens[j];
              k++;
            }
        }
        for(j = 0; j < k; j++) {
            if(j==0) strncpy(dum_inf, "/", BUFSIZE);
            strncat(dum_inf, mod_inf_tokens[j], BUFSIZE);
            strncat(dum_inf, "/", BUFSIZE);
        }

        if(strstr(dum_inf, doc_root) == NULL) {
          fprintf(stderr, "\nError: you are trying to get out of document root! Stop that!\n");
          cont = 0;
        }

        // If we pass all these checks and the input file exists, execute the
        // copy!!
        if( cont && access( full_inf, R_OK ) != -1 ) {
            // file exists
            FILE *fpi, *fpo;
            fpi = fopen(full_inf, "r");
            if(fpi == NULL) {
                fprintf(stderr, "\nError: could not open input file\n");
                cont = 0;
            }
            chdir(out_area);
            chroot(out_area);
            fpo = fopen(outf, "w");
            if(fpo == NULL) {
                fprintf(stderr, "\nError: you are trying to get out of output area! Stop that!\n");
                cont = 0;
            }
            if(cont) {
                int a;
                while ((a = fgetc(fpi)) != EOF) fputc(a, fpo);
                fclose(fpi);
                fclose(fpo);
                printf("\n----------------------------------------------\nSuccess! %s copied to %s\n----------------------------------------------\n", full_inf, full_outf);
            }
        }
        else if(cont) fprintf(stderr, "\nError: given input file doesn't exist\n");
    }
    printf("\n\n");
    // Free all the aliases
    for(i=0; i<ai; i++) {
        char rm_cmd[BUFSIZE];
        snprintf(rm_cmd, BUFSIZE, "rm -f %s/%s", doc_root, alias[i][0]+1);
        system(rm_cmd);
    }
    cleanAlias();
    return OK;
}
