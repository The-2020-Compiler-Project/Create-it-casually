//词法分析中' '代表空白字符，包括换行符，空格，制表符
//源程序格式：1、一行只能有一条语句；2、程序中可以有注释
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stack>
#include <stdlib.h>
using namespace std;

char src[1000010];    //存储源代码
char TokenList_kind[1010][100];    //词法单元流 之 类别
int TokenList_value[1010];    //词法单元流 之 值
char WordList[1010][100];    //符号表 -- 由token_value指向
int LineList[1010];        //记录每一个token对应的行数 -- 由token_value指向
int TokenListPoint;        //词法单元流指针
int WordListPoint;        //符号表指针

int WordsPoint;        //语法分析时的词法单元流指针

int tmpcnt;        //三地址语句中寄存器的编号

int ExpTable[11][8] = {    //用 表驱动法 读取表达式 ，状态转换表
2,    -1,    1,    -1,    -1,    -1,    -1,    9,
2,    -1,    -1,    -1,    -1,    -1,    -1,    9,
2,    -1,    -1,    3,    -1,    -1,    -1,    9,
4,    5,    -1,    -1,    -1,    -1,    -1,    9,
4,    -1,    8,    -1,    6,    -1,    10,    9,
-1,    5,    8,    -1,    6,    -1,    10,    9,
4,    5,    -1,    -1,    -1,    7,    -1,    9,
4,    5,    -1,    -1,    -1,    -1,    -1,    9,
-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
-1,    -1,    8,    -1,    6,    -1,    10,    9
};

/*
  文法
1    Program->BEGIN Stmt-List END
2    Stmt-List->Stmt Stmt-List'
3    Stmt-List'->Stmt Stmt-List' | .
4    Stmt->Assign-stmt
5    Assign-stmt->ID = Expr
6    Expr->Term Expr'
7    Expr'->Add-Op Term Expr' | .
8    Term->Factor Term'
9    Term'->Multiple-Op Factor Term' | .
10    Factor->( Expr ) | ID | NUM
11    Add-Op->+ | -
12    Multiple-Op->* | /
/*
FIRST集
First(Program)={BEGIN}
First(Stmt-List)={ID}
First(Stmt-List')={ ID,ε}
First(Stmt)={ ID}
First(Assign-stmt)={ ID}
First(Expr)={(,ID,NUM}
First(Expr')={+,-,ε}
First(Term)={(,ID,NUM}
First(Term')={*,/,ε}
First(Factor)={(,ID,NUM}
First(Add-Op)={+,-}
First(Multiple-Op)={*,/}
                /*
FOLLOW集
Follow(Program)={$}
Follow(Stmt-List)={END}
Follow(Stmt-List')={ END}
Follow(Stmt)={ ID,END}
Follow(Assign-stmt)={ ID,END }
Follow(Expr)={ ID,END ,)}
Follow(Expr')={ ID,END , )}
Follow(Term)={+,-, ID,END , )}
Follow(Term')={ +,-, ID,END , )}
Follow(Factor)={ *,/,+,-, ID,END , )}
Follow(Add-Op)={(,ID,NUM}
Follow(Multiple-Op)={ (,ID,NUM }
                       /*
  预测分析表
非终结符号    输入符号
            BEGIN    ‘+’    ‘-’    ‘*’    ‘/’    (    )    ‘=’    ID    NUM    END    $
Program        1
Stmt-List                                                            2
Stmt-List'                                                            3        .
Stmt                                                                4
Assign-stmt                                                            5
Expr                                                6                6    6
Expr'                7        7                            .            .        .
Term                                                8                8    8
Term'                .        .        9        9            .            .        .
Factor                                                10_1            10_210_3
Add-Op                11_1    11_2
Multiple-Op                            12_1    12_2
*/

char PreTab[12][12][100]={    //存储预测分析表
    {"BEGIN Stmt-List END","","","","","","","","","","",""},
    {"","","","","","","","","Stmt Stmt-List'","","",""},
    {"","","","","","","","","Stmt Stmt-List'","",".",""},
    {"","","","","","","","","Assign-stmt","","",""},
    {"","","","","","","","","ID = Expr","","",""},
    {"","","","","","Term Expr'","","","Term Expr'","Term Expr'","",""},
    {"","Add-Op Term Expr'","Add-Op Term Expr'","","","",".","",".","",".",""},
    {"","","","","","Factor Term'","","","Factor Term'","Factor Term'","",""},
    {"",".",".","Multiple-Op Factor Term'","Multiple-Op Factor Term'","",".","",".","",".",""},
    {"","","","","","( Expr )","","","ID","NUM","",""},
    {"","+","-","","","","","","","","",""},
    {"","","","*","/","","","","","","",""}
};

char Product[23][50]={    //记录产生式出现的字符串
    "Program",      //0
    "Stmt-List",    //1
    "Stmt-List'",   //2
    "Stmt",         //3
    "Assign-stmt",  //4
    "Expr",         //5
    "Expr'",        //6
    "Term",         //7
    "Term'",        //8
    "Factor",       //9
    "Add-Op",       //10
    "Multiple-Op",  //11

    "BEGIN",        //12
    "+",            //13
    "-",            //14
    "*",            //15
    "/",            //16
    "(",            //17
    ")",            //18
    "=",            //19
    "ID",           //20
    "NUM",          //21
    "END"           //22
};


//语法树节点
struct Node{
    Node(char na[]){    //构造函数
        strcpy(name,na);
        wp = 0;
        brother = NULL;
        next = NULL;
    }
    char name[50];
    int wp;    //终结符在token流中的位置
    Node* brother;  //指向下一个兄弟节点
    Node* next;     //指向当前节点的孩子节点
}*root,*now,*p;

void Init()
{
    memset(src,0,sizeof(src));    //初始化源代码字符数组
    memset(TokenList_kind,0,sizeof(TokenList_kind));    //初始化词法单元流
    memset(TokenList_value,0,sizeof(TokenList_value));    //初始化词法单元流
    memset(WordList,0,sizeof(WordList));    //初始化符号表
    TokenListPoint = 1;
    WordListPoint = 1;

    WordsPoint = 1;

    //初始化指针
    root = NULL;
    now = NULL;
    p = NULL;
    tmpcnt = 1;
}

//--------------- 词法分析 start ---------------
void AddToken(char kind[],char value[],int line)    //将<类别，值>放入token流中
{
    strcpy(TokenList_kind[TokenListPoint],kind);
    TokenList_value[TokenListPoint++] = WordListPoint;

    strcpy(WordList[WordListPoint],value);
    LineList[WordListPoint++] = line;
}

void strsub(char str1[],int start,int len,char str2[])    //截取str1的子字符串给str2
{
    int i=0,j;
    for(j=0;j<len;j++){
        str2[i++] = str1[start+j];
    }
    str2[i] = '\0';
}

void InputString()    //从文件中读取源代码，同时过滤注释，一行只能放置一条语句
{
    //文件读入
    FILE* fs = fopen(".\\src.txt","rb");    //源代码
    if(fs==NULL){
        printf("源代码文件 src.txt 不存在\n");
        return ;
    }

    char s[10010]={0};
    //fscanf(fs,"%s",src);
    while(fgets(s,10010,fs)){
        if(sscanf(s,"%s",s)==-1){    //没有读到字符串，将s清空
            s[0]='\0';
            continue;
        }
        //过滤注释
        if(s[0]=='/' && s[1]=='/')
            continue;
        int i;
        for(i=0;s[i];i++)
            if(s[i]=='/' && s[i+1]=='/')
                break;
        s[i]='\0';

        strcat(src,s);    //连接到源代码字符串后
        strcat(src," ");    //连接一个空白符
    }

    int len = strlen(src);
    len--;
    src[len] = '\0';
}

bool getBEGIN(int &i,int &line)    //读取“BEGIN”
{
    char tmp[6];
    strsub(src,i,5,tmp);
    if(strcmp(tmp,"BEGIN")==0){
        i = i+5;
        AddToken("BEGIN","BEGIN",line);    //添加token,<类别，值>
        return true;
    }
    else
        return false;
}
bool getBLANK(int &i)    //读取“空白符”==>' '
{
    if(src[i]==' '){
        i++;
        return true;
    }
    else
        return false;
}
bool getEND(int &i,int &line)        //读取“END”
{
    char tmp[4];
    strsub(src,i,3,tmp);
    if(strcmp(tmp,"END")==0){
        i = i+3;
        AddToken("END","END",line);    //添加token,<类别，值>
        return true;
    }
    else
        return false;
}
bool getExp(int &i,int &line)        //读取表达式，遇到' '结束
{
    int status=0;
    char tmp[10010]={0};
    char t[2]={0};
    int j=0;
    stack <char> ss;
    while(status!=8){
        if(status==-1)    //跳到错误的状态号，返回false
            return false;
        if(status==9)    //跳到了错误状态，返回false
            return false;

        //根据src[i]确定下一个状态，直到抵达结束状态 8
        if( ('a'<=src[i] && src[i]<='z') || ('A'<=src[i] && src[i]<='Z') ){
            //读取到字母
            status = ExpTable[status][0];
            tmp[j++] = src[i++];
        }
        else if('0'<=src[i] && src[i]<='9'){
            //读取到数字
            status = ExpTable[status][1];
            tmp[j++] = src[i++];
        }
        else{
            switch(src[i]){
            case ' ':    //空白符
                status = ExpTable[status][2];
                if(tmp[0]!='\0'){
                    //如果是后来读到空白符，说明表达式该结束了
                    //将读取的最后一个单词放入token
                    if(j!=0){    //说明之前是一个单词或数字，否则是一个），）已经将最后一个单词放入token流中了，所以不需要处理它
                        if( ('a'<=tmp[0] && tmp[0]<='z') || ('A'<=tmp[0] && tmp[0]<='Z') )
                            AddToken("ID",tmp,line);    //将这个字符代表的单词放入token流中
                        else if( '0'<=tmp[0] && tmp[0]<='9' )
                            AddToken("NUM",tmp,line);    //将这个字符代表的单词放入token流中
                        j=0;
                    }
                }
                line++;
                i++;
                break;
            case '=':    // =
                status = ExpTable[status][3];

                tmp[j] = '\0';
                AddToken("ID",tmp,line);    //=前面一定是标识符，放入token流
                j=0;
                AddToken("=","=",line);    //将当前的=也放入token流

                i++;
                break;
            case '+':    //OP
            case '-':
            case '*':
            case '/':
                status = ExpTable[status][4];

                tmp[j] = '\0';
                if( ('a'<=tmp[0] && tmp[0]<='z') || ('A'<=tmp[0] && tmp[0]<='Z') )    //如果运算符前是字母，则说明是标识符
                    AddToken("ID",tmp,line);
                else if( '0'<=tmp[0] && tmp[0]<='9' )
                    AddToken("NUM",tmp,line);
                j=0;
                t[0] = src[i];
                t[1] = '\0';
                AddToken(t,t,line);    //将运算符放入token流

                i++;
                break;
            case '(':    // (
                status = ExpTable[status][5];

                AddToken("(","(",line);    //将（放入token流
                ss.push('(');

                i++;
                break;
            case ')':    // )
                status = ExpTable[status][6];

                tmp[j] = '\0';
                if( ('a'<=tmp[0] && tmp[0]<='z') || ('A'<=tmp[0] && tmp[0]<='Z') )    //如果)前是字母，则说明是标识符，即ID
                    AddToken("ID",tmp,line);
                else if( '0'<=tmp[0] && tmp[0]<='9' )
                    AddToken("NUM",tmp,line);
                j=0;

                AddToken(")",")",line);    //将（放入token流

                if(ss.empty())    //括号不匹配
                    return false;
                else
                    ss.pop();

                i++;
                break;
            default:    //其它
                status = ExpTable[status][7];
                i++;
                break;
            }
        }
    }

    if(status==8){    //正确跳出
        /*
        if(ss.empty())    //括号匹配
            return true;
        else
            return false;
        */
        return true;
    }
    else
        return false;
}

bool Lex()    //进行词法分析
{
    printf("词法分析......\n");
    int i=0;
    int line = 1;
    InputString();
    try{
        getBEGIN(i,line)?1:throw 1;
        getBLANK(i)?line++:throw 2;
        while(1){    //读取表达式
            if(src[i]=='\0')    //还没检测到END，程序就结束了
                throw 3;
            char tmp[4];
            strsub(src,i,3,tmp);    //截取字符串
            if(strcmp(tmp,"END")==0){
                //如果检测到END，跳出循环
                break;
            }

            //读取表达式
            getExp(i,line)?1:throw 4;
        }
        getEND(i,line)?1:throw 3;
        if(src[i]!='\0')    //如果END结束标志之后还有输入符号，说明出错误了
            throw 5;
    }
    catch(int err){
        printf("【词法错误】\n");
        //计算行号
        int errline = 1;
        int j;
        for(j=0;j<=i;j++)
            if(src[j]==' ')
                errline++;

        switch(err){
        case 1:
            printf("ERROR 1 (第%d行) : 没有读取到开始标识 BEGIN!\n",errline);
            printf("具体位置：\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        case 2:
            printf("ERROR 2 (第%d行) : 没有读取到空白符!\n",errline);
            printf("具体位置：\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        case 3:
            printf("ERROR 3 (第%d行) : 没有读取到结束标识 END!\n",errline);
            printf("具体位置：\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            printf("\n");
            return false;
        case 4:
            printf("ERROR 4 (第%d行) : 表达式错误。例如：标识符有误!\n",errline);
            printf("具体位置：\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        case 5:
            printf("ERROR 5 (第%d行) : BEGIN...END 代码段格式错误!\n",errline);
            printf("具体位置：\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        default:break;
        }
    }
    printf("没有词法错误\n");
    printf("\n");
    return true;
}

void printTokenList()    //输出token流
{
    int i=1;
    printf("单词流：\n");
    printf("<\t类别\t,值\t>\n");
    for(;i<TokenListPoint;i++){
        printf("<\t%s\t,%s\t>\n",TokenList_kind[i],WordList[TokenList_value[i]]);
    }
    printf("\n");
}
//--------------- 词法分析 end ---------------


//--------------- 语法分析 start ---------------
//生成语法分析树

void OutStack(stack <char*> z)        //输出栈中内容的前两个
{
    char t[200]={0};
    int j=0;
    while(!z.empty()){
        if(j==2)
            break;
        if(j==1)
            strcat(t,",");
        strcat(t,z.top());
        z.pop();
        j++;
    }
    strcat(t,"$");
    printf("%23s",t);
}

void OutRightStr()    //输出当前token流中前两个
{
    char t[200]={0};
    int i,j=0;
    for(i = WordsPoint;i<WordListPoint;i++){
        if(j==2)
            break;
        char tt[200]={0};
        sprintf(tt,"<%s,%s>",TokenList_kind[i],WordList[TokenList_value[i]]);
        strcat(t,tt);
        j++;
    }
    printf("%23s$",t);
}

void OutAction(char action[])    //输出动作
{
    printf(" %s\n",action);
}

bool seltab(int row,char f[])
{
    if(strcmp(TokenList_kind[WordsPoint],"BEGIN")==0){
        strcpy(f,PreTab[row][0]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"+")==0){
        strcpy(f,PreTab[row][1]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"-")==0){
        strcpy(f,PreTab[row][2]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"*")==0){
        strcpy(f,PreTab[row][3]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"/")==0){
        strcpy(f,PreTab[row][4]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"(")==0){
        strcpy(f,PreTab[row][5]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],")")==0){
        strcpy(f,PreTab[row][6]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"=")==0){
        strcpy(f,PreTab[row][7]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"ID")==0){
        strcpy(f,PreTab[row][8]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"NUM")==0){
        strcpy(f,PreTab[row][9]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"END")==0){
        strcpy(f,PreTab[row][10]);
    }
    else if(strcmp(TokenList_kind[WordsPoint],"$")==0){
        strcpy(f,PreTab[row][11]);
    }
    else{
        return false;
    }
    if(f[0]=='\0')    //确定的表位置为空
        return false;
    return true;
}

bool Parse()    //语法分析阶段。输入是token流，输出分析结果
{
    stack <char*> z;
    stack <Node*> nz;
    z.push(*Product);

    root = new Node("Program");
    now = root;

    printf("词法分析......\n");
    printf("【分析流程】\n");
    printf("%22s%24s%22s\n","栈","输入","动作");
    char action[100]={0};
    try{
        while(!z.empty() || WordsPoint<WordListPoint){    //栈和输入串中只剩结束符号（栈空 or 指向'\0'）时退出循环

            //输出当前分析结果
            OutStack(z);        //输出栈中内容
            OutRightStr();    //输出剩余输入字符串
            OutAction(action);    //输出动作

            memset(action,0,sizeof(action));

            //预处理。例：匹配e。预处理：将e出栈，输入指针后移。

            //还没结束栈就空了
            if(z.empty())
                throw 1;    //非法跳出

            if(strcmp(z.top(),TokenList_kind[WordsPoint])==0){    //栈顶元素==指针指向字符，匹配成功
                //语义动作 - 构造语法树
                while(!now->brother){    //回到有下一个兄弟节点的节点为止
                    if(nz.empty()){
                        if(now==root)    //如果回到了根节点
                            goto label;
                        else
                            throw 1;
                    }
                    now = nz.top();
                    nz.pop();
                }
                now = now->brother;

label:
                //准备输出字符串 action
                strcat(action,"匹配");
                strcat(action,z.top());

                //出栈、指针后移
                if(!z.empty())
                    z.pop();
                if(WordsPoint<WordListPoint)
                    WordsPoint++;
            }
            else{    //不相等，输出推导
                //确定推导
                char f[200]={0};
                if(strcmp(z.top(),"Program")==0){
                    if(!seltab(0,f))    //从表中确定推导串
                        throw 2;    //没有找到对应的推导
                }
                else if(strcmp(z.top(),"Stmt-List")==0){
                    if(!seltab(1,f))    //从表中确定推导串
                        throw 2;    //没有找到对应的推导
                }
                else if(strcmp(z.top(),"Stmt-List'")==0){
                    if(!seltab(2,f))    //从表中确定推导串
                        throw 2;    //没有找到对应的推导
                }
                else if(strcmp(z.top(),"Stmt")==0){
                    if(!seltab(3,f))    //从表中确定推导串
                        throw 2;    //没有找到对应的推导
                }
                else if(strcmp(z.top(),"Assign-stmt")==0){
                    if(!seltab(4,f))    //从表中确定推导串
                        throw 2;    //没有找到对应的推导
                }
                else if(strcmp(z.top(),"Expr")==0){
                    if(!seltab(5,f))    //从表中确定推导串
                        throw 2;
                }
                else if(strcmp(z.top(),"Expr'")==0){
                    if(!seltab(6,f))    //从表中确定推导串
                        throw 2;
                }
                else if(strcmp(z.top(),"Term")==0){
                    if(!seltab(7,f))    //从表中确定推导串
                        throw 2;
                }
                else if(strcmp(z.top(),"Term'")==0){
                    if(!seltab(8,f))    //从表中确定推导串
                        throw 2;
                }
                else if(strcmp(z.top(),"Factor")==0){
                    if(!seltab(9,f))    //从表中确定推导串
                        throw 2;
                }
                else if(strcmp(z.top(),"Add-Op")==0){
                    if(!seltab(10,f))    //从表中确定推导串
                        throw 2;
                }
                else if(strcmp(z.top(),"Multiple-Op")==0){
                    if(!seltab(11,f))    //从表中确定推导串
                        throw 2;
                }
                else{
                    throw 2;
                }
                //准备输出字符串 action
                strcat(action,"输出");
                strcat(action,z.top());
                strcat(action,"->");
                strcat(action,f);

                //将栈顶字符串记录下来后用
                char proleft[50];
                //将栈顶元素出栈并将推导入栈
                if(!z.empty()){
                    strcpy(proleft,z.top());
                    z.pop();
                }
                else
                    throw 1;

                char tmp[100];

                //如果推导出来的是空，即f->.，不作处理
                if(f[0]=='.'){
                    nz.push(now);

                    now->next = new Node(".");
                    now = now->next;    //now指向当前产生式右边的第一个字符串

                    while(!now->brother){    //回到有下一个兄弟节点的节点为止
                        if(nz.empty()){
                            if(now==root)    //如果回到了根节点
                                goto label;
                            else
                                throw 1;
                        }
                        now = nz.top();
                        nz.pop();
                    }
                    now = now->brother;

                    continue;
                }
                stack <char*> tz;    //临时的栈

                //正向输入到临时栈中
                while(sscanf(f,"%s",tmp)!=-1){
                    //语义动作 - 创建语法树
                    if(strcmp(proleft,now->name)==0){
                        nz.push(now);    //将当前节点记录下来

                        now->next = new Node(tmp);
                        now = now->next;    //now指向当前产生式右边的第一个字符串
                        p = now;
                    }
                    else{
                        p->brother = new Node(tmp);
                        p = p->brother;
                    }

                    char* pos = strstr(f,tmp);
                    strcpy(f,pos+strlen(tmp));
                    if(strcmp(tmp,"Program")==0){
                        tz.push(*(Product));
                    }
                    else if(strcmp(tmp,"Stmt-List")==0){
                        tz.push(*(Product+1));
                    }
                    else if(strcmp(tmp,"Stmt-List'")==0){
                        tz.push(*(Product+2));
                    }
                    else if(strcmp(tmp,"Stmt")==0){
                        tz.push(*(Product+3));
                    }
                    else if(strcmp(tmp,"Assign-stmt")==0){
                        tz.push(*(Product+4));
                    }
                    else if(strcmp(tmp,"Expr")==0){
                        tz.push(*(Product+5));
                    }
                    else if(strcmp(tmp,"Expr'")==0){
                        tz.push(*(Product+6));
                    }
                    else if(strcmp(tmp,"Term")==0){
                        tz.push(*(Product+7));
                    }
                    else if(strcmp(tmp,"Term'")==0){
                        tz.push(*(Product+8));
                    }
                    else if(strcmp(tmp,"Factor")==0){
                        tz.push(*(Product+9));
                    }
                    else if(strcmp(tmp,"Add-Op")==0){
                        tz.push(*(Product+10));
                    }
                    else if(strcmp(tmp,"Multiple-Op")==0){
                        tz.push(*(Product+11));
                    }
                    else if(strcmp(tmp,"BEGIN")==0){
                        tz.push(*(Product+12));
                    }
                    else if(strcmp(tmp,"+")==0){
                        tz.push(*(Product+13));
                    }
                    else if(strcmp(tmp,"-")==0){
                        tz.push(*(Product+14));
                    }
                    else if(strcmp(tmp,"*")==0){
                        tz.push(*(Product+15));
                    }
                    else if(strcmp(tmp,"/")==0){
                        tz.push(*(Product+16));
                    }
                    else if(strcmp(tmp,"(")==0){
                        tz.push(*(Product+17));
                    }
                    else if(strcmp(tmp,")")==0){
                        tz.push(*(Product+18));
                    }
                    else if(strcmp(tmp,"=")==0){
                        tz.push(*(Product+19));
                    }
                    else if(strcmp(tmp,"ID")==0){
                        tz.push(*(Product+20));
                    }
                    else if(strcmp(tmp,"NUM")==0){
                        tz.push(*(Product+21));
                    }
                    else if(strcmp(tmp,"END")==0){
                        tz.push(*(Product+22));
                    }
                    else{
                        throw 1;
                    }

                }

                //反向输出到真正的栈中
                while(!tz.empty()){
                    if(strcmp(tz.top(),"Program")==0){
                        z.push(*(Product));
                    }
                    else if(strcmp(tz.top(),"Stmt-List")==0){
                        z.push(*(Product+1));
                    }
                    else if(strcmp(tz.top(),"Stmt-List'")==0){
                        z.push(*(Product+2));
                    }
                    else if(strcmp(tz.top(),"Stmt")==0){
                        z.push(*(Product+3));
                    }
                    else if(strcmp(tz.top(),"Assign-stmt")==0){
                        z.push(*(Product+4));
                    }
                    else if(strcmp(tz.top(),"Expr")==0){
                        z.push(*(Product+5));
                    }
                    else if(strcmp(tz.top(),"Expr'")==0){
                        z.push(*(Product+6));
                    }
                    else if(strcmp(tz.top(),"Term")==0){
                        z.push(*(Product+7));
                    }
                    else if(strcmp(tz.top(),"Term'")==0){
                        z.push(*(Product+8));
                    }
                    else if(strcmp(tz.top(),"Factor")==0){
                        z.push(*(Product+9));
                    }
                    else if(strcmp(tz.top(),"Add-Op")==0){
                        z.push(*(Product+10));
                    }
                    else if(strcmp(tz.top(),"Multiple-Op")==0){
                        z.push(*(Product+11));
                    }
                    else if(strcmp(tz.top(),"BEGIN")==0){
                        z.push(*(Product+12));
                    }
                    else if(strcmp(tz.top(),"+")==0){
                        z.push(*(Product+13));
                    }
                    else if(strcmp(tz.top(),"-")==0){
                        z.push(*(Product+14));
                    }
                    else if(strcmp(tz.top(),"*")==0){
                        z.push(*(Product+15));
                    }
                    else if(strcmp(tz.top(),"/")==0){
                        z.push(*(Product+16));
                    }
                    else if(strcmp(tz.top(),"(")==0){
                        z.push(*(Product+17));
                    }
                    else if(strcmp(tz.top(),")")==0){
                        z.push(*(Product+18));
                    }
                    else if(strcmp(tz.top(),"=")==0){
                        z.push(*(Product+19));
                    }
                    else if(strcmp(tz.top(),"ID")==0){
                        z.push(*(Product+20));
                    }
                    else if(strcmp(tz.top(),"NUM")==0){
                        z.push(*(Product+21));
                    }
                    else if(strcmp(tz.top(),"END")==0){
                        z.push(*(Product+22));
                    }
                    else{
                        throw 1;
                    }
                    tz.pop();
                }
            }

        }
        if(z.empty() && WordsPoint >= WordListPoint){    //正常退出循环
            //输出最后一行分析结果
            OutStack(z);        //输出栈中内容
            OutRightStr();    //输出剩余输入字符串
            OutAction(action);    //输出动作
        }
        else{    //非正常情况
            throw 1;
        }
    }
    catch(int err){
        printf("\n");
        printf("【语法错误】\n");

        switch(err){
        case 1:
            printf("ERROR 1 (第%d行) : 非法跳出!\n",LineList[WordsPoint]);
            printf("\n");
            return false;
        case 2:
            printf("ERROR 2 (第%d行) : 没有找到 %s 对应的推导!\n",LineList[WordsPoint],z.top());
            printf("\n");
            return false;
        default:
            break;
        }
    }
    printf("没有语法错误\n");
    printf("\n");
    return true;
}
//--------------- 语法分析 end ---------------

//--------------- 生成三地址代码 start ---------------
//对语法分析树进行后序遍历，执行语义规则，一遍扫描之后，树根的code即为三地址代码


void setWP(Node* cur)    //设置每一个叶子节点的wp指针
{
    if(!cur->next){
        if(cur->name[0]=='.')
            return ;
        cur->wp = WordsPoint++;
        return ;
    }
    Node* p = cur->next;
    while(p){
        setWP(p);
        p = p->brother;
    }
}

char* getNext(Node* cur,FILE* fo)
{
    //递归出口
    if(!cur->next){
        if(cur->name[0]=='.'
            || cur->name[0]=='('
            || cur->name[0]==')'){    //忽略空. 和 （ 和 ）
            char* t = new char[2];
            t[0]='\0';
            return t;
        }
        return WordList[TokenList_value[cur->wp]];
    }
    if(strcmp(cur->name,"Program")==0){
        fprintf(fo,"%s\r\n",getNext(cur->next,fo));
        getNext(cur->next->brother,fo);
        fprintf(fo,"%s\r\n",getNext(cur->next->brother->brother,fo));
        char* tmp = new char[2];
        tmp[0] = '\0';
        return tmp;
    }
    else if(strcmp(cur->name,"Assign-stmt")==0){
        char* tmp = new char[2];
        tmp[0] = '\0';
        fprintf(fo,"%s=%s\r\n",getNext(cur->next,fo),getNext(cur->next->brother->brother,fo));
        return tmp;
    }
    else if(strcmp(cur->name,"Term")==0){
        char* tmp = new char[150];
        tmp = getNext(cur->next->brother,fo);
        if(tmp[0]=='*' || tmp[0]=='/'){
            fprintf(fo,"t%d=%s%s\r\n",tmpcnt,getNext(cur->next,fo),tmp);
            sprintf(tmp,"t%d",tmpcnt++);
            return tmp;
        }
    }
    else if(strcmp(cur->name,"Expr")==0){
        char* tmp = new char[150];
        tmp = getNext(cur->next->brother,fo);
        if(tmp[0]=='+' || tmp[0]=='-'){
            fprintf(fo,"t%d=%s%s\r\n",tmpcnt,getNext(cur->next,fo),tmp);
            sprintf(tmp,"t%d",tmpcnt++);
            return tmp;
        }
    }
    else if(strcmp(cur->name,"Term'")==0){
        char* tmp = new char[150];
        if(cur->next->brother!=NULL){
            p = cur->next->brother->brother;
            if(p->next->brother!=NULL){
                tmp = getNext(cur->next->brother->brother,fo);
                if(tmp[0]=='*' || tmp[0]=='/'){
                    if(*getNext(cur->next,fo)=='/'){    //如果最前面的是‘-’，后面的运算符应该是反的
                        if(tmp[0]=='/')
                            tmp[0]='*';
                        else
                            tmp[0]='/';
                    }
                    fprintf(fo,"t%d=%s%s\r\n",tmpcnt,getNext(cur->next->brother,fo),tmp);
                    sprintf(tmp,"%st%d",getNext(cur->next,fo),tmpcnt++);
                    return tmp;
                }
            }
        }
    }
    else if(strcmp(cur->name,"Expr'")==0){
        char* tmp = new char[150];
        if(cur->next->brother!=NULL){
            p = cur->next->brother->brother;
            if(p->next->brother!=NULL){
                tmp = getNext(cur->next->brother->brother,fo);
                if(tmp[0]=='+' || tmp[0]=='-'){
                    if(*getNext(cur->next,fo)=='-'){    //如果最前面的是‘-’，后面的运算符应该是反的
                        if(tmp[0]=='-')
                            tmp[0]='+';
                        else
                            tmp[0]='-';
                    }
                    fprintf(fo,"t%d=%s%s\r\n",tmpcnt,getNext(cur->next->brother,fo),tmp);
                    sprintf(tmp,"%st%d",getNext(cur->next,fo),tmpcnt++);
                    return tmp;
                }
            }
        }
    }

    char * s = new char[150];    //new一个字符串，存储这个节点的结果
    memset(s,0,sizeof(s));

    Node* p = cur->next;
    while(p){
        strcat(s,getNext(p,fo));
        p = p->brother;
    }
    return s;
}

void get3AddrCode()
{
    WordsPoint = 1;
    FILE* fo = fopen(".\\out.txt","wb");
    if(fo==NULL){
        printf("三地址代码生成文件 out.txt 打开失败！\n");
        return ;
    }

    setWP(root);
    printf("生成三地址语句......\n");
    getNext(root,fo);
}
//--------------- 生成三地址代码 end ---------------

int main()
{
    bool f=true;
    Init();
    if(!Lex())    //词法分析
        f=false;
    //printTokenList();
    if(!Parse())    //语法分析
        f=false;
    if(f)    //都通过了才能生成三地址代码
        get3AddrCode();    //生成三地址代码
    return 0;
}
