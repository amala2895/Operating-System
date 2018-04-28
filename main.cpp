#include<iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cctype>
#include <map>
#include <stack>
#include <set>
#include <vector>
#include <cstdlib>


using namespace std;
ifstream inFile;
int lineNumber;
int offset;

map<string,int> SymbolTable;//to store symbol and value
set<string> err;//to store symbols defined multiple times
set<string> used;//to store symbols used
vector <string> finalsymbols;//to store symbols in order
map<string,int> SymbolModule;//for warning rule 4

void initFile(char *filename)
{   lineNumber=0;
    inFile.open(filename);
        if (inFile.fail()) {
        cerr << "Unable to open file for reading." << endl;
            exit(1);
    }
  }





char* ReadToken()
{
// check whether I need a new line
  static bool needNewLine=true;
  static char linebuffer[2048];
  static string line;
  char* token;
  while (1) {
  if (needNewLine) {

    if (getline(inFile,line)) {
            lineNumber++;
            offset=1;
            needNewLine = false;
            strcpy(linebuffer, line.c_str());
            token = strtok(linebuffer,"\t\n ");

    }
    else{
        return NULL;
    }
  }
  else {
    token = strtok(NULL,"\t\n ");

  }
  if (!token) {
    needNewLine = true;
    continue;
  }
  offset=token-linebuffer+1;


  // I have a token;
return token;
}
}//end readtoken

void parseerror(int errcode) {
 const char* errstr[] = {
 "NUM_EXPECTED",
 "SYM_EXPECTED",
 "ADDR_EXPECTED",
 "SYM_TOO_LONG",
 "TOO_MANY_DEF_IN_MODULE",
 "TOO_MANY_USE_IN_MODULE",
 "TOO_MANY_INSTR"};
 cout<<"Parse Error line "<<lineNumber<<" offset "<<offset<<": "<< errstr[errcode]<<endl;
}

int ReadNumber(char* tok){

    int n;
    if(tok!= NULL)
    {   int flag=0;
        string str(tok);
        for (int i = 0; str[i] != '\0'; i++){
           if(!isdigit(str[i]))
           {
              flag=1;
              break;
           }
         }
     if(flag==0)
     {
         stringstream sstr(str);
         sstr>>n;
         return n;
     }
     else
     {
         //cout <<lineNumber << " " << offset << " : " << tok<< endl;
         //cout<<"error not a number"<<endl;
         parseerror(0);
         exit(1);
     }
    }
    else{
       //cout<<"end of file num expected"<<endl;
       parseerror(0);
        exit(1);
    }
}



char* ReadSymbol(char* tok){
    int n;
    if(tok!= NULL)
    {   int flag=0;
        string str(tok);
        if(!isalpha(str[0]))
     {
         //cout <<lineNumber << " " << offset << " : " << tok << endl;
        //cout<<"error symbol does not start with alphabet"<<endl;
        parseerror(1);
         exit(1);
     }
     if(str.length()>16){
           // cout<< lineNumber << " " << offset << " : " << tok << endl;
                //cout<<"length of symbol greater than 16 error"<<endl;
                parseerror(3);
                exit(1);
            }
        for (int i = 1; str[i] != '\0'; i++){
           if(isdigit(str[i]) || isalpha(str[i])){}
           else{
                flag=1;}
     }
     if(flag==0)
     {
         return tok;
     }
     else
     {
         //cout << lineNumber << " " << offset << " : " << tok << endl;
         //cout<<"error symbol does not contain all alpha numeric"<<endl;
         parseerror(1);
         exit(1);
     }
    }
    else{
        //cout<<"end of file sym expected"<<endl;
        parseerror(1);
        exit(1);
    }
}




char ReadAIER(char* tok){
    int n;
    if(tok!= NULL)
    {
        string str(tok);

     if(str.length()>1){
           // cout<< lineNumber << " " << offset << " : " << tok << endl;
                //cout<<"Not an instruction error"<<endl;
                parseerror(2);
                exit(1);
            }
     char i=str[0];
     if(i=='A' || i=='I'||i=='E'||i=='R')
     {
         return i;
     }
     else
     {
       //  cout << lineNumber << " " << offset << " : " << tok << endl;
         //cout<<"not correct instruction"<<endl;
         parseerror(2);
         exit(1);
     }
    }
    else{
        //cout<<"end of file instr expected"<<endl;
        parseerror(2);
        exit(1);
    }
}



void pass1(){

char *tok;
int module=0;//no of modules
int ba=0;
int tot_instr=0;

  while((tok=ReadToken())!=NULL){

        stack<string> temp;//store symbols temp per module
        module++;
        //store total instructions
       // cout<<module<<" module ba: "<<ba<<endl;
        //read def no
        int defno=ReadNumber(tok);
        //cout <<"defno :"<< lineNumber << " " << offset << " : " << defno << endl;
        if(defno>16){
            //cout<<"error def no greater than 16";parseerror(2);
            parseerror(4);
            exit(1);
        }
        offset=offset+strlen(tok);
        //read symbol value pairs
        for(int i=1;i<=defno;i++)
        {
            tok=ReadToken();
            char *symbol=ReadSymbol(tok);
            //cout <<"symbol :"<< lineNumber << " " << offset << " : " << symbol << endl;
            string sym(symbol);
            offset=offset+strlen(tok);

            int value;
            tok=ReadToken();
            value=ReadNumber(tok);
            //cout <<"value :"<< lineNumber << " " << offset << " : " << value << endl;
             offset=offset+strlen(tok);

            //check if already present in symbol table or not
            if(SymbolTable.find(sym)==SymbolTable.end())
            {
              SymbolTable.insert(pair<string,int>(sym,(value+ba)));
              //cout<<"entered in symbol table "<<sym<<" "<<(value)<<endl;
              temp.push(sym);
              finalsymbols.push_back(sym);
            }
            else{
                    //cout <<"SYMBOL TABLE ENTRY: "<< sym <<" "<< SymbolTable[sym] << "  symbol already defined"<<endl;
                    err.insert(sym);
            }
        }


       // read use no
        tok=ReadToken();
        int useno=ReadNumber(tok);
       // cout <<"useno :"<< lineNumber << " " << offset << " : " << useno << endl;
        if(useno>16){
          parseerror(5);
            //cout<<"error use no greater than 16";
            exit(1);
        }
       offset=offset+strlen(tok);

        // read use list
        for(int i=1;i<=useno;i++)
        {
          tok=ReadToken();
            char *symbol=ReadSymbol(tok);
            //cout <<"symbol :"<< lineNumber << " " << offset << " : " << symbol << endl;
            offset=offset+strlen(tok);

        }


        //read instructions
        tok=ReadToken();
        int instrno=ReadNumber(tok);
        tot_instr=tot_instr+instrno;

        //cout <<"instrno :"<< lineNumber << " " << offset << " : " << instrno << endl;
        if(tot_instr>512){
          parseerror(6);
          exit(1);
        }

    offset=offset+strlen(tok);

        for(int i=1;i<=instrno;i++)
        {   tok=ReadToken();
            char instr=ReadAIER(tok);
            //cout <<"instruction :"<< lineNumber << " " << offset << " : " << instr << endl;
            offset=offset+strlen(tok);

            tok=ReadToken();
            int value=ReadNumber(tok);
            //cout <<"value :"<< lineNumber << " " << offset << " : " << value << endl;
            offset=offset+strlen(tok);

        }

        //checking if any symbol of this module had value greater than instructions
        while(!temp.empty())
        {
            string str=temp.top();
            int a=SymbolTable[str]-ba;
            if(a>(instrno-1))
            {
              SymbolTable[str]=ba;
              cout<<"Warning: Module "<<module<<": "<< str <<" too big "<< a<<" (max="<<(instrno-1)<<") assume zero relative"<<endl;
            }
            temp.pop();
        }


    ba=ba+instrno;

  }


}// end of pass1



string output(int a)
{
    string send;
    stringstream ss;
    ss << a;
    string str = ss.str();
   if(a<10)
   {
       //send=to_string(a);
       stringstream ss;
       ss << a;
       string str = ss.str();
       send="000"+str;
   }
   else if(a>=10 && a<100){
        //send=to_string(a);
        stringstream ss;
       ss << a;
       string str = ss.str();
        send="00"+str;

   }
   else if(a>=100 && a<1000){
       //send=to_string(a);
       stringstream ss;
       ss << a;
       string str = ss.str();
       send="0"+str;

   }
   else
   {

     //send=to_string(a);
     stringstream ss;
       ss << a;
       string str = ss.str();
       send=str;
   }

   return send;
}



















void pass2(){
    cout<<"Memory Map"<<endl;

char *tok;
int module=0;//no of modules
int ba=0;
int tot_instr=0;
int pc=0;
  while((tok=ReadToken())!=NULL){

        set<string> usedtemp;//store symbols with external reference

        string temp[16];//store use symbols temp per module

        for( int i = 0; i < 16; ++i )
        {
            temp[i] = "0";
        }
        module++;
        //store total instructions
        //cout<<module<<" module ba: "<<ba<<endl;
        //read def no
        int defno=ReadNumber(tok);
        //cout <<"defno :"<< lineNumber << " " << offset << " : " << defno << endl;
        if(defno>16){
            //cout<<"error def no greater than 16";parseerror(2);
            parseerror(4);
            exit(1);
        }
        offset=offset+strlen(tok);
        //read symbol value pairs
        for(int i=1;i<=defno;i++)
        {
            tok=ReadToken();
            char *symbol=ReadSymbol(tok);
            //cout <<"symbol :"<< lineNumber << " " << offset << " : " << symbol << endl;
            string sym(symbol);
            offset=offset+strlen(tok);

            int value;
            tok=ReadToken();
            value=ReadNumber(tok);
            //cout <<"value :"<< lineNumber << " " << offset << " : " << value << endl;
             offset=offset+strlen(tok);

             SymbolModule.insert(pair<string,int>(sym,module));


        }


       // read use no
        tok=ReadToken();
        int useno=ReadNumber(tok);
        //cout <<"useno :"<< lineNumber << " " << offset << " : " << useno << endl;
        if(useno>16){
          parseerror(5);
            //cout<<"error use no greater than 16";
            exit(1);
        }
       offset=offset+strlen(tok);

        // read use list
        for(int i=1;i<=useno;i++)
        {
            tok=ReadToken();
            char *symbol=ReadSymbol(tok);
            //cout <<"symbol :"<< lineNumber << " " << offset << " : " << symbol << endl;
            offset=offset+strlen(tok);
            string use(tok);
            temp[i-1]=use;
            used.insert(use);

        }


        //read instructions
        tok=ReadToken();
        int instrno=ReadNumber(tok);
        tot_instr=tot_instr+instrno;

        //cout <<"instrno :"<< lineNumber << " " << offset << " : " << instrno << endl;
        if(tot_instr>512){
          parseerror(6);
          exit(1);
        }

    offset=offset+strlen(tok);

        for(int i=1;i<=instrno;i++)
        {   tok=ReadToken();
            char instr=ReadAIER(tok);
            //cout <<"instruction :"<< lineNumber << " " << offset << " : " << instr << endl;
            offset=offset+strlen(tok);

            tok=ReadToken();
            int value=ReadNumber(tok);
            //cout <<"value :"<< lineNumber << " " << offset << " : " << value << endl;
            offset=offset+strlen(tok);

            //imediate instruction
            if (instr=='I'){
                if(value>=10000){
                        value=9999;

                        if(pc<10)
                   cout<<"00"<<pc<<": "<<value<<" Error: Illegal immediate value; treated as 9999"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value<<" Error: Illegal immediate value; treated as 9999"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value<<" Error: Illegal immediate value; treated as 9999"<<endl;

                }
                else{
                        string value1=output(value);

                 if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<endl;
                }
            }


            //absolute instruction
             if (instr=='A'){

                    if(value>=10000)
                    {
                    value=9999;

                        if(pc<10)
                   cout<<"00"<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                    }
                    else{
                        int op1=value%1000;
                        int op2=value/1000;
                        if (op1>512)
                        {
                            value=op2*1000;
                            string value1=output(value);
                            if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<" Error: Absolute address exceeds machine size; zero used"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<" Error: Absolute address exceeds machine size; zero used"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<" Error: Absolute address exceeds machine size; zero used"<<endl;

                        }
                        else
                        {
                            string value1=output(value);
                            if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<endl;

                        }
                    }
             }




            //relative instruction

            if (instr=='R'){
                if(value>=10000){
                        value=9999;

                        if(pc<10)
                   cout<<"00"<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;

                }
                else{
                 int op1=value/1000;
                 int op2=value%1000;
                 //cout<<op2<<endl;
                 if(op2>instrno)
                 {
                     value=op1*1000+ba;
                     string value1=output(value);
                     if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<" Error: Relative address exceeds module size; zero used"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<" Error: Relative address exceeds module size; zero used"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<" Error: Relative address exceeds module size; zero used"<<endl;

                 }
                 else{
                        value=op1*1000+op2+ba;
                        string value1=output(value);
                 if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<endl; }
                }
            }

            //external instruction


            if(instr=='E'){
                if(value>=10000)
                {
                    value=9999;

                        if(pc<10)
                   cout<<"00"<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value<<" Error: Illegal opcode; treated as 9999"<<endl;
                }
                else
                {
                    int op1=value%1000;
                    int op2=value/1000;

                    if(op1>useno-1){
                            string value1=output(value);

                            if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<" Error: External address exceeds length of uselist; treated as immediate"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<" Error: External address exceeds length of uselist; treated as immediate"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<" Error: External address exceeds length of uselist; treated as immediate"<<endl;

                    }

                    else
                    {
                        usedtemp.insert(temp[op1]);
                        if(SymbolTable.find(temp[op1])!=SymbolTable.end())//found the symbol in symbol tabel
                        {
                            value=op2*1000 +SymbolTable[temp[op1]];
                            string value1=output(value);
                            if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<endl;

                   //temp[op1]="0";

                        }

                        else //symbol not found
                        {
                            value=op2*1000;
                            string value1=output(value);
                            if(pc<10)
                   cout<<"00"<<pc<<": "<<value1<<" Error: "<<temp[op1]<<" is not defined; zero used"<<endl;
                 if(pc>=10 && pc<100)
                   cout<<"0"<<pc<<": "<<value1<<" Error: "<<temp[op1]<<" is not defined; zero used"<<endl;
                 if(pc>=100 && pc<=512)
                   cout<<pc<<": "<<value1<<" Error: "<<temp[op1]<<" is not defined; zero used"<<endl;

                        }

                    }

                }
            }





            pc=pc+1;

        }//end of for reading instr and  values
   //(SymbolTable.find(temp[j])!=SymbolTable.end())
    for(int j=0;j<16;j++)
    {
        if(usedtemp.find(temp[j]) == usedtemp.end() && temp[j]!="0" )
            cout<<"Warning: Module "<<module<<": "<<temp[j]<<" appeared in the uselist but was not actually used"<<endl;
    }

    ba=ba+instrno;

  }


}// end of pass2







int main(int argc, char* argv[])
{

    //cout<<"HiHo, off to work we go"<<endl;
    //char *filename="/Users/amaladeshpande/Downloads/lab1samples/input-20";

    initFile(argv[1]);
    pass1();
    cout<<"Symbol Table"<<endl;
    for (int i=0; i<finalsymbols.size(); i++){
      if(err.find(finalsymbols[i]) != err.end()) //Error
      cout<<finalsymbols[i]<<"="<<SymbolTable[finalsymbols[i]]<<" Error: This variable is multiple times defined; first value used"<<endl;
      else
      cout<<finalsymbols[i]<<"="<<SymbolTable[finalsymbols[i]]<<endl;
       }
//cout<<"pass 2"<<endl;
inFile.close();
initFile(argv[1]);
cout<<endl;
    pass2();
cout<<endl;
map<string, int>::iterator it;

for ( it = SymbolTable.begin(); it != SymbolTable.end(); it++ )
{
    if(used.find(it->first) == used.end())//not used
        cout<<"Warning: Module "<<SymbolModule[it->first]<<": "<< it->first <<" was defined but never used"<<endl;
}

      return(0);

  }
