typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef struct{
    u32 noteLength; //l
    u32 panel; //n
    u32 tail; //p
    u32 timing; //t
}Note;

typedef struct{
    float BPM;
    float offset;
}Metadata;

typedef struct{
    char difficulty[4];
    u32 level;
    u32 resolution;
    u32 noteCount;
    Metadata info;
    Note* notes;
}MemonInfo;

typedef enum{
    LCURL, //0
    RCURL, //1
    COLON, //2
    QUOTE, //3
    COMMA, //4
    LBRACKET, //5
    RBRACKET, //6
    BLANK, //7
    NEWLINE, //8
    NONE, //9
    SCANEOF
}MemonToken;

void noteStatement();
void noteScanner();
float getFloatValue();
u32 getValue();
void getString(char* str);
void match(MemonToken expected);
void memonError();
MemonToken tokenScanner();
int isEmptySpace(char c);
void appendNote();
void copyNote(Note currNote, Note newNote);
void initializeMemon();
void addNotes();
void statement();
void statement_list();
void memon();
void parser();
void displayMemonInfo();
MemonInfo interpretMemonInfo(char* filename);
