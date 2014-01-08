#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WORD_MAXIMUM_LENGTH 45    // assume pneumonoultramicroscopicsilicovolcanoconiosis as the longest word.

// prototype declarelation
void trim_nonalnum(char *string);
int strcmpcs(char ch1, char ch2, int flag);

int main(int argc,char *argv[])
{
    char string[WORD_MAXIMUM_LENGTH];
    char word[WORD_MAXIMUM_LENGTH];
    const char *root_path = getenv("USERPROFILE");
    char dict_path[255];
    char alternate_dict_path[255];
    char *file_path;
    char option;
    char termchar;
    const char *help = "Usage:\n"
                       " look [options] string [file]\n"
                       "\n"
                       "Options:\n"
                       " -a        use alternative dictionary\n"
                       " -d        compare only alphanumeric characters\n"
                       " -f        ignore case differences when comparing\n"
                       " -t <char> define string termination character\n"
                       " -h        display this help and exit\n";
    FILE *file;
    int flag_a = 0;
    int flag_d = 0;
    int flag_f = 0;
    int argn =  1;
    int i;
    int flag_error = 0;

    snprintf(dict_path, sizeof(dict_path), "%s\\Documents\\dict\\words\0", root_path);
    snprintf(alternate_dict_path, sizeof(alternate_dict_path), "%s\\Documents\\dict\\web2\0", root_path);
    file_path = dict_path;
    string[0] = '\0';
    termchar = '\0';

    // reading command line options
    opterr = 0;
    while((option=getopt(argc, argv, ":adft:h")) != -1){
        switch(option){
            case 'a': flag_a = 1;
                      argn++;
                      break;
            case 'd': flag_d = 1;
                      argn++;
                      break;
            case 'f': flag_f = 1;
                      argn++;
                      break;
            case 't': termchar = optarg[0];
                      argn += 2;
                      break;
            case 'h': fprintf(stdout, "\n%s", help);
                      return 0;
            case ':': fprintf(stderr, "%s: option requires an argument -- \'%c\'\n\n%s", argv[0], optopt, help);
                      return 1;
            case '?': fprintf(stderr, "%s: invalid option -- \'%c\'\n\n%s", argv[0], optopt, help);
                      return 2;
        }
    }

    // determinations of string and file_path.
    if (argc - argn > 1){
        // file is assigned in command line option.
        strcpy(string, argv[argn++]);
        file_path = argv[argn];
    }
    else if (argc - argn > 0){
        // file is not assigned in command line option.
        // if file is not assigned in command line option, option d and f is on by default.
        flag_d = 1;
        flag_f = 1;

        strcpy(string, argv[argn]);
        if (flag_a == 1){
            file_path = alternate_dict_path;
        }
    }

    // modification of string
    for (i = 0;i<WORD_MAXIMUM_LENGTH+1;i++){
        if (string[i] == '\0'){
            break;
        }
        else if (i == WORD_MAXIMUM_LENGTH){
            string[i] = '\0';
        }
    }
    if (flag_d == 1){
        trim_nonalnum(string);
    }

    // searching and printing matched items.
    if (strcmp(string, "")){
        if ((file = fopen(file_path, "r")) == NULL){
            // can not find words dictionary.
            fprintf(stderr, "%s: %s: No such file or directory", argv[0], file_path);
            flag_error = 3;
        }
        else{
            while(fgets(word, WORD_MAXIMUM_LENGTH, file)){
                if (flag_d == 1){ trim_nonalnum(string); }
                for (i = 0;i<WORD_MAXIMUM_LENGTH+1;i++){
                    if (string[i] == termchar){
                        if (termchar == '\0' || strcmpcs(string[i], word[i], flag_f)){
                            // matched
                            fprintf(stdout, "%s", word);
                            flag_error = 0;
                            break;
                        }
                        else{
                            // not matched
                            break;
                        }
                    }
                    else if (!strcmpcs(string[i], word[i], flag_f)){
                        // not matched
                        break;
                    }
                }
            }
        }
        fclose(file);
    }

    return flag_error;
}

void trim_nonalnum(char *string){
    char *string_copied;

    string_copied = (char *)malloc(sizeof(char) * (WORD_MAXIMUM_LENGTH+1));
    strcpy(string_copied, string);
    while (*string_copied != '\0'){
        if (isalnum(*string_copied)){
            *string = *string_copied;
            string++;
            string_copied++;
        }
        else{
            string_copied++;
        }
    }
    *string = '\0';
}

// comparing character in case-sensitive.
int strcmpcs(char ch1, char ch2, int flag){
    int res = 0;

    if (flag){
        if (toupper(ch1) == toupper(ch2)){
            res = 1;
        }
    }
    else{
        if (ch1 == ch2){
            res = 1;
        }
    }
    return res;
}
