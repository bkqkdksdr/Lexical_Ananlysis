// RealLexical_Analysis.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "iostream"
#include "map"
using namespace std;
//词法分析程序
//首先定义种别码
/*
第一类：标识符   letter(letter | digit)*  无穷集
第二类：数字    (digit)+  无穷集
第三类：保留字(32)
auto       break    case     char        const      continue
default    do       double   else        enum       extern
float      for      goto     if          int        long
register   return   short    signed      sizeof     static
struct     switch   typedef  union       unsigned   void
volatile    while

第四类：界符  ‘/*’、‘//’、 () { } [ ] " "  '
第五类：运算符 <、<=、>、>=、=、+、-、*、/、^、

对所有可数符号进行编码：
<$,0>
<auto,1>
...
<while,32>
<+，33>
<-,34>
<*,35>
</,36>
<<,37>
<<=,38>
<>,39>
<>=,40>
<=,41>
<==,42>
<!=,43>
<;,44>
<(,45>
<),46>
<^,47>
<,,48>
<",49>
<',50>
<#,51>
<&,52>
<&&,53>
<|,54>
<||,55>
<%,56>
<~,57>
<<<,58>左移
<>>,59>右移
<[,60>
<],61>
<{,62>
<},63>
<\,64>
<.,65>
<?,66>
<:,67>
<!,67>
<INT10 94  ,数值>
<INT8 95  ,数值>
<INT16 96  ,数值>
<REAL10 97  ,数值>
<REAL8 98  ,数值>
<REAL16 99  ,数值>
<标识符100 ，标识符指针>


*/

/****************************************************************************************/
//全局变量，保留字表，保留字表全为小写
static char ReserveWord[32][20] = {
	"auto", "break", "case", "char", "const", "continue",
	"default", "do", "double", "else", "enum", "then",
	"float", "for", "goto", "if", "int", "long",
	"register", "return", "short", "signed", "sizeof", "static",
	"struct", "switch", "typedef", "union", "unsigned", "void",
	"volatile", "while"
};
//输出时使用的大写保留字表
static char ReserveWordForOut[32][20] = {
	"AUTO", "BREAK", "CASE", "CHAR", "CONST", "CONTINUE",
	"DEFAULT", "DO", "DOUBLE", "ELSE", "ENUM", "THEN",
	"FLOAT", "FOR", "GOTO", "IF", "INT", "LONG",
	"RESIGTER", "RETURN", "SHORT", "SIGNED", "SIZEOF", "STATIC",
	"STRUCT", "SWITH", "TYPEDEF", "UNION", "UNSIGNED", "VOID",
	"VOLATILE", "WHILE"
};
//界符运算符表,根据需要可以自行增加
static char OperatorOrDelimiter[36][10] = {
	"+", "-", "*", "/", "<", "<=", ">", ">=", "=", "==",
	"!=", ";", "(", ")", "^", ",", "\"", "\'", "#", "&",
	"&&", "|", "||", "%", "~", "<<", ">>", "[", "]", "{",
	"}", "\\", ".", "\?", ":", "!"
};
//标识符表，每次识别之后增加
static  char IDNTable[1000][50] = { "" };
/****************************************************************************************/

/*********转化小数******************/
string toReal(char buf[], int n, int b)
{
	double s = 0;
	double base = b;
	int final;
	string st;
	char str[20];
	for (int i = n; buf[i] != '\0'; i++) { //获得小数部分的数值
		if (buf[i] >= '0' && buf[i] <= '9') {
			s = s + (double)((double)(buf[i] - '0') / (double)base);
			base *= b;
		}
	}
	final = (int)(s * 1000000000); //由于返回一个字符串变量，因此将小数部分扩大后再进行除法
	while (final % 10 == 0)
		final /= 10;
	sprintf(str, "%d", final);
	st = str;
	return st;
}
/*********转化小数******************/


/*********8进制数******************/
string Num8(char s[]) { //识别 8 进制数
	int i;
	int num = 0;
	char str[20];
	string re;
	for (i = 1; s[i] != '\0'; i++) {
		if (s[i] >= '0' && s[i] <= '7')
			num = (s[i] - '0') + num * 8;
		else if (s[i] == '.') { //发现小数点
			sprintf(str, "%d.", num);
			re = str;
			re += toReal(s, i + 1, 8);
			return re;
		}
	}
	sprintf(str, "%d", num);
	re = str;
	return re;
}
/*********8进制数******************/

/*********16进制数******************/
string Num16(char s[]) { //识别 16 进制数
	int i;
	int num = 0;
	char str[20];
	string re;
	for (i = 2; s[i] != '\0'; i++) {
		if ((s[i] >= '0' && s[i] <= '9') ||
			(s[i] >= 'a' && s[i] <= 'f') ||
			(s[i] >= 'A' && s[i] <= 'F')) {
			if (s[i] >= '0' && s[i] <= '9') {
				num = (s[i] - '0') + num * 16;
			}
			else if (s[i] >= 'a' && s[i] <= 'f') {
				num = (s[i] - 'a' + 10) + num * 16;
			}
			else if (s[i] >= 'A' && s[i] <= 'F') {
				num = (s[i] - 'A' + 10) + num * 16;
			}
		}
		else if (s[i] == '.') { //发现小数点
			sprintf(str, "%d.", num);
			re = str;
			re += toReal(s, i + 1, 16);
			return re;
		}
	}
	sprintf(str, "%d", num);
	re = str;
	return re;
}
/*********16进制数******************/

/********查找保留字*****************/
int SearchReserveWord(char ReserveWord[][20], char s[])
{
	for (int i = 0; i < 32; i++)
	{
		if (strcmp(ReserveWord[i], s) == 0)
		{//若成功查找，则返回种别码
			return i + 1;//返回种别码
		}
	}
	return -1;//否则返回-1，代表查找不成功，即为标识符
}
/********查找保留字*****************/

/*********************判断是否为字母********************/
bool IsLetter(char letter)
{//注意C语言允许下划线也为标识符的一部分可以放在首部或其他地方
	if (letter >= 'a' && letter <= 'z' || letter >= 'A' && letter <= 'Z' || letter == '_')
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*********************判断是否为字母********************/


/*****************判断是否为数字************************/
bool IsDigit(char digit)
{
	if (digit >= '0'&&digit <= '9')
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*****************判断是否为数字************************/


/********************编译预处理，取出无用的字符和注释**********************/
void FilterCode(char r[], int curStr)
{
	char tempString[10000];
	int count = 0;
	for (int i = 0; i <= curStr; i++)
	{
		if (r[i] == '/'&& r[i + 1] == '/')
		{//若为单行注释“//”,则去除注释后面的东西，直至遇到回车换行
			while (r[i] != '\n')
			{
				i++;//向后扫描
			}
		}
		if (r[i] == '/'&&r[i + 1] == '*')
		{//若为多行注释“/* 。。。*/”则去除该内容
			i += 2;
			while (r[i] != '*' || r[i + 1] != '/')
			{
				i++;//继续扫描
				if (r[i] == '$')
				{
					printf("注释出错，没有找到 */，程序结束！！！\n");
					exit(0);
				}
			}
			i += 2;//跨过“*/”
		}
		if (r[i] != '\n'&&r[i] != '\t'&&r[i] != '\v'&&r[i] != '\r')
		{//若出现无用字符，则过滤；否则加载
			tempString[count++] = r[i];
		}
	}
	tempString[count] = '\0';
	strcpy(r, tempString);//产生净化之后的源程序
}
/********************编译预处理，取出无用的字符和注释**********************/


/****************************分析子程序，算法核心***********************/
void Scan(int &syn, char OriginalCode[], char PendingStr[], int &curStr)
{//根据DFA的状态转换图设计 
	//syn是分析结果,OriginalCode是整个需要分析的代码,PendingStr每次进行分析的字符串,curStr第二个参数
	int i, count = 0;//count用来做PendingStr[]的下标，收集有用字符
	char ch;//作为判断使用
	ch = OriginalCode[curStr];
	while (ch == ' ')
	{//过滤空格，防止程序因识别不了空格而结束
		curStr++;
		ch = OriginalCode[curStr];
	}
	for (i = 0; i<20; i++)
	{//每次收集前先清零
		PendingStr[i] = '\0';
	}
	if (IsLetter(OriginalCode[curStr]))
	{//开头为字母
		PendingStr[count++] = OriginalCode[curStr];//收集
		curStr++;//下移
		while (IsLetter(OriginalCode[curStr]) || IsDigit(OriginalCode[curStr]))
		{//后跟字母或数字
			PendingStr[count++] = OriginalCode[curStr];//收集
			curStr++;//下移
		}//多读了一个字符既是下次将要开始的指针位置
		PendingStr[count] = '\0';
		syn = SearchReserveWord(ReserveWord, PendingStr);//查表找到种别码
		if (syn == -1)
		{//若不是保留字则是标识符
			syn = 100;//标识符种别码
		}
		return;
	}
	else if (IsDigit(OriginalCode[curStr]))
	{//首字符为数字
		char first = OriginalCode[curStr];
		char second;
		bool get = false;
		bool sign = false;
		while (IsDigit(OriginalCode[curStr]) ||
			OriginalCode[curStr] == 'x' ||
			OriginalCode[curStr] == 'X' ||
			OriginalCode[curStr] == '.' ||
			(OriginalCode[curStr] >= 'A' && OriginalCode[curStr] <= 'F') ||
			(OriginalCode[curStr] >= 'a' && OriginalCode[curStr] <= 'f')) {//后跟数字或者X与x,以及小数点
			if (OriginalCode[curStr] == '.') sign = true;
			PendingStr[count++] = OriginalCode[curStr];//收集
			curStr++;
			if (!get) {
				second = OriginalCode[curStr];
				get = true;
			}
		}//多读了一个字符既是下次将要开始的指针位置
		PendingStr[count] = '\0';
		if (sign) {
			if (first >= '1' && first <= '9') {
				syn = 97;
			}
			else if (first == '0') {
				if (second >= '0' && second <= '7')
					syn = 98;
				else if (second == 'x' || second == 'X')
					syn = 99;
				else
					syn = 97;
			}
		}
		else {
			if (first >= '1' && first <= '9') {
				syn = 94;
			}
			else if (first == '0') {
				if (second >= '0' && second <= '7')
					syn = 95;
				else if (second == 'x' || second == 'X')
					syn = 96;
				else
					syn = 94;
			}
		}

	}
	else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ';' || ch == '(' || ch == ')' || ch == '^'
		|| ch == ',' || ch == '\"' || ch == '\'' || ch == '~' || ch == '#' || ch == '%' || ch == '['
		|| ch == ']' || ch == '{' || ch == '}' || ch == '\\' || ch == '.' || ch == '\?' || ch == ':')
	{//若为运算符或者界符，查表得到结果
		PendingStr[0] = OriginalCode[curStr];
		PendingStr[1] = '\0';//形成单字符串
		for (i = 0; i<36; i++)
		{//查运算符界符表
			if (strcmp(PendingStr, OperatorOrDelimiter[i]) == 0)
			{
				syn = 33 + i;//获得种别码，使用了一点技巧，使之呈线性映射
				break;//查到即退出
			}
		}
		curStr++;//指针下移，为下一扫描做准备
		return;
	}
	else  if (OriginalCode[curStr] == '<')
	{//<,<=,<<
		curStr++;//后移，超前搜索
		if (OriginalCode[curStr] == '=')
		{
			syn = 38;
		}
		else if (OriginalCode[curStr] == '<')
		{//左移
			curStr--;
			syn = 58;
		}
		else
		{
			curStr--;
			syn = 37;
		}
		curStr++;//指针下移
		return;
	}
	else  if (OriginalCode[curStr] == '>')
	{//>,>=,>>
		curStr++;
		if (OriginalCode[curStr] == '=')
		{
			syn = 40;
		}
		else if (OriginalCode[curStr] == '>')
		{
			syn = 59;
		}
		else
		{
			curStr--;
			syn = 39;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '=')
	{//=.==
		curStr++;
		if (OriginalCode[curStr] == '=')
		{
			syn = 42;
		}
		else
		{
			curStr--;
			syn = 41;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '!')
	{//!,!=
		curStr++;
		if (OriginalCode[curStr] == '=')
		{
			syn = 43;
		}
		else
		{
			syn = 68;
			curStr--;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '&')
	{//&,&&
		curStr++;
		if (OriginalCode[curStr] == '&')
		{
			syn = 53;
		}
		else
		{
			curStr--;
			syn = 52;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '|')
	{//|,||
		curStr++;
		if (OriginalCode[curStr] == '|')
		{
			syn = 55;
		}
		else
		{
			curStr--;
			syn = 54;
		}
		curStr++;
		return;
	}
	else  if (OriginalCode[curStr] == '$')
	{//结束符
		syn = 0;//种别码为0
	}
	else
	{//不能被以上词法分析识别，则出错。
		printf("error：there is no exist %c \n", ch);
		exit(0);
	}
}


int main()
{
	//打开一个文件，读取其中的源程序
	char OriginalCode[10000];
	char PendingStr[20] = { 0 };
	int syn = -1, i;//初始化
	int curStr = 0;//源程序指针
	FILE *fp, *fp1;
	if ((fp = fopen("LA_OriginalCode.txt", "r")) == NULL)
	{//打开源程序
		cout << "can not open this file";
		exit(0);
	}
	OriginalCode[curStr] = fgetc(fp);
	while (!feof(fp))
	{//将源程序读入OriginalCode[]数组
		curStr++;
		OriginalCode[curStr] = fgetc(fp);
	}
	OriginalCode[curStr] = '$';
	OriginalCode[++curStr] = '\0';
	fclose(fp);
	cout << endl << "源程序为:" << endl;
	cout << OriginalCode << endl;
	//对源程序进行过滤
	FilterCode(OriginalCode, curStr);
	cout << endl << "过滤之后的程序:" << endl;
	cout << OriginalCode << endl;
	curStr = 0;//从头开始读

	if ((fp1 = fopen("LA_CodeCompile.txt", "w+")) == NULL)
	{//打开源程序
		cout << "can not open this file";
		exit(0);
	}//提交，test  在测试
	while (syn != 0)
	{
		//启动扫描
		Scan(syn, OriginalCode, PendingStr, curStr);
		if (syn == 100)
		{//标识符
			for (i = 0; i<1000; i++)
			{//插入标识符表中
				if (strcmp(IDNTable[i], PendingStr) == 0)
				{//已在表中
					break;
				}
				if (strcmp(IDNTable[i], "") == 0)
				{//查找空间
					strcpy(IDNTable[i], PendingStr);
					break;
				}
			}
			printf("(IDN  ,%s)\n", PendingStr);
			fprintf(fp1, "(IDN   ,%s)\n", PendingStr);
		}
		else if (syn >= 1 && syn <= 32)
		{//保留字
			printf("(%s   ,  _)\n", ReserveWordForOut[syn - 1]);
			fprintf(fp1, "(%s   ,  _)\n", ReserveWordForOut[syn - 1]);
		}
		else if (syn == 94)
		{//10进制整数
			printf("(INT10   ,   %s)\n", PendingStr);
			fprintf(fp1, "(INT10   ,   %s)\n", PendingStr);
		}
		else if (syn == 95)
		{//8进制整数
			string temp = Num8(PendingStr);
			strcpy(PendingStr, temp.c_str());
			printf("(INT8   ,   %s)\n", PendingStr);
			fprintf(fp1, "(INT8   ,   %s)\n", PendingStr);
		}
		else if (syn == 96)
		{//16进制整数
			string temp = Num16(PendingStr);
			strcpy(PendingStr, temp.c_str());
			printf("(INT16   ,   %s)\n", PendingStr);
			fprintf(fp1, "(INT16   ,   %s)\n", PendingStr);
		}
		else if (syn == 97)
		{//10进制浮点
			printf("(REAL10   ,   %s)\n", PendingStr);
			fprintf(fp1, "(REAL10   ,   %s)\n", PendingStr);
		}
		else if (syn == 98)
		{//8进制浮点
			string temp = Num8(PendingStr);
			strcpy(PendingStr, temp.c_str());
			printf("(REAL8   ,   %s)\n", PendingStr);
			fprintf(fp1, "(REAL8   ,   %s)\n", PendingStr);
		}
		else if (syn == 99)
		{//16进制浮点
			string temp = Num16(PendingStr);
			strcpy(PendingStr, temp.c_str());
			printf("(REAL16   ,   %s)\n", PendingStr);
			fprintf(fp1, "(REAL16   ,   %s)\n", PendingStr);
		}
		else if (syn >= 33 && syn <= 68)
		{
			printf("(%s   ,   _)\n", OperatorOrDelimiter[syn - 33]);
			fprintf(fp1, "(%s   ,   _)\n", OperatorOrDelimiter[syn - 33]);
		}
	}
	for (i = 0; strcmp(IDNTable[i], "") != 0; i++)
	{//插入标识符表中
		printf("第%d个标识符：  %s\n", i + 1, IDNTable[i]);
		fprintf(fp1, "第%d个标识符：  %s\n", i + 1, IDNTable[i]);
	}
	fclose(fp1);//4554
	return 0;
}
