#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    int i, j = 0;
    while (*(user_str + i) != '\0' && j < len) {
        if (*(user_str + i) != ' ' && *(user_str + i) != '\t') {
            *(buff + j) = *(user_str + i);
            ++j;
        }
        else if (*(user_str + i) == ' ' ||  *(user_str + i) == '\t') {
            if (j > 0 && *(buff + j - 1) != ' '){
                *(buff + j) = ' ';
                ++j;
            } 
        }
        ++i;
    }

    if (j > len) {
        return -1;
    }

    for(int k = j; k < len; k++){
        *(buff + k) = '.';
    }

    return j; //for now just so the code compiles. 
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //Word Start is a boolean, but technically boolean isn't included in the libraries included at the top of the document
    // I'd rather not lose points, so we're treating '0' as false, '1' as true.
    int word_start;
    int wc;

    word_start = 0;
    wc = 0;
    for(int i = 0; i<len; i++){
        char c_char = *(buff + i);
        if (word_start == 0){
            if (c_char == ' ' || c_char == '.'){
                continue;
            }
            else {
                wc++;
                word_start = 1;
            }
        }
        else{
            if (c_char == ' '){
                word_start = 0;
            }
            else{
                continue;
            }
        }
    }
    //YOU MUST IMPLEMENT
    return wc;
}

void reverse_string(char *buff, int len){
    int end_idx = len - 1;
    int start_idx = 0;
    char temp_char;
    while (end_idx > start_idx){
        temp_char = *(buff +start_idx);
        *(buff + start_idx) = *(buff + end_idx);
        *(buff + end_idx) = temp_char;
        start_idx++;
        end_idx--;
    }
}

void  word_print(char *buff, int len){
    //suggested local variables
    int last_char_idx;  //index of last char - strlen(str)-1;
    int wc = 0;         //counts words
    int wlen = 0;       //length of current word
    int word_start = 0;    //am I at the start of a new word
    last_char_idx = len - 1;
    // Please implement
    word_start = 0;

    for (int i = 0; i < len; i++){
        char c_char = *(buff + i);
        if (word_start == 0) {
            if (c_char == ' '){
                printf(" (%d)\n", wlen);
                word_start = 0;
                wlen = 0;
            }
            else {
                wc++;
                word_start = 1;
                //wlen = 0;
                wlen++;
                printf("%d. ", wc);
                printf("%c",c_char);
            }
        }
        else{
            if (c_char != ' '){
                printf("%c",c_char);
                wlen++;
            }
            else{
               printf(" (%d)\n", wlen); 
               word_start = 0; 
               wlen = 0;
            }
        }
        if (i == last_char_idx && wlen > 0){
               printf(" (%d)\n", wlen);
               word_start = 0;
               wlen = 0;
            }
        
    }
    return;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    *buff = malloc(BUFFER_SZ);

    if (*buff == NULL){
        exit(99);
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_string(buff,user_str_len);
            break;

        case 'w':
            printf("Word Print\n----------\n");
            word_print(buff,user_str_len);
            break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE