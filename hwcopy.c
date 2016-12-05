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
        for(j=0; j<ai; j++) free(alias[i][j]);
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
                strncpy(alias[ai][0], line_tokens[1], BUFSIZE);
                strncpy(alias[ai][1], line_tokens[2], BUFSIZE);
                ai++;
            }
            else return INV_CONFIG;
        }
        else return INV_CONFIG;
    }
    if(doc_root == NULL || !strcmp(doc_root, "\0") || out_area == NULL || !strcmp(out_area, "\0")) return INV_CONFIG;
    printf("\n----------------------------------------------\ndoc_root: %s\nout_area: %s\n", doc_root, out_area);
    for(i = 0; i < ai; i++) printf("alias: %s-->%s\n", alias[i][0], alias[i][1]);
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
    char config[BUFSIZE], input[BUFSIZE], dum_inf[BUFSIZE], dum_outf[BUFSIZE], *mod_inf_tokens[BUFSIZE], *mod_outf_tokens[BUFSIZE] , mod_inf[BUFSIZE], mod_outf[BUFSIZE], full_inf[BUFSIZE], full_outf[BUFSIZE], dummy_inf[BUFSIZE], dummy_outf[BUFSIZE], *inf_tokens[BUFSIZE], *outf_tokens[BUFSIZE], *inf, *outf;
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
        fprintf(stderr, "Error: invalid config file given\n");
        exit(1);
    }

    // Ask user for a commands and read it into input
    while(!(stat = (getLine("Please enter command lines of the form:\n\t> <input_file> <output_file>\n>", input, BUFSIZE)))){
        int cont = 1;
        // If there's no input, exit w/ error
        if(stat == NO_INPUT){
            fprintf(stderr, "Error: no input!\n");
            cleanAlias();
            return stat;
        }
        // If input is too long, exit w/ error
        if(stat == TOO_LONG){
            fprintf(stderr, "Error: input too long, please limit to %d characters\n", BUFSIZE);
            cleanAlias();
            return stat;
        }
        // Tokenize input into inf(ile) and outf(ile)
        inf = strtok(input, " ");
        outf = strtok(NULL, " ");
        if(inf == NULL || outf == NULL || strtok(NULL, " ") != NULL) {
            fprintf(stderr, "Error: input not of requested form\n");
            exit(1);
        }
        // Iterate through aliases and search input and output files for each
        // If it's there, replace the alias with its alias value
        for(i=0; i<ai; i++) {
            char *c;
            if((c = strstr(inf, alias[i][0])) != NULL) {
                strncpy(mod_inf, inf, c-inf);
                strncat(mod_inf, alias[i][1], BUFSIZE);
                strncat(mod_inf, c+strlen(alias[i][0])-1, BUFSIZE);
            }
            if((c = strstr(outf, alias[i][0])) != NULL) {
                strncpy(mod_outf, outf, c-outf);
                strncat(mod_outf, alias[i][1], BUFSIZE);
                strncat(mod_outf, c+strlen(alias[i][0])-1, BUFSIZE);
            }
        }
        if(!strcmp(mod_inf, "")) strncpy(mod_inf, inf, BUFSIZE);
        if(!strcmp(mod_outf, "")) strncpy(mod_outf, outf, BUFSIZE);

        // Add document root and output area to full input file path and full
        // output file paths respectively
        strncpy(full_inf, doc_root, BUFSIZE);
        strncpy(full_outf, out_area, BUFSIZE);
        strncat(full_inf, "/", BUFSIZE);
        strncat(full_outf, "/", BUFSIZE);
        // Add the modified input file name and output file name with the
        // aliases replaced accordingly to the full input file path and full
        // output file path respectively
        strncat(full_inf, mod_inf, BUFSIZE);
        strncat(full_outf, mod_outf, BUFSIZE);

        //printf("\n-------------------------------------------------------\nInf: %s\nOutf: %s\n-------------------------------------------------------\n", full_inf, full_outf);

        // Need dummy_inf to tokenize
        strncpy(dummy_inf, full_inf, BUFSIZE);
        // Tokenize full input file path by "/"
        inf_tokens[0] = strtok(dummy_inf, "/");
        i = 1;
        while((inf_tokens[i] = strtok(NULL, "/")) != NULL) i++;
        k = 0;
        // Search out "..", use it to remove the preceeding directory.
        // Do this for input and output file paths. We will then search for the
        // document root and output area in the final result. If they are not
        // found, we have determined an attempt to get out of the document root
        // or output file
        for(j = 0; j < i; j++) {
            if(!strcmp(inf_tokens[j], "..") && k!=0) k--;
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

        strncpy(dummy_outf, full_outf, BUFSIZE);
        outf_tokens[0] = strtok(dummy_outf, "/");
        i = 1;
        while((outf_tokens[i] = strtok(NULL, "/")) != NULL) i++;
        k = 0;
        for(j = 0; j < i; j++) {
            if(!strcmp(outf_tokens[j], "..") && k!=0) k--;
            else {
              mod_outf_tokens[k] = outf_tokens[j];
              k++;
            }
        }
        for(j = 0; j < k; j++) {
            if(j==0) strncpy(dum_outf, "/", BUFSIZE);
            strncat(dum_outf, mod_outf_tokens[j], BUFSIZE);
            strncat(dum_outf, "/", BUFSIZE);
        }

        if(strstr(dum_inf, doc_root) == NULL) {
          fprintf(stderr, "Error: you are trying to get out of document root! Stop that!\n");
          cont = 0;
        }
        if(strstr(dum_outf, out_area) == NULL) {
          fprintf(stderr, "Error: you are trying to get out of output area! Stop that!\n");
          cont = 0;
        }
        // If we pass all these checks and the input file exists, execute the
        // copy!!
        if( cont && access( full_inf, R_OK ) != -1 ) {
            // file exists
            FILE *fpi, *fpo;
            fpi = fopen(full_inf, "r");
            if(fpi == NULL) {
                fprintf(stderr, "Error: could not open input file\n");
                cont = 0;
            }
            fpo = fopen(full_outf, "w");
            if(fpo == NULL) {
                fprintf(stderr, "Error: could not open output file\n");
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
        else if(cont) fprintf(stderr, "Error: given input file doesn't exist\n");
    }
    printf("\n\n");
    // Free all the aliases
    cleanAlias();
    return OK;
}
