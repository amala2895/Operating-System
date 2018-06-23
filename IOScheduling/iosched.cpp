#include <iostream>
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
#include <list>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility>
#include <queue>
#include <limits>
#include <cstdint>
int sizee;
using namespace std;
ifstream inFile;
int total_time=0, tot_movement=0;
double avg_turnaround=0.0, avg_waittime=0.0;
int max_waittime=0;
int head;

class IO{
 public:

     int at_time;
     int track;
     int start;
     int endd;
     int tat;//turn around time
     int wait;//wait time

     IO(int a_t,int t){

        at_time=a_t;
        track=t;
        start=0;
        endd=0;
        tat=0;//turn around time (endd-at_time)
        wait=0;//wait time (start-at_time)

     }
     void display()
     {
         cout<<"track: "<<track<<" arrival time: "<<at_time<<" "<<start<<" "<<endd<<" "<<tat<<" "<<wait<<endl;
     }



};
IO* CURRENT_IO=NULL;
//Reading file
vector<IO*> instructions;
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
    if(inFile.fail()){
        cerr<<"Unable to open file"<<endl;
        exit(1);
    }


    int time=0, track=0;
    string line;
    while(true){

        line=sendline();
        if (line==""){

            break;}
        stringstream ss(line);
        ss >> time>> track;
        IO* new_i=new IO(time,track);
        instructions.push_back(new_i);


    }

}
//Base class
class IOSched{
    public:
    IOSched(){}
    virtual IO* getIO(){}
    virtual void addIO(IO* n_io){}
};

class FIFO:public IOSched{
    public:
        queue<IO*> fifoqueue;
    FIFO(){

    }

    IO* getIO(){

        if(fifoqueue.empty())
            return NULL;
        IO *i=fifoqueue.front();
        fifoqueue.pop();

        return i;
    }

    void addIO(IO* n_io){

        fifoqueue.push(n_io);

    }
};

class SSTF:public IOSched{
    public:
        vector<IO*> ssq;
    SSTF(){

    }

    IO* getIO(){

        if(ssq.empty())
            return NULL;

        int movement=2147483647;

        int j=0;
        for (int i =0;i<ssq.size();i++){

            if(movement>abs(head-ssq[i]->track))
            {
                movement=abs(head-ssq[i]->track);
                j=i;
            }


        }
        IO* ii=ssq[j];
        ssq.erase(ssq.begin()+j);

        return ii;
    }

    void addIO(IO* n_io){

        ssq.push_back(n_io);

    }
};

class Look:public IOSched{
    public:
        vector<IO*> ssq;
        int direction=1;
    Look(){

    }

    IO* getIO(){

        if(ssq.empty())
            return NULL;

        while(true){

        if(direction==1){
            int movement=2147483647;
            int j=-1;

            for (int i =0;i<ssq.size();i++){
                if(ssq[i]->track-head<0)
                    continue;
                if(movement>ssq[i]->track-head)
                {
                    movement=ssq[i]->track-head;
                    j=i;
                }

            }
            if (j==-1){
                direction=-1;
            }

            else{
                IO* ii=ssq[j];
                ssq.erase(ssq.begin()+j);
                return ii;}


        }
        if(direction==-1){

            int movement=2147483647;
            int j=-1;

            for (int i =0;i<ssq.size();i++){
                if(head-ssq[i]->track<0)
                    continue;
                if(movement>head-ssq[i]->track)
                {
                    movement=head-ssq[i]->track;
                    j=i;
                }

            }
            if (j==-1){
                direction=1;
            }

            else{
                IO* ii=ssq[j];
                ssq.erase(ssq.begin()+j);
                return ii;}

        }
    }

    }

    void addIO(IO* n_io){

        ssq.push_back(n_io);

    }
};


class CLook:public IOSched{
public:
    vector<IO*> ssq;
    int head1;
    CLook(){

    }

    IO* getIO(){

        if(ssq.empty())
        return NULL;
        head1=head;
        while(true){


                int movement=2147483647;
                int j=-1;

                for (int i =0;i<ssq.size();i++){
                    if(ssq[i]->track-head1<0)
                    continue;
                    if(movement>ssq[i]->track-head1)
                    {
                        movement=ssq[i]->track-head1;
                        j=i;
                    }

                }
                if (j==-1){
                    head1=0;
                    continue;
                }

                else{
                    IO* ii=ssq[j];
                    ssq.erase(ssq.begin()+j);
                    return ii;}


        }

    }

    void addIO(IO* n_io){

        ssq.push_back(n_io);

    }
};



class FLook:public IOSched{
    public:
        vector<IO*> *addq_ptr;
        vector<IO*> *retrq_ptr;
        vector<IO*> addq;
        vector<IO*> retrq;
        int direction=1;
    FLook(){
        addq_ptr=&addq;
        retrq_ptr=&retrq;
    }

    IO* getIO(){

        if(addq_ptr->empty() && retrq_ptr->empty())
            return NULL;
        if(retrq_ptr->empty()){
            vector<IO*> *temp=addq_ptr;
            addq_ptr=retrq_ptr;
            retrq_ptr=temp;

        }
        while(true){

        if(direction==1){
            int movement=2147483647;
            int j=-1;

            for (int i =0;i<retrq_ptr->size();i++){
                if((*retrq_ptr)[i]->track-head<0)
                    continue;
                if(movement>(*retrq_ptr)[i]->track-head)
                {
                    movement=(*retrq_ptr)[i]->track-head;
                    j=i;
                }

            }
            if (j==-1){
                direction=-1;
            }

            else{
                IO* ii=(*retrq_ptr)[j];
                retrq_ptr->erase(retrq_ptr->begin()+j);
                return ii;}


        }
        if(direction==-1){

            int movement=2147483647;
            int j=-1;

            for (int i =0;i<retrq_ptr->size();i++){
                if(head-(*retrq_ptr)[i]->track<0)
                    continue;
                if(movement>head-(*retrq_ptr)[i]->track)
                {
                    movement=head-(*retrq_ptr)[i]->track;
                    j=i;
                }

            }
            if (j==-1){
                direction=1;
            }

            else{
                IO* ii=(*retrq_ptr)[j];
                retrq_ptr->erase(retrq_ptr->begin()+j);
                return ii;}

        }
    }

    }

    void addIO(IO* n_io){

        addq_ptr->push_back(n_io);

    }
};




IOSched* THE_SCHED;


void simulation(){

     int timecounter=0;
     int k=0;
     IO *last=instructions[k];

     int flag=0;
     while(true){

        //check if new io arrives at this point
        if(k!=instructions.size())//if no more ios have to be read and put in the queue
        {
        if(instructions[k]->at_time==timecounter){
                //add to the queue
                THE_SCHED->addIO(instructions[k]);
                k++;
            }
        }
        //is an io active
        if(CURRENT_IO!=NULL){
            //a io is active
            if(head!=CURRENT_IO->track)
            {   //head yet not reached track
                if(CURRENT_IO->track-head>0){
                    head++;
                    tot_movement++;
                }
                else{
                    head--;
                    tot_movement++;
                }
            }
            else{//head reahced track
                CURRENT_IO->endd=timecounter;



                sizee--;//decrement from total number of ios to be performed
                CURRENT_IO=NULL;
            }

        }
        if(CURRENT_IO==NULL){
            //cout<<"no current process"<<endl;
            IO *new1=THE_SCHED->getIO();
            //cout<<"got new process"<<endl;
            if(new1==NULL)
            {

                if(sizee!=0)
                    {
                        timecounter++;
                        continue;
                    }
                else//all ios have been performed
                    break;
            }
            CURRENT_IO=new1;

            CURRENT_IO->start=timecounter;
            continue;

        }

        timecounter++;
     }

     total_time=timecounter;


}



int main(int argc, char* argv[])
{
    char *argSched=NULL;
    int c;
    opterr=0;
    while((c=getopt(argc,argv,"s:"))!=-1){
        switch(c)
        {
        case 's':
            argSched=optarg;
            break;

        case '?':
            {
                if (optopt=='s')
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
    }

    string schedulerArg(argSched);
    string InputFile=argv[optind];

    //read file
    readfile(InputFile);
    if(schedulerArg=="i"){
       THE_SCHED=new FIFO();
       }
    if(schedulerArg=="j"){
       THE_SCHED=new SSTF();
       }
    if(schedulerArg=="s"){
       THE_SCHED=new Look();
       }
    if(schedulerArg=="c"){
        THE_SCHED=new CLook();
    }
    if(schedulerArg=="f"){
        THE_SCHED=new FLook();
    }


    sizee=instructions.size();

    simulation();
    max_waittime=-1;

    for(int i=0;i<instructions.size();i++){
        avg_turnaround+=instructions[i]->endd-instructions[i]->at_time;

        avg_waittime+=instructions[i]->start-instructions[i]->at_time;

        if(instructions[i]->start-instructions[i]->at_time>max_waittime)
            max_waittime=instructions[i]->start-instructions[i]->at_time;
        printf("%5d: %5d %5d %5d\n", i, instructions[i]->at_time, instructions[i]->start, instructions[i]->endd);
    }
    avg_turnaround=avg_turnaround/instructions.size();
    avg_waittime=avg_waittime/instructions.size();
    printf("SUM: %d %d %.2lf %.2lf %d\n",total_time, tot_movement, avg_turnaround, avg_waittime, max_waittime);

}
