#include <iostream>
#include <bitset>
#include<fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>

using namespace std;

#define N 32


int num_randoms;
long int *randvals;
ifstream rfile;
int ofs;

ifstream inFile;


int numofproc;//number of procedures
int TOT;//total frames
unsigned long int inst_count=0;//instrunction  count
unsigned long ctx_switches=0;//total context switches
unsigned long long int cost=0;//total cost


//function to return random number
int myrandom(int burst)
{
    if(ofs==num_randoms)
        ofs=0;

    int a=(randvals[ofs] % burst);
    ofs=ofs+1;

    return a;
}

//Page Table
struct PTE  {
     unsigned pageno:6;

     unsigned filemap:1;
     unsigned write_protect:1;
     unsigned pagedout:1;

     unsigned modified:1;
     unsigned referenced:1;
     unsigned valid:1;

     unsigned frame:7;

};

//Virtual Memory Space
class VMA{
public:
    int vid, start,ending,wp,fm;
    VMA()
    {
        vid=0;
    }
    VMA(int v,int s,int e,int w,int f)
    {
        vid=v;
        start=s;
        ending=e;
        wp=w;
        fm=f;
    }
  void display(){
      cout<<"vid " <<vid<<" details "<<start<<" "<<ending<<" "<<wp<<" "<<fm<<endl;
  }

};


class Process{

public:
    int pid, numvmas;
    vector<VMA*> myvmas;
    PTE *pagetable[64];

	unsigned long int segv;
	unsigned long int segprot;
	unsigned long int unmaps;
	unsigned long int maps;
	unsigned long int ins;
	unsigned long int fins;
	unsigned long int outs;
	unsigned long int fouts;
	unsigned long int zeros;
    Process()
    {
        pid=0;

    }
    Process(int p,int n)
    {
        pid=p;
        numvmas=n;
    	segv=0;
 		segprot=0;
	 	unmaps=0;
	 	maps=0;
	 	ins=0;
	 	fins=0;
	 	outs=0;
	 	fouts=0;
	 	zeros=0;


    }
    void createpagetable(){

        for (int i=0;i<64;i++)
        {
          PTE* pp=new PTE;
          pagetable[i]=pp;
          pagetable[i]->pageno=i;
          pagetable[i]->valid=0;
          pagetable[i]->filemap=0;
          pagetable[i]->write_protect=0;
          pagetable[i]->modified=0;
          pagetable[i]->referenced=0;
          pagetable[i]->pagedout=0;
          pagetable[i]->frame=0;

        }
        //Map page table entries with VMAs
        for(int j=0;j<myvmas.size();j++)
        {
            for (int i =myvmas[j]->start;i<=myvmas[j]->ending;i++)
            {
                pagetable[i]->filemap=myvmas[j]->fm;
                pagetable[i]->write_protect=myvmas[j]->wp;
                pagetable[i]->frame=127;
            }
        }
     }
     void display(){
          cout<<"pid "<<pid<<" num of vmas "<<numvmas<<endl;
          for(int i=0;i<myvmas.size();i++)
          {
              myvmas[i]->display();
          }
          cout<<"Pagetable"<<endl;
          for (int i =0;i<64;i++)
          {   cout<<pagetable[i]->pageno;
              cout<<pagetable[i]->filemap;
              cout<<pagetable[i]->write_protect;
              cout<<pagetable[i]->pagedout;
              cout<<pagetable[i]->modified;
              cout<<pagetable[i]->referenced;
              cout<<pagetable[i]->valid;
              cout<<pagetable[i]->frame<<endl;
            }
    }
};
//Process List
vector<Process*> procobjects;

class Frame{
public:
    int frameadd,vpage;
    Process *proc;
    bool used;
    unsigned int age;
    Frame()
    {
        frameadd=-1;
        age=0;
        proc=NULL;
        vpage=-1;
        used=false;

    }
    Frame(int f)
    {
        frameadd=f;
        proc=NULL;
        vpage=-1;
        used=false;
    }
    void display()
    {
        if(!used)
            cout<<"Frame no - "<<frameadd<<" used - "<<used<<endl;
        else
        cout<<"Frame no - "<<frameadd<<" process "<<proc->pid<<" vpage "<<vpage<<" used - "<<used<<endl;
    }
};
//Stores Frames
vector<Frame> FrameTable;

//Used to send next line of input File
string sendline()
{
    string l;
    while(true){
        if(inFile.eof()){
        l="";
        break;}
    getline(inFile,l);
    if(l[0]=='#')
            continue;
    else
        break;

    }
    return l;
}

void readfile(string InputFile)
{

    inFile.open(InputFile);
    int novma;
    int start;
    int ending;
    bool wp;
    bool fm;
    int pid;
    string line;
    line=sendline();
    if (line=="")
        cout<<"end"<<endl;
    stringstream ss(line);
    ss >> numofproc;//first number is number of procedures
    for(int i=0;i<numofproc;i++)
    {   pid=i;
        line=sendline();
        stringstream ss1(line);
        ss1 >> novma;//next number is number of vmas of each process
        Process *p=new Process(pid,novma); //create process
        int vid;
        for(int j=0;j<novma;j++)
        {   vid=j;
            line=sendline();
            stringstream ss2(line);
            ss2 >> start >>ending>>wp>>fm;//next line has details about each vma
            VMA* v=new VMA(vid,start,ending,wp,fm);
            p->myvmas.push_back(v);
        }
        p->createpagetable();
        procobjects.push_back(p);
    }

}

//Base class for Page Replacement Algorithms
class Pager{
    public:
    Pager(){}
    virtual Frame* determine_victim_frame(){}

};

class FIFO:public Pager{
   public:
    queue<Frame*> fifoqueue;
    FIFO(){
        //Push all frames in the fifoqueue at initialization
        for(int i=0;i<FrameTable.size();i++)
        {
            fifoqueue.push(&FrameTable[i]);
        }
    }
    Frame* determine_victim_frame()
    {
        Frame *f=fifoqueue.front();
        fifoqueue.pop();
        fifoqueue.push(f);
        return f;//return first frame in queue and push it at back
    }
};

class Random:public Pager{
  public:
    Random(){}
    Frame* determine_victim_frame()
    {
        int f=myrandom(TOT);
        return &FrameTable[f];//return random frame
    }
};


class Aging:public Pager{
 public:
    Aging(){}
    Frame* determine_victim_frame()
    {
        for(int i=0;i<FrameTable.size();i++){

            Process *pro = FrameTable[i].proc;//process in this frame
            PTE *pt = pro->pagetable[FrameTable[i].vpage];//pointer to page in this frame

            if(pt->referenced==1){
                //right shift and set msb 1 for age of frame
                unsigned int x=1;
                x=x<<31;
                FrameTable[i].age=  (FrameTable[i].age >> 1) | x;
                pt->referenced=0;
            }
            else{
                //right shift age of frame
                unsigned int x=0;
                x=x<<31;
                FrameTable[i].age=(FrameTable[i].age >> 1)|x;
                }

        }
        unsigned int minn=4294967295;
        int j=-1;
        //find frame with smallest age
        for(int i=0;i<FrameTable.size();i++){
            if(FrameTable[i].age<minn){
                minn=FrameTable[i].age;
                j=i;
            }
        }
        return &FrameTable[j];
    }
};


class Clock:public Pager{
  public:
    int lastpage;
    Clock(){
    lastpage=-1;
    }
    Frame* determine_victim_frame()
    {
        while(true){
            if (lastpage==TOT-1)
                lastpage=-1;
            lastpage++;
            Process *pro = FrameTable[lastpage].proc;//process in this frame
            PTE *pt = pro->pagetable[FrameTable[lastpage].vpage];//pointer to page in this frame
            if(pt->referenced==0){
                return &FrameTable[lastpage];}

            else {
                pt->referenced=0;
            }
        }
    }
};


class SecondChance:public Pager{
 public:
    queue<Frame*> fifoqueue;
    SecondChance(){
        for(int i=0;i<FrameTable.size();i++)
        {
            fifoqueue.push(&FrameTable[i]);
        }
    }
    Frame* determine_victim_frame()
    {
        while(true){
            Frame *f=fifoqueue.front();
            PTE *pt = f->proc->pagetable[f->vpage];//pointer to page in this frame
            if(pt->referenced==0){
                fifoqueue.pop();
                fifoqueue.push(f);
                return f;
            }
            pt->referenced=0;//give second chance
            fifoqueue.pop();
            fifoqueue.push(f);
        }
    }
};

class NRU:public Pager{
public:
    int count1;
    NRU(){
        count1=0;
    }
    Frame* determine_victim_frame()
    {

        count1++;
        vector<Frame*> class1;
        vector<Frame*> class2;
        vector<Frame*> class3;
        vector<Frame*> class4;
        for(int i=0;i<FrameTable.size();i++){

            Process *pro = FrameTable[i].proc;//process in this frame
            PTE *pt = pro->pagetable[FrameTable[i].vpage];//pointer to page in this frame
            if(pt->referenced==0 && pt->modified==0)
                class1.push_back(&FrameTable[i]);

            if(pt->referenced==0 && pt->modified==1)
                class2.push_back(&FrameTable[i]);

            if(pt->referenced==1 && pt->modified==0)
                class3.push_back(&FrameTable[i]);

            if(pt->referenced==1 && pt->modified==1)
                class4.push_back(&FrameTable[i]);

            if(count1==10){
                pt->referenced=0;}
        }
        int index=0;
        if(count1==10){
            count1=0;}
        //select frame randomly from least possible class
        if(!class1.empty())
        {
            index=myrandom(class1.size());
            return class1[index];
        }
        else if(!class2.empty())
        {
            index=myrandom(class2.size());
            return class2[index];
        }
        else if(!class3.empty())
        {
            index=myrandom(class3.size());
            return class3[index];
        }
        else
        {
            index=myrandom(class4.size());
            return class4[index];
        }
    }
};


Pager* THE_PAGER;

Frame *allocateframefromefreelist(){
    Frame *f=NULL;
    for(int i=0;i<FrameTable.size();i++){
        if(FrameTable[i].used==true)
            continue;
        else{
            f=&FrameTable[i];
            break;}
    }
    return f;
}

Frame *getFrame(){
Frame *frame =allocateframefromefreelist();

    if(frame==NULL){//call page replacement algorithm as no free frames
        frame=THE_PAGER->determine_victim_frame();}
    return frame;
}

Process* current_process=NULL;

Process* getprocess(int pr){
    return procobjects[pr];
}

//check if page exists (based on eacg vms start and end
bool checkifvpagepresent(int vp){
    for(int i=0;i<current_process->myvmas.size();i++){
        if (vp>=current_process->myvmas[i]->start && vp<=current_process->myvmas[i]->ending)
            return true;
    }
    return false;
}


void Simulation()
{
    char inst;
    string vpp;
    int vp;
    string line;
    unsigned long long int i=0;
    while(true)
    {
        line=sendline();
        if (line=="")
            break;//stop simulation when we reach end of file
        stringstream ss(line);
        ss>>inst>>vpp;
        vp=atoi(&vpp[0]);

        cout<<i<<":"<<" ==> "<<inst<<" "<<vp<<endl;
        i=i+1;
        if(inst=='c')//context switch, get process
            {
            current_process=getprocess(vp);
            ctx_switches++;
            continue;
            }

        if(!checkifvpagepresent(vp))
            {//if page not present
            cout<<"  SEGV"<<endl;
            current_process->segv++;
            continue;
            }

        PTE *pt=current_process->pagetable[vp];
        //if page table not yet in frames
        if(pt->valid==0){
            Frame* newframe=getFrame();
            if(newframe->used)
            {
                cout<<" UNMAP "<<newframe->proc->pid<<":"<<newframe->vpage<<endl;
                PTE *oldpt=newframe->proc->pagetable[newframe->vpage];
                newframe->proc->unmaps++;
                //make changes in old page which is unmapped
                if(oldpt->modified==1){
                    if(oldpt->filemap==1)
                    {
                        cout<<" FOUT"<<endl;
                        newframe->proc->fouts++;
                    }
                    else{
                        cout<<" OUT"<<endl;
                        newframe->proc->outs++;
                        oldpt->pagedout=1;
                    }
                }
                oldpt->referenced=0;
                oldpt->modified=0;
                oldpt->valid=0;
                oldpt->frame=127;
                newframe->age=0;
            }
            //allocate frame to cuurent page
            newframe->used=true;
            newframe->proc=current_process;
            newframe->vpage=pt->pageno;
            pt->frame=newframe->frameadd;
            pt->valid=1;
            if(pt->pagedout==0 && pt->filemap==0)
            {
                cout<<" ZERO"<<endl;
                current_process->zeros++;

            }
            if(pt->filemap==1)
            {
              cout<<" FIN"<<endl;
              current_process->fins++;
            }

             if(pt->pagedout==1){
                cout<<" IN"<<endl;
                current_process->ins++;
             }
            cout<<" MAP "<<newframe->frameadd<<endl;
            current_process->maps++;

       }//end of page was not valid
        //if instruction is r or w
        if(inst=='r')
            pt->referenced=1;
        if(inst=='w'){
            if(pt->write_protect==1){
                cout<<"  SEGPROT"<<endl;
                pt->referenced=1;
                current_process->segprot++;
            }
            else{
                pt->referenced=1;
                pt->modified=1;}
        }

    }
    inst_count=i;
}//end of simulation


int main(int argc, char* argv[])

{
    //using getopt to read arguments
    char* Output = NULL;
    char* FrameNumber = NULL;
    char* Algorithm = NULL;
    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "a:o:f:")) != -1)
    switch (c)
    {
        case 'o':
            {
                Output = optarg;
                break;
            }
        case 'f':
            {
                FrameNumber= optarg;
                break;
            }
        case 'a':
            {
                Algorithm = optarg;
                break;
            }
        case '?':
            {
                if (optopt == 'a' || optopt == 'o' || optopt == 'f')
                {
                    std::fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (std::isprint(optopt))
                {
                    std::fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    std::fprintf(stderr, "Unknown option character `\\x%x'.\n",optopt);
                }
                return 1;
            }
          default:
                exit(1);
    }
    string Algo(Algorithm);
    string Out(Output);
    string FN(FrameNumber);
    string InputFile = argv[optind];
    string RandFile = argv[optind + 1];
    stringstream numframes(FN);
    rfile.open(RandFile);

    if (rfile.fail()) {
        cerr << "Unable to open file for reading." << endl;
            exit(1);
    }
    //reading random number file
    rfile >> num_randoms;
    randvals = new long int[num_randoms];

    for (int i=0;i<num_randoms;i++){
            rfile >> randvals[i];
        }
    if (rfile.is_open())
        rfile.close();
    ofs=0;

    numframes>>TOT;
    //create frame table
    for(int i=0;i<TOT;i++)
    {
        Frame f = Frame(i);
        FrameTable.push_back(f);
    }
    //Initializing class as per algorithm
    if(Algo=="f")
        THE_PAGER=new FIFO();
    else if(Algo=="r")
        THE_PAGER=new Random();
    else if(Algo=="s")
        THE_PAGER=new SecondChance();
    else if(Algo=="c")
        THE_PAGER=new Clock();
    else if(Algo=="n")
        THE_PAGER=new NRU();
    else
        THE_PAGER=new Aging();

    readfile(InputFile);

    Simulation();

    inFile.close();
    //Print Summary

    for(int i=0;i<procobjects.size();i++){
        cout<<"PT["<<i<<"]:";
        //Print Page Table of each Process
        for(int j =0;j<64;j++)
        {
            if(procobjects[i]->pagetable[j]->valid==0)
            {
                if(procobjects[i]->pagetable[j]->pagedout==0)
                    cout<<" *";
                else
                    cout<<" #";
            }
            else{
                cout<<" "<<procobjects[i]->pagetable[j]->pageno<<":";
                if(procobjects[i]->pagetable[j]->referenced==1)
                    cout<<"R";
                else
                    cout<<"-";
                if(procobjects[i]->pagetable[j]->modified==1)
                    cout<<"M";
                else
                    cout<<"-";
                if(procobjects[i]->pagetable[j]->pagedout==1)
                    cout<<"S";
                else
                    cout<<"-";
            }

        }
        cout<<endl;
    }
    //Print Frame Table
    cout<<"FT:";
    for(int i=0;i<FrameTable.size();i++){
            if(FrameTable[i].proc==NULL)
            {
                cout<<" *";
            }
            else
                cout<<" "<<FrameTable[i].proc->pid<<":"<<FrameTable[i].vpage;
    }

    cout<<endl;
    //Print Statistics
    for(int i=0;i<procobjects.size();i++){
        cost=cost+400*(procobjects[i]->unmaps+procobjects[i]->maps)+3000*(procobjects[i]->ins+procobjects[i]->outs)+2500*(procobjects[i]->fins +procobjects[i]->fouts)+150*(procobjects[i]->zeros)+300*(procobjects[i]->segprot)+240*(procobjects[i]->segv);

        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n", procobjects[i]->pid, procobjects[i]->unmaps, procobjects[i]->maps, procobjects[i]->ins, procobjects[i]->outs, procobjects[i]->fins, procobjects[i]->fouts, procobjects[i]->zeros, procobjects[i]->segv, procobjects[i]->segprot);
    }
    cost=cost+inst_count-ctx_switches+(ctx_switches*121);
    printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, cost);
    return 0;
}
