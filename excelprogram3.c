#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include <string.h>
#include <math.h>
#include <ctype.h> 

#define SIZESTRING  50
#define FLAG_NUMBER 0
#define FLAG_STORAGENUM 1
#define FLAG_MINUS 2
#define FLAG_EXIT 3
#define FLAG_ERORRDIVZERO 4
#define FLAG_WITHOUT 5
#define FLAG_ERORRVAL 6

#define STATUS_FORMULA_TODECIPHE 1
#define STATUS_FORMULA_DECODED 2
#define STATUS_STRING 3
#define STATUS_DIGIT 4
#define STATUS_DIGIT_MINUS 5
#define STATUS_EXCEEDING_CELL_SIZE_ERROR 12
#define STATUS_VALUE_ERROR 13
#define STATUS_ZERO_DIVISION_ERROR 14
#define CIRCULAR_REFERENCE_ERROR 15
/*CELL_STRUCTURE*/
typedef struct {
	int addressRow;
	int addressColumn;
	char str[SIZESTRING], * stp;
	int status;
	double num;
	int rid;
	int ls;
}cell;

int sizearr = 0;
cell* parr = NULL;

cell* CreatesCells(char* dir);
int cntArrCellSize(FILE* fcsv);
void adminCells();
void printCells(cell* p);
void printAddressColumn(int num);
void printAddressColumnRecur(int num, int size);
void setCells(cell* p, FILE* fcsv);
void partDigit(cell* p);
void assessmentStr(cell* p);
void classificationStr(cell* pn);
void whereReference(cell* pn, int q, int cr[2]);
double adminReference(cell* p, int q);
double calculatorCell(cell* p, int sit);
cell* findcell(int row, int column);
void freeall(cell* p);
void printStatusError(int status);
void changeStr(cell* p, int start, int colond);
void writrAddressColumnRecur(int num, int size, char* str);

int main(int argc, char* argv[])
{
	cell* p = 0;
	if (argc != 2)
	{
		printf("The amount of arguments is incorrect");
		return 0;
	}
	parr = p = CreatesCells(argv[1]);
	if (p == NULL)
		exit(1);
	adminCells();
	printCells(p);
	freeall(p);
	return 0;
}

/*Prints cells*/
void printCells(cell* p)
{
	for (int i = 0; i < sizearr; i++)
	{
		printf("cell %d\naddress ", i + 1);
		printAddressColumn(p[i].addressColumn);
		printf("%d\nstring is %s", p[i].addressRow + 1, p[i].stp);
		//printf("status %d", p[i].status);
		if (p[i].status < 10)
			printf("\nval = %g", p[i].num);
		else
			printStatusError(p[i].status);
		printf("\n-----------------------\n\n");
	}
}
/*Cell Printer Returns cell caption by number for prints*/
void printAddressColumn(int num)
{
	int i, size = 1;
	for (i = 26; num >= i; size++)
	{
		num -= i;
		i *= i;
	}
	printAddressColumnRecur(num, size);
}
/*Help print cell caption*/
void printAddressColumnRecur(int num, int size)
{
	int i = num % 26;
	if (--size)
		printAddressColumnRecur(num / 26, size);
	printf("%c", i + 'A');
}
/*Writes a reference string*/
void writrAddressColumnRecur(int num, int size, char* str)
{
	int i = num % 26;
	if (--size)
		writrAddressColumnRecur(num / 26, size, 0);
	int j = strlen(str);
	str[j] = i + 'A';
	str[j + 1] = 0;
}


/*Count how many cells are needed for the program*/
int cntArrCellSize(FILE* fcsv)
{
	
	char c, flag1 = 0, flag2 = 0;
	int cnt = 0;
	while ((c = fgetc(fcsv)) != EOF)
	{
		if (!flag1 && c != ',' && c != '\n')
		{
			cnt++;
			flag1 = 1;
		}
		if (c == '"')
			flag2 = !(flag2);
		if (flag1 && (c == ',' || c == '\n') && !flag2)
			flag1 = 0;
	}
	fseek(fcsv, 0L, SEEK_SET);
	return cnt;
}


/*Runs the entire file and creates cells*/
cell* CreatesCells(char* dir)
{
	FILE* fcsv = fopen(dir, "rt");
	if (fcsv == NULL)
	{
		printf("Error opening file");
		exit(1);
	}
	cell* p = 0;
	if ((sizearr = cntArrCellSize(fcsv)) == 0)
	{
		printf("The file is empty");
		return 0;
	}
	p = (cell*)malloc(sizeof(cell) * (sizearr));
	if (!p)
	{
		printf("Error There is not enough free memory");
		exit(1);
	}
	setCells(p, fcsv);
	fclose(fcsv);
	return p;
}
/*Puts the string in the cell and updates a row and column number and initializes values*/
void setCells(cell* p, FILE* fcsv)
{
	char str[SIZESTRING * 50], c, flag = 0;
	int i = 0, j = 0, row = 0, column = 0;
	while ((c = fgetc(fcsv)) != EOF)
	{
		if ((str[j++] = c) == '"')
			flag = !(flag);
		if ((!flag) && c == ',' || (c == '\n' || c == '\r'))
		{
			str[--j] = 0;
			if (j > 0)
			{
				if (j < SIZESTRING)
					p[i].stp = p[i].str;
				else
				{
					p[i].stp = (char*)malloc(sizeof(char) * (j + 1));
					if (!p[i].stp)
					{
						printf("erorr");
						exit(1);
					}
					p[i].str[0] = 0;
				}
				if (str[0] == '"')
				{
					str[--j] = 0;
					strcpy(p[i].stp, &(str[1]));
				}
				else
					strcpy(p[i].stp, str);
				p[i].addressColumn = column;
				p[i].addressRow = row;
				p[i].status = 0;
				p[i].rid = 0;
				i++;
				j = 0;
				str[0] = 0;
			}
			if (c == ',')
				column++;
			if (c == '\n')
			{
				column = 0;
				row++;
			}
		}
	}
}

/*Cell calculator*/
double calculatorCell(cell* p, int sit)
{
	double num = 0, div = 0;
	int flag = FLAG_NUMBER;
	if (!sit)
		p->ls = 1;
	while (flag == FLAG_NUMBER || flag == FLAG_MINUS || flag == FLAG_STORAGENUM)
	{
		if (p->status == STATUS_VALUE_ERROR)
			flag = FLAG_ERORRVAL;
		else if (p->status == STATUS_ZERO_DIVISION_ERROR)
			flag = FLAG_ERORRDIVZERO;
		if (flag == FLAG_NUMBER)
			flag = FLAG_WITHOUT;
		if (isdigit(p->stp[p->ls]) && flag != FLAG_STORAGENUM)
		{
			do
				num = num * 10 + p->stp[p->ls++] - '0';
			while (isdigit(p->stp[p->ls]));
			if (p->stp[p->ls] == '.')
			{
				int j = 1;
				while (isdigit(p->stp[++p->ls]))
					num += (p->stp[p->ls] - (double)'0') / pow(10, j++);
				if (j == 1)
					flag = FLAG_ERORRVAL;
			}
			if (flag == FLAG_MINUS)
				num *= -1;
			flag = FLAG_NUMBER;
		}
		if (isalpha(p->stp[p->ls]) && flag != FLAG_STORAGENUM && flag != FLAG_NUMBER && flag != FLAG_ERORRVAL)
		{
			int q = p->ls;
			while (isalpha(p->stp[++p->ls]));
			if (isdigit(p->stp[p->ls]))
			{
				while (isdigit(p->stp[++p->ls]));
				if (p->stp[p->ls] == ':')
				{
					int k = 0;
					if (isalpha(p->stp[++k + p->ls]))
						while (isalpha(p->stp[++k + p->ls]));
					else
						p->status = STATUS_VALUE_ERROR;
					if (isdigit(p->stp[k++ + p->ls]))
					{
						while (isdigit(p->stp[k++ + p->ls]));
						changeStr(p, q, p->ls);

					}
					else
						p->status = STATUS_VALUE_ERROR;
				}

				num = adminReference(p, q);
				if (p->status < 10)
					flag = FLAG_STORAGENUM;
				else
					flag = FLAG_ERORRVAL;
			}
			else if (p->stp[p->ls++] == '(')
			{
				/*Call to function by matching*/
				if (!(strncmp("SUM(", &(p->stp[q]), ((4 < p->ls - q) ? 4 : p->ls - q))))
				{
					if (flag != FLAG_NUMBER)
					{
						do
						{
							num += calculatorCell(p, 2);
						} while (p->stp[p->ls - 1] != ')' && p->stp[p->ls] != 0 && p->status != STATUS_VALUE_ERROR && p->status != STATUS_ZERO_DIVISION_ERROR);
						if (flag == FLAG_MINUS)
							num *= -1;
						flag = (p->status < 10) ? FLAG_STORAGENUM : FLAG_ERORRVAL;
					}
					else
						flag = FLAG_ERORRVAL;
				}
				else if (!(strncmp("AVERAGE(", &(p->stp[q]), ((8 < p->ls - q) ? 8 : p->ls - q))))
				{
					int j = 0;
					if (flag != FLAG_NUMBER)
					{
						do
						{
							j++;
							num += calculatorCell(p, 2);
						} while (p->stp[p->ls - 1] != ')' && p->stp[p->ls] != 0 && p->status != STATUS_VALUE_ERROR && p->status != STATUS_ZERO_DIVISION_ERROR);
						if (flag == FLAG_MINUS)
							num *= -1;
						num /= j;
						flag = (p->status != STATUS_ZERO_DIVISION_ERROR && p->status != STATUS_VALUE_ERROR) ? FLAG_STORAGENUM : FLAG_ERORRVAL;
					}
					else
						flag = FLAG_ERORRVAL;
				}
				else if (!(strncmp("MOD(", &(p->stp[q]), ((4 < p->ls - q) ? 4 : p->ls - q))))
				{
					if (flag != FLAG_NUMBER)
					{
						num = calculatorCell(p, 2);
						if (p->stp[p->ls - 1] == ')')
							p->status = STATUS_VALUE_ERROR;
						else
							num = (double)((int)num % (int)calculatorCell(p, 1));
						if (flag == FLAG_MINUS)
							num *= -1;
						flag = (p->status != STATUS_ZERO_DIVISION_ERROR && p->status != STATUS_VALUE_ERROR) ? FLAG_STORAGENUM : FLAG_ERORRVAL;
					}
					else
						flag = FLAG_ERORRVAL;
				}
				else if (!(strncmp("SIN(", &(p->stp[q]), ((4 < p->ls - q) ? 4 : p->ls - q))))
				{
					if (flag != FLAG_NUMBER)
					{
						num = sin(calculatorCell(p, 1));
						if (flag == FLAG_MINUS)
							num *= -1;
						flag = (p->status != STATUS_ZERO_DIVISION_ERROR && p->status != STATUS_VALUE_ERROR) ? FLAG_STORAGENUM : FLAG_ERORRVAL;
					}
					else
						flag = FLAG_ERORRVAL;
				}
				else if (!(strncmp("COS(", &(p->stp[q]), ((4 < p->ls - q) ? 4 : p->ls - q))))
				{
					if (flag != FLAG_NUMBER)
					{
						num = cos(calculatorCell(p, 1));
						if (flag == FLAG_MINUS)
							num *= -1;
						flag = (p->status != STATUS_ZERO_DIVISION_ERROR && p->status != STATUS_VALUE_ERROR) ? FLAG_STORAGENUM : FLAG_ERORRVAL;
					}
					else
						flag = FLAG_ERORRVAL;
				}
				else
				{
					p->status = STATUS_VALUE_ERROR;
					flag = FLAG_ERORRVAL;
				}
			}
		}
		if (flag != FLAG_ERORRVAL && flag != FLAG_ERORRDIVZERO)
			switch (p->stp[p->ls++])
			{
			case '(':
				if (flag != FLAG_NUMBER)
				{
					num = calculatorCell(p, 1);
					if (flag == FLAG_MINUS)
						num *= -1;
					flag = FLAG_STORAGENUM;
				}
				else
					flag = FLAG_WITHOUT;
				break;
			case '+':
				if (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM)
				{
					if (sit < 4)
					{
						num += calculatorCell(p, 4);
						flag = FLAG_STORAGENUM;
					}
					else
						flag = FLAG_EXIT;
				}
				break;
			case '-':
				if (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM)
				{
					if (sit < 4)
					{
						num -= calculatorCell(p, 5);
						flag = FLAG_STORAGENUM;
					}
					else
						flag = FLAG_EXIT;
				}
				else
					flag = (flag != FLAG_MINUS) ? FLAG_MINUS : FLAG_WITHOUT;
				break;
			case '*':
				if (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM)
					if (sit < 6)
					{
						num *= calculatorCell(p, 6);
						flag = FLAG_STORAGENUM;
					}
					else
						flag = FLAG_EXIT;
				break;
			case '/':
				if (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM)
					if (sit < 6)
					{
						div = calculatorCell(p, 7);
						if (div)
						{
							num /= div;
							flag = FLAG_STORAGENUM;
						}
						else
							flag = FLAG_ERORRDIVZERO;
					}
					else
						flag = FLAG_EXIT;
				break;
			case ',':
				if (sit != 0 && sit != 1 && (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM))
				{
					if (sit == 2)
						p->ls++;
					flag = FLAG_EXIT;
				}
				else
					flag = FLAG_WITHOUT;
				break;
			case ')':
				if (sit != 0 && (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM))
				{
					if (sit == 1 || sit == 2)
						p->ls++;
					flag = FLAG_EXIT;
				}
				else
					flag = FLAG_WITHOUT;
				break;
			case 0:
				if (sit != 1 && sit != 2 && (flag == FLAG_NUMBER || flag == FLAG_STORAGENUM))
					flag = FLAG_EXIT;
				else
					flag = FLAG_ERORRVAL;
				break;
			default:
				flag = FLAG_WITHOUT;
				break;
			}
	}
	if ((flag == FLAG_WITHOUT || flag == FLAG_ERORRDIVZERO || flag == FLAG_ERORRVAL) && p->status < 10)
		p->status = (flag == FLAG_ERORRDIVZERO) ? STATUS_ZERO_DIVISION_ERROR : STATUS_VALUE_ERROR;
	if (p->status > 10)
	{

		return 0;
	}
	p->ls--;
	p->status = STATUS_FORMULA_DECODED;
	return num;
}

/*Classification of types of strings*/
void classificationStr(cell* pn)
{
	int i, j;
	for (j = i = 0; pn->stp[i] != '\0'; i++)
	{
		if (!(isdigit(pn->stp[i]) || i == 0 && pn->stp[i] == '-' || pn->stp[i] == '.' && !j && i != 0))
			break;
		if (pn->stp[i] == '.')
			j = 1;
	}
	if (pn->stp[i] == '\0')
	{
		pn->status = (pn->stp[0] == '-') ? STATUS_DIGIT_MINUS : STATUS_DIGIT;
	
	}
	else if (pn->stp[0] == '=')
		pn->status = STATUS_FORMULA_TODECIPHE;
	else
		pn->status = STATUS_STRING;
}

/*Main reference management*/
double adminReference(cell* p, int q)
{
	int cr[2] = { -1,-1 };
	cell* npoint;
	whereReference(p, q, cr);
	npoint = findcell(cr[0], cr[1]);
	if (!npoint)
		return 0;
	assessmentStr(npoint);
	p->status = npoint->status;
	return npoint->num;
}
/*Cell finds*/
cell* findcell(int row, int column)
{
	int i = 0, low = 0, high = sizearr - 1;
	while (low < high)
	{
		i = (high + low) / 2;
		if (parr[i].addressRow > row || parr[i].addressRow == row && parr[i].addressColumn > column)
			high = i - 1;
		else
			low = i + 1;
	}
	if (parr[low].addressRow == row && parr[low].addressColumn == column)
		return parr + low;
	else if (low && parr[low - 1].addressRow == row && parr[low - 1].addressColumn == column)
		return parr + low - 1;
	else
		return 0;
}
/*Returns attribution*/
void whereReference(cell* pn, int i, int cr[2])
{
	int row = 0, column = 0, j = -1;
	while (isalpha(pn->stp[i]))
	{
		column = column * 26 + pn->stp[i] - ((pn->stp[i] < 'a') ? 'A' : 'a');
		i++;
		j++;
	}
	while (j > 0)
		column += (int)pow(26, j--);

	while (isdigit(pn->stp[i]))
		row = row * 10 + pn->stp[i++] - '0';
	row--;
	if (row < 0 || column < 0 || row >= 1048576 || column >= 16384)
		pn->status = STATUS_EXCEEDING_CELL_SIZE_ERROR;
	else
	{
		cr[0] = row;
		cr[1] = column;
	}
}
/*Cell center management*/
void adminCells()
{
	for (int i = 0; i < sizearr; i++)
		assessmentStr(parr + i);
}

/*Secondary management of string decoding*/
void assessmentStr(cell* p)
{
	if (p->rid == 2)
		return;
	else if (p->rid == 1)
	{
		p->status = CIRCULAR_REFERENCE_ERROR;
		return;
	}
	p->rid = 1;
	classificationStr(p);
	switch (p->status)
	{
	case STATUS_DIGIT:
	case STATUS_DIGIT_MINUS:
		partDigit(p);
		break;
	case STATUS_FORMULA_TODECIPHE:
		p->num = calculatorCell(p, 0);
		break;
	}
	p->rid = 2;
}

/*Assumes a real number according to the status of the place*/
void partDigit(cell* p)
{
	double num = 0;
	int j = 1, i;
	i = (p->status == STATUS_DIGIT) ? 0 : 1;
	while (isdigit(p->stp[i]))
		num = num * 10 + p->stp[i++] - '0';
	if (p->stp[i] == '.')
		while (isdigit(p->stp[++i]))
			num += (p->stp[i] - (double)'0') / pow(10, j++);
	p->num = num * ((p->status == STATUS_DIGIT_MINUS) ? -1 : 1);
}
/*free Memory*/
void freeall(cell* p)
{
	for (int i = 0; i < sizearr; i++)
		if ((p + i)->str[0] == 0 && (p + i)->stp && (p + i)->stp[0])
			free((p + i)->stp);
	free(p);
}
/*error print*/
void printStatusError(int status)
{
	switch (status)
	{
	case STATUS_EXCEEDING_CELL_SIZE_ERROR:
		printf("\nUnrecognized referral error");
		break;

	case STATUS_VALUE_ERROR:
		printf("\nerror value");
		break;
	case STATUS_ZERO_DIVISION_ERROR:
		printf("\nzero division error");
		break;
	case CIRCULAR_REFERENCE_ERROR:
		printf("\ncircular reference ");
		break;

	}
}
/*Subdivision of cell sequence*/
void changeStr(cell* p, int start, int colon)
{

	char temp[300] = { 0 }, add[100] = { 0 };
	int cr1[2] = { 1,1 }, cr2[2] = { 1,1 }, down, right, i, size, j, row, col, num, k;

	strcpy(temp, p->stp);
	if (p->str[0] == 0)
		free(p->stp);
	else
		p->str[0] = 0;
	whereReference(p, start, cr1);
	whereReference(p, colon + 1, cr2);
	down = (cr1[0] <= cr2[0]) ? 1 : -1;
	right = (cr1[1] <= cr2[1]) ? 1 : -1;
	row = cr1[0];
	col = cr1[1];
	k = 0;
	while (row != cr2[0] || col != cr2[1])
	{


		num = col;
		for (size = 1, i = 26; num >= i; size++)
		{
			num -= i;
			i *= i;
		}
		writrAddressColumnRecur(num, size, add + k);
		k = strlen(add);
		num = row + 1;
		j = 0;
		for (j = 0; num; j++)
			num /= 10;
		num = row + 1;
		while (j)
		{

			add[k++] = num / (int)pow(10, --j) + '0';
			add[k] = 0;

		}
		if (col != cr2[1])
			col += right;
		else
		{
			col = cr1[1];
			if (row != cr2[0])
				row += down;
		}
		add[k++] = ',';
		add[k] = 0;

	}
	if (add[0])
		add[k] = 0;
	temp[start] = 0;
	strcat(temp, strcat(add, temp + colon + 1));
	if (p->str[0] == 0 && p->stp && p->stp[0])
		free(p->stp);
	p->stp = (char*)malloc(sizeof(char) * (strlen(temp) + 1));

	if (p->stp == 0)
	{
		printf("Memory error");
		exit(1);
	}
	p->str[0] = 0;
	strcpy(p->stp, temp);
}
