//�ʷ�������' '����հ��ַ����������з����ո��Ʊ��
//Դ�����ʽ��1��һ��ֻ����һ����䣻2�������п�����ע��
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stack>
#include <stdlib.h>
using namespace std;

char src[1000010];    //�洢Դ����
char TokenList_kind[1010][100];    //�ʷ���Ԫ�� ֮ ���
int TokenList_value[1010];    //�ʷ���Ԫ�� ֮ ֵ
char WordList[1010][100];    //���ű� -- ��token_valueָ��
int LineList[1010];        //��¼ÿһ��token��Ӧ������ -- ��token_valueָ��
int TokenListPoint;        //�ʷ���Ԫ��ָ��
int WordListPoint;        //���ű�ָ��

int WordsPoint;        //�﷨����ʱ�Ĵʷ���Ԫ��ָ��

int tmpcnt;        //����ַ����мĴ����ı��

int ExpTable[11][8] = {    //�� �������� ��ȡ���ʽ ��״̬ת����
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
  �ķ�
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
FIRST��
First(Program)={BEGIN}
First(Stmt-List)={ID}
First(Stmt-List')={ ID,��}
First(Stmt)={ ID}
First(Assign-stmt)={ ID}
First(Expr)={(,ID,NUM}
First(Expr')={+,-,��}
First(Term)={(,ID,NUM}
First(Term')={*,/,��}
First(Factor)={(,ID,NUM}
First(Add-Op)={+,-}
First(Multiple-Op)={*,/}
                /*
FOLLOW��
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
  Ԥ�������
���ս����    �������
            BEGIN    ��+��    ��-��    ��*��    ��/��    (    )    ��=��    ID    NUM    END    $
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

char PreTab[12][12][100]={    //�洢Ԥ�������
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

char Product[23][50]={    //��¼����ʽ���ֵ��ַ���
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


//�﷨���ڵ�
struct Node{
    Node(char na[]){    //���캯��
        strcpy(name,na);
        wp = 0;
        brother = NULL;
        next = NULL;
    }
    char name[50];
    int wp;    //�ս����token���е�λ��
    Node* brother;  //ָ����һ���ֵܽڵ�
    Node* next;     //ָ��ǰ�ڵ�ĺ��ӽڵ�
}*root,*now,*p;

void Init()
{
    memset(src,0,sizeof(src));    //��ʼ��Դ�����ַ�����
    memset(TokenList_kind,0,sizeof(TokenList_kind));    //��ʼ���ʷ���Ԫ��
    memset(TokenList_value,0,sizeof(TokenList_value));    //��ʼ���ʷ���Ԫ��
    memset(WordList,0,sizeof(WordList));    //��ʼ�����ű�
    TokenListPoint = 1;
    WordListPoint = 1;

    WordsPoint = 1;

    //��ʼ��ָ��
    root = NULL;
    now = NULL;
    p = NULL;
    tmpcnt = 1;
}

//--------------- �ʷ����� start ---------------
void AddToken(char kind[],char value[],int line)    //��<���ֵ>����token����
{
    strcpy(TokenList_kind[TokenListPoint],kind);
    TokenList_value[TokenListPoint++] = WordListPoint;

    strcpy(WordList[WordListPoint],value);
    LineList[WordListPoint++] = line;
}

void strsub(char str1[],int start,int len,char str2[])    //��ȡstr1�����ַ�����str2
{
    int i=0,j;
    for(j=0;j<len;j++){
        str2[i++] = str1[start+j];
    }
    str2[i] = '\0';
}

void InputString()    //���ļ��ж�ȡԴ���룬ͬʱ����ע�ͣ�һ��ֻ�ܷ���һ�����
{
    //�ļ�����
    FILE* fs = fopen(".\\src.txt","rb");    //Դ����
    if(fs==NULL){
        printf("Դ�����ļ� src.txt ������\n");
        return ;
    }

    char s[10010]={0};
    //fscanf(fs,"%s",src);
    while(fgets(s,10010,fs)){
        if(sscanf(s,"%s",s)==-1){    //û�ж����ַ�������s���
            s[0]='\0';
            continue;
        }
        //����ע��
        if(s[0]=='/' && s[1]=='/')
            continue;
        int i;
        for(i=0;s[i];i++)
            if(s[i]=='/' && s[i+1]=='/')
                break;
        s[i]='\0';

        strcat(src,s);    //���ӵ�Դ�����ַ�����
        strcat(src," ");    //����һ���հ׷�
    }

    int len = strlen(src);
    len--;
    src[len] = '\0';
}

bool getBEGIN(int &i,int &line)    //��ȡ��BEGIN��
{
    char tmp[6];
    strsub(src,i,5,tmp);
    if(strcmp(tmp,"BEGIN")==0){
        i = i+5;
        AddToken("BEGIN","BEGIN",line);    //���token,<���ֵ>
        return true;
    }
    else
        return false;
}
bool getBLANK(int &i)    //��ȡ���հ׷���==>' '
{
    if(src[i]==' '){
        i++;
        return true;
    }
    else
        return false;
}
bool getEND(int &i,int &line)        //��ȡ��END��
{
    char tmp[4];
    strsub(src,i,3,tmp);
    if(strcmp(tmp,"END")==0){
        i = i+3;
        AddToken("END","END",line);    //���token,<���ֵ>
        return true;
    }
    else
        return false;
}
bool getExp(int &i,int &line)        //��ȡ���ʽ������' '����
{
    int status=0;
    char tmp[10010]={0};
    char t[2]={0};
    int j=0;
    stack <char> ss;
    while(status!=8){
        if(status==-1)    //���������״̬�ţ�����false
            return false;
        if(status==9)    //�����˴���״̬������false
            return false;

        //����src[i]ȷ����һ��״̬��ֱ���ִ����״̬ 8
        if( ('a'<=src[i] && src[i]<='z') || ('A'<=src[i] && src[i]<='Z') ){
            //��ȡ����ĸ
            status = ExpTable[status][0];
            tmp[j++] = src[i++];
        }
        else if('0'<=src[i] && src[i]<='9'){
            //��ȡ������
            status = ExpTable[status][1];
            tmp[j++] = src[i++];
        }
        else{
            switch(src[i]){
            case ' ':    //�հ׷�
                status = ExpTable[status][2];
                if(tmp[0]!='\0'){
                    //����Ǻ��������հ׷���˵�����ʽ�ý�����
                    //����ȡ�����һ�����ʷ���token
                    if(j!=0){    //˵��֮ǰ��һ�����ʻ����֣�������һ���������Ѿ������һ�����ʷ���token�����ˣ����Բ���Ҫ������
                        if( ('a'<=tmp[0] && tmp[0]<='z') || ('A'<=tmp[0] && tmp[0]<='Z') )
                            AddToken("ID",tmp,line);    //������ַ�����ĵ��ʷ���token����
                        else if( '0'<=tmp[0] && tmp[0]<='9' )
                            AddToken("NUM",tmp,line);    //������ַ�����ĵ��ʷ���token����
                        j=0;
                    }
                }
                line++;
                i++;
                break;
            case '=':    // =
                status = ExpTable[status][3];

                tmp[j] = '\0';
                AddToken("ID",tmp,line);    //=ǰ��һ���Ǳ�ʶ��������token��
                j=0;
                AddToken("=","=",line);    //����ǰ��=Ҳ����token��

                i++;
                break;
            case '+':    //OP
            case '-':
            case '*':
            case '/':
                status = ExpTable[status][4];

                tmp[j] = '\0';
                if( ('a'<=tmp[0] && tmp[0]<='z') || ('A'<=tmp[0] && tmp[0]<='Z') )    //��������ǰ����ĸ����˵���Ǳ�ʶ��
                    AddToken("ID",tmp,line);
                else if( '0'<=tmp[0] && tmp[0]<='9' )
                    AddToken("NUM",tmp,line);
                j=0;
                t[0] = src[i];
                t[1] = '\0';
                AddToken(t,t,line);    //�����������token��

                i++;
                break;
            case '(':    // (
                status = ExpTable[status][5];

                AddToken("(","(",line);    //��������token��
                ss.push('(');

                i++;
                break;
            case ')':    // )
                status = ExpTable[status][6];

                tmp[j] = '\0';
                if( ('a'<=tmp[0] && tmp[0]<='z') || ('A'<=tmp[0] && tmp[0]<='Z') )    //���)ǰ����ĸ����˵���Ǳ�ʶ������ID
                    AddToken("ID",tmp,line);
                else if( '0'<=tmp[0] && tmp[0]<='9' )
                    AddToken("NUM",tmp,line);
                j=0;

                AddToken(")",")",line);    //��������token��

                if(ss.empty())    //���Ų�ƥ��
                    return false;
                else
                    ss.pop();

                i++;
                break;
            default:    //����
                status = ExpTable[status][7];
                i++;
                break;
            }
        }
    }

    if(status==8){    //��ȷ����
        /*
        if(ss.empty())    //����ƥ��
            return true;
        else
            return false;
        */
        return true;
    }
    else
        return false;
}

bool Lex()    //���дʷ�����
{
    printf("�ʷ�����......\n");
    int i=0;
    int line = 1;
    InputString();
    try{
        getBEGIN(i,line)?1:throw 1;
        getBLANK(i)?line++:throw 2;
        while(1){    //��ȡ���ʽ
            if(src[i]=='\0')    //��û��⵽END������ͽ�����
                throw 3;
            char tmp[4];
            strsub(src,i,3,tmp);    //��ȡ�ַ���
            if(strcmp(tmp,"END")==0){
                //�����⵽END������ѭ��
                break;
            }

            //��ȡ���ʽ
            getExp(i,line)?1:throw 4;
        }
        getEND(i,line)?1:throw 3;
        if(src[i]!='\0')    //���END������־֮����������ţ�˵����������
            throw 5;
    }
    catch(int err){
        printf("���ʷ�����\n");
        //�����к�
        int errline = 1;
        int j;
        for(j=0;j<=i;j++)
            if(src[j]==' ')
                errline++;

        switch(err){
        case 1:
            printf("ERROR 1 (��%d��) : û�ж�ȡ����ʼ��ʶ BEGIN!\n",errline);
            printf("����λ�ã�\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        case 2:
            printf("ERROR 2 (��%d��) : û�ж�ȡ���հ׷�!\n",errline);
            printf("����λ�ã�\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        case 3:
            printf("ERROR 3 (��%d��) : û�ж�ȡ��������ʶ END!\n",errline);
            printf("����λ�ã�\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            printf("\n");
            return false;
        case 4:
            printf("ERROR 4 (��%d��) : ���ʽ�������磺��ʶ������!\n",errline);
            printf("����λ�ã�\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        case 5:
            printf("ERROR 5 (��%d��) : BEGIN...END ����θ�ʽ����!\n",errline);
            printf("����λ�ã�\t%s%s\n",WordList[WordListPoint-2],WordList[WordListPoint-1]);
            printf("\t\t");
            for(j=0;j<strlen(WordList[WordListPoint-2]);j++)
                printf(" ");
            printf("^\n");
            printf("\n");
            return false;
        default:break;
        }
    }
    printf("û�дʷ�����\n");
    printf("\n");
    return true;
}

void printTokenList()    //���token��
{
    int i=1;
    printf("��������\n");
    printf("<\t���\t,ֵ\t>\n");
    for(;i<TokenListPoint;i++){
        printf("<\t%s\t,%s\t>\n",TokenList_kind[i],WordList[TokenList_value[i]]);
    }
    printf("\n");
}
//--------------- �ʷ����� end ---------------


//--------------- �﷨���� start ---------------
//�����﷨������

void OutStack(stack <char*> z)        //���ջ�����ݵ�ǰ����
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

void OutRightStr()    //�����ǰtoken����ǰ����
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

void OutAction(char action[])    //�������
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
    if(f[0]=='\0')    //ȷ���ı�λ��Ϊ��
        return false;
    return true;
}

bool Parse()    //�﷨�����׶Ρ�������token��������������
{
    stack <char*> z;
    stack <Node*> nz;
    z.push(*Product);

    root = new Node("Program");
    now = root;

    printf("�ʷ�����......\n");
    printf("���������̡�\n");
    printf("%22s%24s%22s\n","ջ","����","����");
    char action[100]={0};
    try{
        while(!z.empty() || WordsPoint<WordListPoint){    //ջ�����봮��ֻʣ�������ţ�ջ�� or ָ��'\0'��ʱ�˳�ѭ��

            //�����ǰ�������
            OutStack(z);        //���ջ������
            OutRightStr();    //���ʣ�������ַ���
            OutAction(action);    //�������

            memset(action,0,sizeof(action));

            //Ԥ��������ƥ��e��Ԥ������e��ջ������ָ����ơ�

            //��û����ջ�Ϳ���
            if(z.empty())
                throw 1;    //�Ƿ�����

            if(strcmp(z.top(),TokenList_kind[WordsPoint])==0){    //ջ��Ԫ��==ָ��ָ���ַ���ƥ��ɹ�
                //���嶯�� - �����﷨��
                while(!now->brother){    //�ص�����һ���ֵܽڵ�Ľڵ�Ϊֹ
                    if(nz.empty()){
                        if(now==root)    //����ص��˸��ڵ�
                            goto label;
                        else
                            throw 1;
                    }
                    now = nz.top();
                    nz.pop();
                }
                now = now->brother;

label:
                //׼������ַ��� action
                strcat(action,"ƥ��");
                strcat(action,z.top());

                //��ջ��ָ�����
                if(!z.empty())
                    z.pop();
                if(WordsPoint<WordListPoint)
                    WordsPoint++;
            }
            else{    //����ȣ�����Ƶ�
                //ȷ���Ƶ�
                char f[200]={0};
                if(strcmp(z.top(),"Program")==0){
                    if(!seltab(0,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;    //û���ҵ���Ӧ���Ƶ�
                }
                else if(strcmp(z.top(),"Stmt-List")==0){
                    if(!seltab(1,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;    //û���ҵ���Ӧ���Ƶ�
                }
                else if(strcmp(z.top(),"Stmt-List'")==0){
                    if(!seltab(2,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;    //û���ҵ���Ӧ���Ƶ�
                }
                else if(strcmp(z.top(),"Stmt")==0){
                    if(!seltab(3,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;    //û���ҵ���Ӧ���Ƶ�
                }
                else if(strcmp(z.top(),"Assign-stmt")==0){
                    if(!seltab(4,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;    //û���ҵ���Ӧ���Ƶ�
                }
                else if(strcmp(z.top(),"Expr")==0){
                    if(!seltab(5,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else if(strcmp(z.top(),"Expr'")==0){
                    if(!seltab(6,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else if(strcmp(z.top(),"Term")==0){
                    if(!seltab(7,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else if(strcmp(z.top(),"Term'")==0){
                    if(!seltab(8,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else if(strcmp(z.top(),"Factor")==0){
                    if(!seltab(9,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else if(strcmp(z.top(),"Add-Op")==0){
                    if(!seltab(10,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else if(strcmp(z.top(),"Multiple-Op")==0){
                    if(!seltab(11,f))    //�ӱ���ȷ���Ƶ���
                        throw 2;
                }
                else{
                    throw 2;
                }
                //׼������ַ��� action
                strcat(action,"���");
                strcat(action,z.top());
                strcat(action,"->");
                strcat(action,f);

                //��ջ���ַ�����¼��������
                char proleft[50];
                //��ջ��Ԫ�س�ջ�����Ƶ���ջ
                if(!z.empty()){
                    strcpy(proleft,z.top());
                    z.pop();
                }
                else
                    throw 1;

                char tmp[100];

                //����Ƶ��������ǿգ���f->.����������
                if(f[0]=='.'){
                    nz.push(now);

                    now->next = new Node(".");
                    now = now->next;    //nowָ��ǰ����ʽ�ұߵĵ�һ���ַ���

                    while(!now->brother){    //�ص�����һ���ֵܽڵ�Ľڵ�Ϊֹ
                        if(nz.empty()){
                            if(now==root)    //����ص��˸��ڵ�
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
                stack <char*> tz;    //��ʱ��ջ

                //�������뵽��ʱջ��
                while(sscanf(f,"%s",tmp)!=-1){
                    //���嶯�� - �����﷨��
                    if(strcmp(proleft,now->name)==0){
                        nz.push(now);    //����ǰ�ڵ��¼����

                        now->next = new Node(tmp);
                        now = now->next;    //nowָ��ǰ����ʽ�ұߵĵ�һ���ַ���
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

                //���������������ջ��
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
        if(z.empty() && WordsPoint >= WordListPoint){    //�����˳�ѭ��
            //������һ�з������
            OutStack(z);        //���ջ������
            OutRightStr();    //���ʣ�������ַ���
            OutAction(action);    //�������
        }
        else{    //���������
            throw 1;
        }
    }
    catch(int err){
        printf("\n");
        printf("���﷨����\n");

        switch(err){
        case 1:
            printf("ERROR 1 (��%d��) : �Ƿ�����!\n",LineList[WordsPoint]);
            printf("\n");
            return false;
        case 2:
            printf("ERROR 2 (��%d��) : û���ҵ� %s ��Ӧ���Ƶ�!\n",LineList[WordsPoint],z.top());
            printf("\n");
            return false;
        default:
            break;
        }
    }
    printf("û���﷨����\n");
    printf("\n");
    return true;
}
//--------------- �﷨���� end ---------------

//--------------- ��������ַ���� start ---------------
//���﷨���������к��������ִ���������һ��ɨ��֮��������code��Ϊ����ַ����


void setWP(Node* cur)    //����ÿһ��Ҷ�ӽڵ��wpָ��
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
    //�ݹ����
    if(!cur->next){
        if(cur->name[0]=='.'
            || cur->name[0]=='('
            || cur->name[0]==')'){    //���Կ�. �� �� �� ��
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
                    if(*getNext(cur->next,fo)=='/'){    //�����ǰ����ǡ�-��������������Ӧ���Ƿ���
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
                    if(*getNext(cur->next,fo)=='-'){    //�����ǰ����ǡ�-��������������Ӧ���Ƿ���
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

    char * s = new char[150];    //newһ���ַ������洢����ڵ�Ľ��
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
        printf("����ַ���������ļ� out.txt ��ʧ�ܣ�\n");
        return ;
    }

    setWP(root);
    printf("��������ַ���......\n");
    getNext(root,fo);
}
//--------------- ��������ַ���� end ---------------

int main()
{
    bool f=true;
    Init();
    if(!Lex())    //�ʷ�����
        f=false;
    //printTokenList();
    if(!Parse())    //�﷨����
        f=false;
    if(f)    //��ͨ���˲�����������ַ����
        get3AddrCode();    //��������ַ����
    return 0;
}
