#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WORD_MAXIMUM_LENGTH 255
#define LOAD_AT_ONCE 1000

// prototype declarelation
void trim_nonalnum(char *string);
int strcmpcs(char ch1, char ch2, int flag);
int front_match(char *string, char *word, char termchar, int flag);
int load_strings(FILE **fp, char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH]);
int search_start(char *string, char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH], char termchar, int flag);
int search_end(char *string, char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH], char termchar, int flag, int head);

int main(int argc,char *argv[]) {
    char string[WORD_MAXIMUM_LENGTH];
    char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH];
    const char *root_path = getenv("USERPROFILE");
    char dict_path[255];
    char alternate_dict_path[255];
    char *file_path;
    char option;
    char termchar;
    const char help[] = "Usage:\n"
                       " look [options] string [file]\n"
                       "\n"
                       "Options:\n"
                       " -a        use alternative dictionary\n"
                       " -d        compare only alphanumeric characters\n"
                       " -f        ignore case differences when comparing\n"
                       " -t <char> define string termination character\n"
                       " -h        display this help and exit\n";
    FILE *fp;
    int flag_a = 0;
    int flag_d = 0;
    int flag_f = 0;
    int argn = 1;
    int i, j, n;
    int err;
    int flag_error = 0;

    snprintf(dict_path, sizeof(dict_path), "%s\\Documents\\dict\\words\0", root_path);
    snprintf(alternate_dict_path, sizeof(alternate_dict_path), "%s\\Documents\\dict\\web2\0", root_path);
    file_path = dict_path;
    string[0] = '\0';
    termchar = '\0';

    // reading command line options
    opterr = 0;
    while((option=getopt(argc, argv, ":adft:h")) != -1) {
        switch(option) {
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
    if (argc - argn > 1) {
        // file is assigned in command line option.
        strcpy(string, argv[argn++]);
        file_path = argv[argn];
    } else if (argc - argn > 0) {
        // file is not assigned in command line option.
        // if file is not assigned in command line option, option d and f is on by default.
        flag_d = 1;
        flag_f = 1;

        strcpy(string, argv[argn]);
        if (flag_a == 1) {
            file_path = alternate_dict_path;
        }
    }

    // modification of string
    if (flag_d == 1) {
        trim_nonalnum(string);
    }

    // searching and printing matched items.
    if (strncmp(string, "", WORD_MAXIMUM_LENGTH)) {
        if ((fp = fopen(file_path, "r")) == NULL) {
            // can not find words dictionary.
            fprintf(stderr, "%s: %s: No such file or directory\n", argv[0], file_path);
            flag_error = 3;
        } else {
            do {
                n = load_strings(&fp, words);
                i = search_start(string, words, termchar, flag_f);
                fprintf(stdout, "n = %d, i = %d\n", n, i);
                if (i < LOAD_AT_ONCE) {
                    break;
                }
            } while (n == LOAD_AT_ONCE);

            j = search_end(string, words, termchar, flag_f, i);
            for (; i < j; i++) {
                fprintf(stdout, "%s", words[i]);
            }

            if (j == LOAD_AT_ONCE) {
                do {
                    n = load_strings(&fp, words);
                    j = search_end(string, words, termchar, flag_f, 0);
                    if (j >= 0) {
                        for (i = 0; i < j; i++) {
                            fprintf(stdout, "%s", words[i]);
                        }
                    } else {
                        break;
                    }
                } while (n == LOAD_AT_ONCE);
            }
        }
        fclose(fp);
    } else {
        fprintf(stdout, "\n%s", help);
    }

    return flag_error;
}

int load_strings(FILE **fp, char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH]) {
    int i;

    for (i = 0;i<LOAD_AT_ONCE;i++) {
        if (fgets(words[i], WORD_MAXIMUM_LENGTH, *fp) == NULL) {
            break;
        }
    }
    return i++;
}

int search_start(char *string,
                 char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH],
                 char termchar,
                 int flag) {
    int head = 0;
    int tail = LOAD_AT_ONCE - 1;
    int needle = (LOAD_AT_ONCE - 1)/2;
    int idx;
    int is_head_matched;
    int is_tail_matched;
    int is_needle_matched;

    is_tail_matched = front_match(string, words[tail], termchar, flag);
    if (is_tail_matched > 0) {
        idx = LOAD_AT_ONCE;
    } else {
        is_head_matched = front_match(string, words[head], termchar, flag);
        if (is_head_matched == 0) {
            idx = 0;
        } else {
            while(1) {
                is_needle_matched = front_match(string, words[needle], termchar, flag);
                if (tail - head <= 1) {
                    if (is_needle_matched == 0) {
                        idx = head;
                        break;
                    } else {
                        idx = tail;
                        break;
                    }
                } else {
                    if (is_needle_matched <= 0) {
                        tail = needle;
                    } else {
                        head = needle;
                    }
                }
                needle = (head + tail)/2;
            }
        }
    }
    return idx;
}

int search_end(char *string,
               char words[LOAD_AT_ONCE][WORD_MAXIMUM_LENGTH],
               char termchar,
               int flag,
               int head) {
    int tail = LOAD_AT_ONCE - 1;
    int needle = (head + tail)/2;
    int idx;
    int is_head_matched;
    int is_tail_matched;
    int is_needle_matched;

    is_tail_matched = front_match(string, words[tail], termchar, flag);
    if (is_tail_matched == 0) {
        idx = LOAD_AT_ONCE;
    } else {
        is_head_matched = front_match(string, words[head], termchar, flag);
        if (is_head_matched >= 0) {
            while(1) {
                is_needle_matched = front_match(string, words[needle], termchar, flag);
                if (tail - head <= 1) {
                    if (is_needle_matched == 0) {
                        idx = tail;
                        break;
                    } else {
                        idx = head;
                        break;
                    }
                } else {
                    if (is_needle_matched == 0) {
                        head = needle;
                    } else {
                        tail = needle;
                    }
                }
                needle = (head + tail)/2;
            }
        } else {
            idx = 0;
        }
    }
    return idx;
}

void trim_nonalnum(char *string) {
    char *string_copied;

    string_copied = (char *)malloc(sizeof(char) * (WORD_MAXIMUM_LENGTH+1));
    strcpy(string_copied, string);
    while (*string_copied != '\0') {
        if (isalnum(*string_copied)) {
            *string = *string_copied;
            string++;
            string_copied++;
        } else {
            string_copied++;
        }
    }
    *string = '\0';
}

// comparing character in case-sensitive.
int strcmpcs(char ch1, char ch2, int flag) {
    if (flag) {
        return toupper(ch1) - toupper(ch2);
    } else {
        return ch1 - ch2;
    }
}

int front_match(char *string, char *word, char termchar, int flag) {
    int res;

    while(*string != '\0') {
        res = strcmpcs(*string, *word, flag);
        if (*string == termchar) {
            if (!res) {
                // matched
                break;
            } else {
                // not matched
                return res;
            }
        } else if (res) {
            // not matched
            return res;
        }

        string++;
        word++;
    }
    return 0;
}
