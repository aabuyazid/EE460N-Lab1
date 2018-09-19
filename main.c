/*
       Name 1:
       Name 2:
       UTEID 1:
       UTEID 2:
 */
#include <stdio.h>
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

FILE* infile = NULL;
FILE* outfile = NULL;

#define MAX_LINE_LENGTH 255
enum
{
    DONE, OK, EMPTY_LINE
};
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];	/* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

/* generates symbol table */
void func(void);

int makeSymbolTable(char* pLabel, int memLocation, int currentStore);


//int
validLabel(char* pLabel);
/* generates 1's and 0's */
void func2(void);

int readAndParse(FILE* pInfile, char* pLine, char** pLabel,  char** pOpcode, char** pArg1, char** pArg2, char** pArg3, char** pArg4);

int isOpcode(char* input);

int isEnd(char* input);

int isFront(char* input);

/* checks the value depending on operation, .Orig is 29 and .fill is 30*/
int isOrig(char* input, int operation);

void decToHex(int value, char* pArg1);
//Gets the top four bits
char opToHex(int input);
//executes code based on op and if op is there
void checkTwelveBits( char* pArg1, char* pArg2,char* pArg3, char* pArg4, char* psecondHexVal, char* pthirdHexVal, char* pfourthHexVal, int input);

//Checks if the values are allowed for add, and, xor, not
void checkForPlacement(char* pArg1, char* pArg2, char* pArg3);

//Does add, and, xor, not
int constructAdd(char* pArg1, char* pArg2, char* pArg3);

int checkLDBPlacement(char* pArg1, char* pArg2, char* pArg3);

int trapPlacement(char* pArg1);

int jsrJmpPlacement(char* pArg1);

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


int toNum( char * pStr ) {
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


int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4)
{
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
    //check proper way of doing .orig to exit 3 when it is not word aligned or if too large
    if(front == -1)
        exit (4);
    //Program Counter
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
                //int label
                validLabel(lLabel);
                //Symbol table index
                label = makeSymbolTable(lLabel, invalidOrig, label);
            }
            //Check for case when .orig is xFFFF or close to that
            invalidOrig += 2;
        }
    } while( lRet != DONE );
}

/* When is pArg4 not zero?*/
void func2(void) {
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;
    lRet = readAndParse(infile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
    int invalidOrig = isOrig(lArg1, 29); //Get value of .orig, should be valid  because of func
    decToHex(invalidOrig, lArg1);
    char secondHexVal = '\0';
    char thirdHexVal = '\0';
    char fourthHexVal = '\0';
    do
    {
        lRet = readAndParse( infile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            //Line below just shows what Arg4 is if not null
            if(*lArg4 != '\0') printf("%s", lArg4);
            char hexVal[7] = {'0', 'x', '\0', '\0', '\0', '\0', '\0'}; //\n
            int checked = isOpcode(lOpcode);
            if(checked == -1) exit(2);
            hexVal[3] = opToHex(checked);
            checkTwelveBits(lArg1, lArg2, lArg3, lArg4, &secondHexVal, &thirdHexVal, &fourthHexVal, checked);
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
validLabel(char* pLabel){
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
    //return 1;
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

void decToHex(int value, char* pArg1){
            fprintf(outfile, "0x%.4X\n",value);
}

char opToHex(int input){
    char opBinary [14] = {'1', '5', '0', 'C', '4', '2', '6', 'E', '8', 'D', '3', '7','F', '9'};
    if(input == 1) return opBinary[0];
    if(input == 2) return  opBinary[1];
    if((input >2) && (input <11)) return opBinary[2];
    if(input == 11) return opBinary[3];
    if((input == 12) ||(input == 13)) return opBinary[4];
    if(input == 14) return opBinary[5];
    if(input == 15) return opBinary[6];
    if(input == 16) return opBinary[7];
    if((input == 17) || (input == 18)) return opBinary[8];
    if((input == 19) || (input == 20) || (input == 21)) return opBinary[9];
    if(input == 22) return opBinary[10];
    if(input == 23) return opBinary[11];
    if(input == 24) return opBinary[12];
    if((input == 25) || (input == 28)) return opBinary[13];
    printf("Something went wrong %d\n", input);
    return '\0';
}

void checkTwelveBits( char* pArg1, char* pArg2,char* pArg3, char* pArg4, char* psecondHexVal, char* pthirdHexVal, char* pfourthHexVal, int input){
    int bottomFour = 0;
    int middleFour = 0;
    int topFour = 0;
    if((input == 1) || (input == 2) || (input == 25)|| (input == 28)){
        if((*pArg1 == '\0') || (*pArg2 == '\0') || (*pArg3 == '\0')) exit (4); //Wrong number of operands
        checkForPlacement(pArg1, pArg2, pArg3);
         int twelveBit = constructAdd(pArg1,pArg2,pArg3);
         //0x000F
         bottomFour = twelveBit & 15;
         middleFour = (twelveBit >> 4) & 15;
         topFour = (twelveBit >> 8) & 15;
         //Value of int should fit
         //Unless the values weren't checked for oversized
         *psecondHexVal = topFour + '0';
         *pthirdHexVal = middleFour + '0';
         *pfourthHexVal = bottomFour + '0';
         return;
    }
    if((input == 14) || (input == 15) || (input == 22)|| (input == 23)){
        if((*pArg1 == '\0') || (*pArg2 == '\0') || (*pArg3 == '\0')) exit (4); //Wrong number of operands
        int bits = checkLDBPlacement(pArg1, pArg2, pArg3);
        //0x000F
        bottomFour = bits & 15;
        middleFour = (bits >> 4) & 15;
        topFour = (bits >> 8) & 15;
        //Value of int should fit
        //Unless the values weren't checked for oversized
        *psecondHexVal = topFour + '0';
        *pthirdHexVal = middleFour + '0';
        *pfourthHexVal = bottomFour + '0';
        return;
    }
    if(((input > 2 ) && (input < 11)) || (input == 16)){
        //checks branch operands
        if(input != 16){
            if((*pArg1 == '\0') || (*pArg2 != '\0')) exit (4); //Wrong number of operands, either 0 or more than 1
        }
        //checks lea below for two operands
        else{
            if((*pArg1 == '\0') || (*pArg2 == '\0') || (*pArg3 != '\0')) exit (4); //not 2 operands exactly
        }
    }
    //Trap, jmp, jsr, jsrr, ret, ret
    if((input == 11)|| (input == 12) || (input == 13) || (input == 17) || (input == 18) || (input == 24)){
        //
        if((input > 10) && (input < 14)){
            if((*pArg1 == '\0') || (*pArg2 != '\0')) exit(4); //Wrong number of operands
        }
        //trap
        else if(input == 24){
            if((*pArg1 == '\0') || (*pArg2 != '\0')) exit(4); //Wrong number of operands
            int bitVals = trapPlacement(pArg1);
            psecondHexVal = '0';
            pthirdHexVal =  ((bitVals >> 4) & 15) + '0';
            pfourthHexVal = ((bitVals & 15) + '0');
        }
        else{
            if(*pArg1 != "\0") exit(4); //Wrong number of operands
        }
    }// this one may be wrong
    if((input > 18) && (input < 22)){
        if((*pArg1 == '\0') || (*pArg2 == '\0') || (*pArg3 == '\0')) exit (4); //Wrong number of operands
    }
    //How to check for halt and nop?
}

void checkForPlacement(char* pArg1, char* pArg2, char* pArg3){
    char registers[8][3] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int trueCheck = 0;
    //Checks DR
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg1, registers[k]);
        if(result == 0) trueCheck = 1;
    }
    //Checks SR1
    if(trueCheck == 0) exit(4);//It was not 0-7
    trueCheck = 0;
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg2, registers[k]);
        if(result == 0) trueCheck = 1;
    }
    //Checks SR2
    if(trueCheck == 0) exit(4);//It was not 0-7
    trueCheck = 0;
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg3, registers[k]);
        if(result == 0) trueCheck = 1;
    }
    //Checks if sr2 was actually a x or # value
    if(trueCheck == 0){
        if(*pArg3 == 'x' || *pArg3 == '#') trueCheck = 1;
        if(trueCheck == 0) exit(4);
    }//It was not 0-7
}

int constructAdd(char* pArg1, char* pArg2, char* pArg3){
    char registers[8][3] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int registerVal = 0;
    int destination = 0;
    int sourceOne = 0;
    int sourceTwo = 0;
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg1, registers[k]);
        if(result == 0) registerVal = k;
    }
    //Bit-Wise  shift it 9 to 11-9
    destination = registerVal << 9;
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg2, registers[k]);
        if(result == 0) registerVal = k;
    }
    //Bit-Wise shift by 6
    sourceOne = registerVal << 6;
    int immediateOrSource = 0;
    //Source indicated by one
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg3, registers[k]);
        if(result == 0) {
            registerVal = k;
            immediateOrSource = 1;
        }
    }
    if(immediateOrSource == 1){
        sourceTwo = registerVal;
    }
    else{
        sourceTwo = 32;
        int val = toNum(pArg3);
        //x000F
        val = (val & 31);
        sourceTwo = (sourceTwo | val);
    }
    //x0FFF
    int returnVal = 4095;
    returnVal = (destination | sourceOne | sourceTwo) & 4095;
    return returnVal;
}

int checkLDBPlacement(char* pArg1, char* pArg2, char* pArg3){

    char registers[8][3] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int trueCheck = 0;
    int registerVal = 0;
    int destination = 0;
    int sourceOne = 0;
    int sourceTwo = 0;
    //Checks DR
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg1, registers[k]);
        if(result == 0) {
            registerVal = k;
            trueCheck = 1;
        }
    }
    //Checks SR1
    if(trueCheck == 0) exit(4);//It was not 0-7
    destination = registerVal << 9;
    trueCheck = 0;
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg2, registers[k]);
        if(result == 0) {
            registerVal = k;
            trueCheck = 1;
        }
    }
    //Checks SR2
    if(trueCheck == 0) exit(4);//It was not 0-7
    sourceOne = registerVal <<6;
    trueCheck = 0;
    //Checks if sr2 was actually a x or # value
    if(trueCheck == 0){
        if(*pArg3 == 'x' || *pArg3 == '#'){
            trueCheck = 1;
        }
        if(trueCheck == 0) exit(4);
        sourceTwo = 0;
        //This may be wrong way of clipping value
        int val = toNum(pArg3);
        //x000F
        val = (val & 63);
        sourceTwo = (sourceTwo | val);
    }//It was not 0-7
    int returnVal = 4095;
    returnVal = (destination | sourceOne | sourceTwo) & 4095;
    return returnVal;
}

int trapPlacement(char* pArg1){
     int length = 8191;
     int val = toNum(pArg1);
    if(val < 0){
        printf("Trap Value %d", val);
        exit(3);
    }
     if(val > length) exit(3);
    return val;
}

//Checks for commas
int jsrJmpPlacement(char* pArg1){
    char registers[8][3] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int trueCheck = 0;
    int registerVal = 0;
    int destination = 0;
    for(int k = 0; k < 8; k++){
        int result = strcmp(pArg1, registers[k]);
        if(result == 0) {
            registerVal = k;
            trueCheck = 1;
        }
    }
    //Checks SR1
    if(trueCheck == 0) exit(4);//It was not 0-7
    destination = registerVal << 6;
    trueCheck = 0;
    destination = destination & 511;
    return destination;
}

