#include "4-symtab.c"

#define N 0x02
#define I 0x01
#define X 0x08
#define B 0x04
#define P 0x02
#define E 0x01

int xpbe = 0;

int Register(char Rname)
{
    switch (Rname)
    {
        case 'A':
            return 0;
            break;
        case 'X':
            return 1;
            break;
        case 'L':
            return 2;
            break;
        case 'B':
            return 3;
            break;
        case 'S':
            return 4;
            break;
        case 'T':
            return 5;
            break;
        case 'F':
            return 6;
            break;
        default:
            return 0;
            break;
    }
}

int Adder(Instruction *op, LINE *line)
{
    switch (line->addressing)
    {
        case ADDR_SIMPLE:
            return op->code + N + I;
            break;
        case ADDR_IMMEDIATE:
            return op->code + I;
            break;
        case ADDR_INDIRECT:
            return op->code + N;
            break;
        case ADDR_INDEX:
            xpbe += X;
            return op->code;
            break;
        case ADDR_INDEX + ADDR_SIMPLE:
            xpbe += X;
            return op->code + N + I;
            break;
    }
}

void counting(char *name, int programLength, int symlen, LINE *line)
{
    const char* filename = name;
    SYMBOL_PATH *symtab;
    Instruction	*op;
    FILE* output_file = fopen(filename, "wb");
    char *p, str;
    
    int PC, Base = 0, x = 0, c, CTH, NPC, SPA, i = 0, next = 0, index = 0;
    char Header[7], Thex[7];
    char Hhex[12], len[2], hex[10], THex[61];
    THex[0] = '\0';

    c = process_line(line);
    if(strcmp(line->op, "START") == 0)
    {
        sscanf(line->operand1, "%x", &PC);
        sprintf(Header, "H%s\t", line->symbol);
        fwrite(Header, 1, strlen(Header), output_file);
        sprintf(Hhex, "%06X%06X\n", PC, programLength);
        fwrite(Hhex, 1, strlen(Hhex), output_file);
    }
    else 
    {
        PC = 0;
        sprintf(Header, "H%s\t", line->symbol);
        fwrite(Header, 1, strlen(Header), output_file);
        sprintf(Hhex, "%06X%06X\n", PC, programLength);
        fwrite(Hhex, 1, strlen(Hhex), output_file);
    }
    CTH = PC;
    SPA = PC;
    while (strcmp(line->op, "END") != 0 && c != LINE_EOF)
    {   
        c = process_line(line);
        if(c == LINE_COMMENT) continue;
        if(line->op[0] != '\0')
        {
            int FMT = line->fmt;
            switch (FMT)
            {
                case 0:
                    if(strcmp(line->op, "BASE") == 0) 
                    {
                        symtab = is_symtab(line->operand1, symlen);
                        Base = symtab->PC;
                        continue;
                    }
                    else if(strcmp(line->op, "WORD") == 0) PC += 3;
                    else if(strcmp(line->op, "RESW") == 0){
                        PC = PC + 3 * atoi(line->operand1);
                        next = 1;
                        NPC = PC;
                        continue;
                    } 
                    else if(strcmp(line->op, "RESB") == 0){
                        PC += atoi(line->operand1);
                        next = 1;
                        NPC = PC;
                        continue;
                    } 
                    else if(strcmp(line->op, "BYTE") == 0)
                    {
                        
                        p = strtok(line->operand1, "'");
                        if(p[0] == 'C')
                        {
                            str = p[0];
                            p = strtok(NULL, " '");
                            PC += strlen(p);
                        }
                        else if(p[0] == 'X')
                        {
                            str = p[0];
                            p = strtok(NULL, " '");
                            PC += strlen(p)/2;
                        }
                    }
                    break;
                case 1:
                    PC++;
                    break;
                case 2:
                    PC += 2;
                    break;
                case 4:
                    PC += 3;
                    break;
                case 8:
                    PC += 4;
                    break;
                default:
                    //return -3;
                    break;
            }
            int disp = 0;
            op = is_opcode(line->op);
            xpbe = 0;
            index = 0;
            hex[0] = '\0';
            symtab = NULL;
            switch (line->fmt)
            {
                case FMT0:
                    if(op->code == OP_WORD){
                        sscanf(line->operand1, "%x", &disp);
                        sprintf(hex, "%06X", disp);
                    }
                    else if(op->code == OP_BYTE){
                        if(str == 'C')
                        {
                            p = strrev(p);
                            sscanf(p, "%s", &disp);
                            sprintf(hex, "%X", disp);
                        }
                        else if(str == 'X')
                        {
                            sscanf(p, "%x", &disp);
                            sprintf(hex, "%02X", disp);
                        }
                    }
                    break;
                case FMT1:
                    sprintf(hex, "%02X", op->code);
                    break;
                case FMT2:
                    sprintf(hex, "%02X%X%X", op->code, Register(line->operand1[0]), Register(line->operand2[0]));
                    break;
                case FMT3:
                    sprintf(hex, "%02X", Adder(op, line));
                    index = 2;
                    break;
                case FMT4:
                    sprintf(hex, "%02X", Adder(op, line));
                    index = 2;
                    xpbe += E;
                    break;
            }
            if((symtab = is_symtab(line->operand1, symlen)) != NULL){
                disp = symtab->PC;
            } else {
                disp = atoi(line->operand1);
            }
            if((xpbe & E) == E && line->fmt > FMT2)
                sprintf(hex+index, "%X%05X", xpbe, disp);
            else if(line->fmt > FMT2){
                if((xpbe & X) == X && Base == 0){
                    disp -= x;
                    sprintf(hex+index, "%X%03X", xpbe, disp);
                }
                else if(line->operand1[0] == '\0' || symtab == NULL){
                     sprintf(hex+index, "%X%03X", xpbe, disp);
                }
                else if(disp - PC > 2047 || disp - PC < -2048)
                {
                    xpbe += B;
                    disp -= Base;
                    sprintf(hex+index, "%X%03X", xpbe, disp);
                }
                else if(disp - PC < 0)
                {
                    xpbe += P;
                    int d = PC - disp;
                    disp = 0xFFF ^ d - 1;
                    sprintf(hex+index, "%X%X", xpbe, disp);
                } else {
                    xpbe += P;
                    disp -= PC;
                    sprintf(hex+index, "%X%03X", xpbe, disp);
                }
            }
            if(strlen(THex) + strlen(hex) > 60 || next == 1){
                sprintf(Thex, "T%06X", CTH);
                fwrite(Thex, 1, strlen(Thex), output_file);
                sprintf(len, "%02X", strlen(THex)/2);
                fwrite(len, 1, strlen(len), output_file);
                strcat(THex, "\n");
                fwrite(THex, 1, strlen(THex), output_file);
                if(next == 1)
                    CTH = NPC;
                else
                    CTH += strlen(THex)/2;
                THex[0] = '\0';
                next = 0;
            }
            strcat(THex, hex);
        }
    }
    sprintf(Thex, "T%06X", CTH);
    fwrite(Thex, 1, strlen(Thex), output_file);
    sprintf(len, "%02X", strlen(THex)/2);
    fwrite(len, 1, strlen(len), output_file);
    strcat(THex, "\n");
    fwrite(THex, 1, strlen(THex), output_file);
    CTH += strlen(THex)/2;
    THex[0] = '\0';
    next = 0;
    sprintf(Thex, "E%06X", SPA);
    fwrite(Thex, 1, strlen(Thex), output_file);
    fclose(output_file);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int *data;
	int			i = 0, line_count = 1;
	char		buf[LEN_SYMBOL];
	LINE		line;

	if(argc < 3)
	{
		printf("Usage: %s fname.asm fname.obj\n", argv[0]);
	}
	else
	{
        if(ASM_open(argv[1]) == NULL)
			printf("File not found!!\n");
		else
		{
            data = buil_symtab(&line);
            ASM_close();
			if(data[0] == LINE_ERROR || data[0] < 0)
            {
                if(data[0] == -1)
				    printf("Symbol : Error\n");
                if(data[0] == -2)
                    printf("BYTE Type : Error\n");
                if(data[0] == -3)
                    printf("OP code : Error\n");
            }
            ASM_open(argv[1]);
            counting(argv[2], data[0], data[1], &line);
			ASM_close();
		}
	}
}