/*
 **********************************
 * C memon interpreter for memon  *
 *   files produced by F.E.I.S    *
 *        By fishguy6564          *
 **********************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memon.h"

MemonInfo memon_info;
Note currNote;
u32 counter = 0;
u32 lineNumber = 0;
MemonToken next_token;
FILE* file;
u32 recursionDepth = 0;

/*
 ********************************************************
 * Better to have keywords in heap than in stack since  *
 * we are using recursion that can potentially get deep.*
 ********************************************************
 */
char keywordList[14][20] = {
    "data", //0
    "BSC", //1
    "ADV", //2
    "EXT", //3
    "level", //4
    "notes", //5
    "resolution", //6
    "metadata", //7
    "BPM", //8
    "offset", //9
    "l", //10
    "n", //11
    "p", //12
    "t" //13
};

void initializeMemon(){
    memon_info.notes = (Note*)malloc(sizeof(Note));
    memon_info.noteCount = 0;
}

void appendNote(){
    Note newNote;
    memcpy(&newNote, &currNote, sizeof(Note));
    memon_info.notes = (Note*)realloc(memon_info.notes, memon_info.noteCount * sizeof(Note));
    memon_info.notes[memon_info.noteCount - 1] = newNote;
}

int isEmptySpace(char c){
    return c == ' ' || c == '\t';
}

MemonToken tokenScanner(){
    int c;
    MemonToken tok;
    c = fgetc(file);

    if(c == EOF) return SCANEOF;

    switch(c){
        case '{':
            tok = LCURL;
            break;
        case '}':
            tok = RCURL;
            break;
        case '"':
            tok = QUOTE;
            break;
        case ':':
            tok = COLON;
            break;
        case ',':
            tok = COMMA;
            break;
        case '[':
            tok = LBRACKET;
            break;
        case ']':
            tok = RBRACKET;
            break;
        case '\n':
            lineNumber++;
            tok = NEWLINE;
            break;
        default:
            if(isEmptySpace(c)) tok = BLANK;
            else{
                tok = NONE;
                ungetc(c, file);
            }
    }
    return tok;
}

void memonError(){
    printf("There is an issue with this memon. Exiting program\n");
    exit(0);
}

void match(MemonToken expected){
    while(next_token == BLANK || next_token == NEWLINE) next_token = tokenScanner();

    if(next_token != expected) memonError();

    next_token = tokenScanner();
    while(next_token == BLANK || next_token == NEWLINE) next_token = tokenScanner();

}

void getString(char* str){
    char c;
    while(1){
        c = getc(file);
        if(c == '"') break;
        strncat(str, &c, 1);
    }

    next_token = tokenScanner();
}

u32 getValue(){
    char* str = malloc(50);
    char c;
    u32 value;

    memset(str, 0, 50);
    while(1){
        c = getc(file);
        if(c == '\n') break;
        strncat(str, &c, 1);
    }

    value = atoi(str);
    free(str);
    return value;
}

float getFloatValue(){
    char* str = malloc(50);
    char c;
    float value;

    memset(str, 0, 50);
    while(tokenScanner() != NEWLINE){
        c = getc(file);
        strncat(str, &c, 1);
    }

    value = atof(str);
    free(str);
    return value;
}

void noteScanner(){
    char* str = malloc(50);


    memset(str, 0, 50);
    getString(str);

    if(!strcmp(str, keywordList[10])){
        match(COLON);
        currNote.noteLength = getValue();
    }
    else if(!strcmp(str, keywordList[11])){
        match(COLON);
        currNote.panel = getValue();
    }
    else if(!strcmp(str, keywordList[12])){
        match(COLON);
        currNote.tail = getValue();
    }
    else if(!strcmp(str, keywordList[13])){
        match(COLON);
        currNote.timing = getValue();
    }
    free(str);

}

void addNotes(){
    while(1){
        match(LCURL);

        if(next_token == QUOTE) noteScanner();

        while(next_token != QUOTE) next_token = tokenScanner();
        noteScanner();

        while(next_token != QUOTE) next_token = tokenScanner();
        noteScanner();

        while(next_token != QUOTE) next_token = tokenScanner();
        noteScanner();

        next_token = tokenScanner();
        match(RCURL);

        memon_info.noteCount++;
        appendNote();

        if(next_token == RBRACKET) break;

        next_token = tokenScanner();
    }
}

void statement(){
    char* str = malloc(100);
    char c;

    memset(str, 0, 100);

    getString(str);

    if(!strcmp(str, keywordList[0])){
        match(COLON);
        match(LCURL);
        statement_list();
    }
    else if(!strcmp(str, keywordList[1])){
        strcpy(memon_info.difficulty, keywordList[1]);
        match(COLON);
        match(LCURL);
        statement_list();
    }
    else if(!strcmp(str, keywordList[2])){
        strcpy(memon_info.difficulty, keywordList[2]);
        match(COLON);
        match(LCURL);
        statement_list();
    }
    else if(!strcmp(str, keywordList[3])){
        strcpy(memon_info.difficulty, keywordList[3]);
        match(COLON);
        match(LCURL);
        statement_list();
    }
    else if(!strcmp(str, keywordList[4])){
        match(COLON);
        memon_info.level = getValue();
        statement_list();
    }
    else if(!strcmp(str, keywordList[5])){
        match(COLON);
        match(LBRACKET);
        addNotes();
        next_token = tokenScanner();
        //match(COMMA);
    }

    else if(!strcmp(str, keywordList[6])){
        match(COLON);
        memon_info.resolution = getValue();
    }
    else if(!strcmp(str, keywordList[7])){
        match(COLON);
        match(LCURL);
        statement_list();
        //match(COMMA);
    }
    else if(!strcmp(str, keywordList[8])){
        match(COLON);
        memon_info.info.BPM = getFloatValue();
    }
    else if(!strcmp(str, keywordList[9])){
        match(COLON);
        memon_info.info.offset = getFloatValue();
    }
    free(str);
}


void statement_list(){
    while(1){
        if(next_token == QUOTE) statement();

        else if(next_token == RCURL){
            next_token = tokenScanner();
            break;
        }else if(next_token == SCANEOF){
            break;
        }
        next_token = tokenScanner();
    }
}

void memon(){
    match(LCURL);
    statement_list();
}

void parser(){
    next_token = tokenScanner();
    memon();
    match(SCANEOF);
}

void displayMemonInfo(){
    printf("Memon info:\n");
    printf("Chart difficulty: %s\n", memon_info.difficulty);
    printf("Level: %d\n", memon_info.level);
    printf("Resolution %d\n", memon_info.resolution);
    printf("BPM: %f\n", memon_info.info.BPM);
    printf("Offset: %f\n", memon_info.info.offset);
    printf("Total number of notes: %d\n", memon_info.noteCount - 1);
}

MemonInfo interpretMemonInfo(char* filename){

    initializeMemon(memon);

    file = fopen(filename, "r");

    if (file == NULL){
        printf("File does not exist\n");
        exit(0);
    }

    parser();

    fclose(file);

    return memon_info;
}
