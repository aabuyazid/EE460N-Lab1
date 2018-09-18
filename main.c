/*
	Name 1:
	Name 2 :
	UTEID 1:
	UTEID 2:
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
FILE* infile = NULL;
FILE* outfile = NULL;

enum
{
    DONE, OK, EMPTY_LINE
};

/* Creates Symbol table entry*/
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];	/* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

/* generates symbol table */
void func(void);
/* Adds an element to the symbol table and checks if it is already there*/
int makeSymbolTable(char* pLabel, int memLocation, int currentStore);

/* Checks if label is anything it should not be*/
int validLabel(char* pLabel);

/* generates 1's and 0's */
void func2(void);

int readAndParse(FILE* pInfile, char* pLine, char** pLabel,  char** pOpcode, char** pArg1, char** pArg2, char** pArg3, char** pArg4);

int isOpcode(char* input);

/* looks at if input is .end*/
int isEnd(char* input);

/* Looks at if input is .orig*/
int isFront(char* input);

/* checks the value depending on operation, .Orig is 29 and .fill is 30*/
int isOrig(char* input, int operation);

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");


    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    int lInstr = 0;
    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);


    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    /* Do stuff with files */
    func();
    rewind(infile);
    func2();
    FILE * pOutfile;
    pOutfile = fopen( "data.out", "w" );


    fprintf( pOutfile, "0x%.4X\n", lInstr );	/* where lInstr is declared as an int */


    fclose(infile);
    fclose(outfile);



    return 0;
}

int
toNum( char * pStr )
{
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if( *pStr == '#' )				/* decimal */
    {
        pStr++;
        if( *pStr == '-' )				/* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    }
    else if( *pStr == 'x' )	/* hex     */
    {
        pStr++;
        if( *pStr == '-' )				/* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
        if( lNeg )
            lNum = -lNum;
        return lNum;
    }
    else
    {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4){
    char * lRet, * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
        return( DONE );
    for( i = 0; i < strlen( pLine ); i++ )
        pLine[i] = tolower( pLine[i] );

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while( *lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n' )
        lPtr++;

    *lPtr = '\0';
    if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
        return( EMPTY_LINE );

    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    }

    *pOpcode = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg1 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg3 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg4 = lPtr;

    return( OK );
}

/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */

void func(void) {
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;

    //FILE * lInfile;

    //lInfile = fopen( "data.in", "r" );	/* open the input file */
/* checks the first line to see if it is .orig */
    lRet = readAndParse(infile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
    if(*lLabel != '\0'){
        exit(4);
    }
    int front = isFront(lOpcode);
    if(front == -1)
        exit (4);
    int invalidOrig = isOrig(lArg1, 29);
   // if(invalidOrig == -1)
     //   exit (3);
    if(*lArg2 != '\0')
        exit(4);

    int label = 0;
    do
    {
        lRet = readAndParse( infile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            if(*lLabel != '\0' ){
                int labelVal = validLabel(lLabel);
                label = makeSymbolTable(lLabel, invalidOrig, label);
            }
            //Check for case when .orig is xFFFF or close to that
            invalidOrig += 2;
        }
    } while( lRet != DONE );
}


void func2(void) {
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;

    do
    {
        lRet = readAndParse( infile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {

        }
    } while( lRet != DONE );
}
/* returns count if opcode match and returns -1 if no opcode match
 * uses double char array to store opcodes and compares the strings until
 * both reach NULL
 * count says which opcode out of the 28 it is
 * */
int isOpcode(char* input){
    char ops[28][6] ={ "add", "and","br","brn","brz","brp","brnz",
                       "brnp","brzp","brnzp","jmp","jsr","jsrr","ldb",
                       "ldw","lea" ,"rti" , "ret","lshf","rshfl", "rshfa",
                       "stb","stw","trap","xor","halt","nop","not"
    };
    int result = 0;
/*
 *	char* and = "and";
	char* add = "add";
	char* br = "br";
	char* halt = "halt";
	char* jmp = "jmp";
	char* jsr = "jsr";
	char* jsrr = "jsrr";
	char* ldb = "ldb";
	char* ldw = "ldw";
	char* lea = "lea";
	char* nop = "nop";
	char* not  "not";
	char* not = "ret";
	char* lshf = "lshf";
	char* rshfl = "rshfl";
	char* rshfa = "rshfa";
	char* rti = "rti";
	char* stb = "stb";
	char* stw = "stw";
	char* trap = "trap";
	char* xor = "xor";
	char* brnp = "brnp";
	char* brn = "brn";
	char* brnz = "brnz";
	char* brnzp = "brnzp";
	char* brz = "brz";
	char* brzp = "brzp";
	char* brp = "brp";
	char* temp = input;
 * */
    int count = 0;
    while(count < 28){
        result = strcmp(input, ops[count]);
            if(result == 0) return (count + 1);
        count++;
    }
    return -1;
}

/*
 * Checks  if the input is the end, 1 if it is, -1 if it is not
 * */
int isEnd(char* input){
    char* ending = ".end";
        int result = strcmp(input, ending);
        if(result == 0){
            return 1;
    }
return -1;
}

int isFront(char* input){
    char* ending = ".orig";
        int result = strcmp(input, ending);
        if(result == 0){
            return 1;
    }
return -1;
}

/*Checks for signed and unsigned maxes. If negative, it checks if bigger than 32768
 * for unsigned 65535
 * returns 1 if it is not bigger
 * */
int isOrig(char* input, int operation){
    if(*input == '\0') return -1;
    unsigned int maxVal = 65535;
    int lowerVal = 32768;
    int inputVal = 0;
    int result = toNum(input);
    if((operation == 1) || (operation == 2) || (operation == 25)){
        /* -16 to 15 range for 2^5  */
        if((result > 15) || (result < -16)){
            exit(3);
        }
        return 1;
    }
    if((operation == 19) || (operation == 20) || (operation == 21)){
     if((result < 0) || (result > 15)) exit(3);
     return 1;
    }
    if((operation == 29) || (operation == 30)){
    if((result < -32768) || (result > 65535)) exit(3);
    if(operation == 29) {
        if ((result % 2) != 0) exit(3);
    }
    return result;
    }
    if((operation < 0) || (operation > 30)){
        exit(2);
    }
    /* invalid immediate on opcode*/
    return -1; //if -1, there is an error, but I don't know what it is
}

/*
 * Checks if more than 255 labels
 * checks the length of label if greater than 20
 * makes copy of label and stores it (must we malloc?)
 * checks if already in symbol table
 * updates current stores
 * */
int makeSymbolTable(char* pLabel, int memLocation, int currentStore){
    if(currentStore >= MAX_SYMBOLS)
        exit(4);
    int length = strlen(pLabel);
    if(length > 20){
        exit(4);
    }
    char* str = (char*) malloc(21);
    strcpy(str, pLabel);
    TableEntry t1 = {memLocation ,*str};
    for(int i = 0; i < currentStore + 1; i++){
        TableEntry t2 = symbolTable[i];
        char* copy = t2.label;
        printf("%s\n",copy);
        if(strcmp(str, copy) == 0) exit(4);
        //appends the string below for 21
    }
    for(int j = 0; j < length; j++){
        t1.label[j] = str[j];
    }
    t1.label[length] = '\0';
    symbolTable[currentStore] = t1;
    return (currentStore + 1);
}

/* returns 1 if a valid label*/
int validLabel(char* pLabel){
    char notLabels [4][5] = {"in", "out", "getc", "puts"}; //can it not be halt or so either
    char ops[28][6] ={ "add", "and","br","brn","brz","brp","brnz",
                       "brnp","brzp","brnzp","jmp","jsr","jsrr","ldb",
                       "ldw","lea" ,"rti" , "ret","lshf","rshfl", "rshfa",
                       "stb","stw","trap","xor","halt","nop","not"
    };
    for(int i = 0; i < 5; i++){
        if(strcmp(pLabel, notLabels[i]) == 0) exit(4);
    }
    for(int i = 0; i < 28; i++){
        if(strcmp(pLabel, ops[i]) == 0) exit(4);
    }
    char c = pLabel[0];
    if(c == 'x') exit(4);
    //assumes it has null on end
    for(int i = 0; i < strlen(pLabel); i++){
        c = pLabel[i];
        if(isalnum(c) == 0) exit(4);
    }
    return 1;
}

/* returns 1 if the label was in the symbol table
 * checks the argument for the second one on BR, LEA, JSR, JSRR
 * exits 1 if the label is not in the symbol table
 * */
int doesLabelExist(char* pLabel){
    for(int i = 0; i < 255; i++){
        TableEntry t1 = symbolTable[i];
        if(strcmp(pLabel, t1.label)){
            return 1;
        }
    }
    exit(1);
}