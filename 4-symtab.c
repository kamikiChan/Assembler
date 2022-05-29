#include "3-asm_pass1_u.c"
#include <stdlib.h>

typedef struct 
{
    char    symbol[LEN_SYMBOL];
    int     PC;
} SYMBOL_PATH;

SYMBOL_PATH symtab[10000];

SYMBOL_PATH *is_symtab(char *symbol, int len)
{
    int tmep;
    for(int i = 0; i < len; i++)
    {
        if(strcmp(symtab[i].symbol, symbol) == 0) return symtab+i;
    }
    return NULL;
}

int *buil_symtab(LINE *line)
{
    int c, PC, len = 0;
    static int data[2];

    c = process_line(line);
    if(strcmp(line->op, "START") == 0)
    {
        sscanf(line->operand1, "%x", &PC);
        printf("%06X\t%s\t%s\t%s\n", PC, line->symbol, line->op
                        , line->operand1);
    }
    else PC = 0;
    while (strcmp(line->op, "END") != 0 && c != LINE_EOF)
    {   
        c = process_line(line);
        if(c == LINE_COMMENT) continue;
        printf("%06X\t%s", PC, line->symbol);
        if(line->fmt == 8)
            printf("\t+%s", line->op);
        else
            printf("\t%s", line->op);
        switch (line->addressing)
        {
            case 1:
                printf("\t%s", line->operand1);
                break;
            case 2:
                printf("\t#%s", line->operand1);
                break;
            case 4:
                printf("\t@%s", line->operand1);
                break;
            case 8:
            case 9:
                printf("\t%s,X", line->operand1);
                break;
        }
        if(line->operand2[0] != '\0')
            printf(",%s\n", line->operand2);
        else
            printf("\n");
        if(line->symbol[0] != '\0')
        {
            if(is_symtab(line->symbol, len) != NULL)
            {
                data[0] = -1;
                return data;
            }
            else 
            {
                symtab[len].PC = PC;
                strcpy(symtab[len++].symbol, line->symbol);
            }
        }
        if(line->op[0] != '\0')
        {
            int FMT = line->fmt;
            switch (FMT)
            {
                case 0:
                    if(strcmp(line->op, "BASE") == 0) continue;
                    else if(strcmp(line->op, "WORD") == 0) PC += 3;
                    else if(strcmp(line->op, "RESW") == 0) PC = PC + 3 * atoi(line->operand1);
                    else if(strcmp(line->op, "RESB") == 0) PC += atoi(line->operand1);
                    else if(strcmp(line->op, "BYTE") == 0)
                    {
                        char *p = strtok(line->operand1, "'");
                        if(p[0] == 'C')
                        {
                            p = strtok(NULL, " '");
                            PC += strlen(p);
                        }
                        else if(p[0] == 'X')
                        {
                            p = strtok(NULL, " '");
                            PC += strlen(p)/2;
                        }else{
                            data[0] = -2;
                            return data;
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
                    data[0] = -3;
                    return data;
                    break;
            }
        }
    }
    printf("Program length = %04X\n", PC);
    for(int i = 0; i < len; i++)
    {
        printf("%12s :\t%06X\n", symtab[i].symbol, symtab[i].PC);
    }
    data[0] = PC;
    data[1] = len;
    return data;
}


/*int main(int argc, char *argv[])
{
	int			i = 0, c, line_count = 1;
	char		buf[LEN_SYMBOL];
	LINE		line;

	if(argc < 2)
	{
		printf("Usage: %s fname.asm\n", argv[0]);
	}
	else
	{
        if(ASM_open(argv[1]) == NULL)
			printf("File not found!!\n");
		else
		{
            c = buil_symtab(&line);
			if(c == LINE_ERROR || c < 0)
            {
                if(c == -1)
				    printf("Symbol : Error\n");
                if(c == -2)
                    printf("BYTE Type : Error\n");
                if(c == -3)
                    printf("OP code : Error\n");
            }
		}
	}
}*/