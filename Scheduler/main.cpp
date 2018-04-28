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

using namespace std;

ifstream inFile;
enum state {STATE_CREATED, STATE_READY, STATE_RUNNING, STATE_BLOCKED };
int num_randoms;
bool CALL_SCHEDULER;
int CURRENT_TIME;
long int *randvals;
int ofs;
int lastft;
int cpu=0;
int iou=0;

char *ErrorTypes[] =
{
    "CREATED",
    "READY",
    "RUNNG",
    "BLOCK",

};



int myrandom(int burst)
{
    if(ofs==num_randoms)
        ofs=0;

    int a=1 + (randvals[ofs] % burst);
    ofs=ofs+1;

    return a;
}
//each process stores arriavl time, total cpu time, total cpu burst time, total io time, static priority, dynamic priority
class Process {
  public:
    int a_t, t_c,c_b,i_o,pid,state_ts,timeInPrevState,prio,ft,tt,it,cw,d_prio;
    int cb,rcb,ib;//current cpu burst, remaning cpu time, current io burst
    state state_c;//current state

    Process(){
          a_t=0, t_c=0,c_b=0,i_o=0;
          pid=0;
          state_c=STATE_CREATED;
          state_ts=0;
          timeInPrevState=0;
          prio=0;
          cw=0;
    }

    Process(int x, int y, int z, int w,int l) {
        a_t=x, t_c=y,c_b=z,i_o=w;
        pid=l;
        state_ts=a_t;
        state_c=STATE_CREATED;
        cb= 0;
        rcb=t_c;
        ib=  0;
        prio=myrandom(4);
        d_prio=prio-1;
        timeInPrevState=0;
        it=0;
        cw=0;
        ft=0;
        tt=0;
  }

    void display(){
        cout<<"pid "<<pid<<" at "<<a_t<<" tc "<<t_c<<" cb "<<c_b<<" io "<<i_o<<" ft:"<<ft<<" tt:"<<tt<<" it:"<<it<<" cw:"<<cw<<endl;}

    void displayf(){
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",pid,a_t,t_c,c_b,i_o,prio,ft,tt,it,cw);}
};

Process* CURRENT_RUNNING_PROCESS=NULL;

class Events{

public:
    int timestamp;
    Process *proc;
    state newstate;

    Events(){
        timestamp=0;
        Process p=Process();
        proc= &p;
        newstate=STATE_READY;
    }
    Events(int t, Process *pp, state n){
        timestamp=t;
        proc= pp;
        newstate=n;
    }


    void displayEvent(){

        cout<<timestamp<<" ";
        cout<<proc->pid<<" ";
        cout<<proc->timeInPrevState<<": ";
        cout<<ErrorTypes[proc->state_c]<<" -> ";
        cout<<ErrorTypes[newstate];

    }
};


vector<Process> proc_objects;
list<Events*> eventq;

void displayeventq()
{
    cout<<"eventq"<<endl;
    list<Events*>::iterator itt;
        for (itt = eventq.begin(); itt!=eventq.end(); itt++) {
        (*itt)->displayEvent();}

}

void insertev(Events* ev)
{
    list<Events*>::iterator it;

    int tm=ev->timestamp;
    if(eventq.empty()){
        eventq.push_back(ev);
    }
    else{
        //push event at the right position
        int flag=0;
        for (it = eventq.begin(); it!=eventq.end(); it++) {
            if ((*it)->timestamp>tm)
            {
                eventq.insert(it,ev);
                flag=1;
                break;
            }

        }
        if(flag==0)
        {
            eventq.push_back(ev);
        }

    }

}
class Scheduler{

public:

    Scheduler(){}
    virtual void add_process(Process* p)
    {

    }
    virtual Process* get_next_process()
    {

    }
    virtual void displayrunq(){}
    virtual int getquantum(){}
};

class FCFS:public Scheduler
{
  public:
    list<Process*> runq;
    int quantum;
    FCFS(int q){quantum=q;}

    int getquantum(){
        return quantum;
    }
    //add process at the end
    void add_process(Process* p)
    {
        runq.push_back(p);
    }
    //return the first process in the list and remove it from the list
    Process* get_next_process()
    {
        if(runq.empty())
            return NULL;
        Process* p1=runq.front();
        runq.pop_front();

        return p1;

    }
    void displayrunq()
    {
        list<Process*>::iterator itt;
        for (itt = runq.begin(); itt!=runq.end(); itt++) {
        cout<<(*itt)->pid<<endl;
      }

    }
};


class LCFS:public Scheduler
{
    public:
    list<Process*> runq;
    int quantum;
    LCFS(int q){quantum=q;}

    int getquantum(){
        return quantum;
    }
    //add proces at the end
    void add_process(Process* p)
    {
        runq.push_back(p);
    }
    //return the last process in list and remove the process
    Process* get_next_process()
    {
        if(runq.empty())
            return NULL;
        Process* p1=runq.back();
        runq.pop_back();

        return p1;

    }
    void displayrunq()
    {
        list<Process*>::iterator itt;
        for (itt = runq.begin(); itt!=runq.end(); itt++) {
        cout<<(*itt)->pid<<endl;
      }

    }
};

class SJF:public Scheduler
{
    public:
    list<Process*> runq;
    int quantum;

    SJF(int q){quantum=q;}

    int getquantum(){
        return quantum;
    }
    //add process based on remaining cpu burst
    void add_process(Process* p)
    {   int flag=0;
        list<Process*>::iterator itt;
        if(runq.empty())
            runq.push_back(p);
        else{
            for (itt = runq.begin(); itt!=runq.end(); itt++) {

                if((*itt)->rcb > p->rcb){
                        flag=1;

                runq.insert(itt,p);
                break;}

        }
        if (flag==0){
            runq.push_back(p);
        }

        }
    }
    //return process at the front of list
    Process* get_next_process()
    {
        if(runq.empty())
            return NULL;
        Process* p1=runq.front();
        runq.pop_front();

        return p1;

    }
    void displayrunq()
    {
        list<Process*>::iterator itt;
        for (itt = runq.begin(); itt!=runq.end(); itt++) {
        cout<<(*itt)->pid<<endl;
      }

    }
};

//Round Robin
class RR:public Scheduler
{
    public:
    list<Process*> runq;
    int quantum;
    RR(int q){quantum=q;}

    int getquantum(){
    return quantum;
    }
    void add_process(Process* p)
    {
        runq.push_back(p);
    }
    Process* get_next_process()
    {
        if(runq.empty())
            return NULL;
        Process* p1=runq.front();
        runq.pop_front();

        return p1;

    }
    void displayrunq()
    {
        list<Process*>::iterator itt;
        for (itt = runq.begin(); itt!=runq.end(); itt++) {
        cout<<(*itt)->pid<<endl;
      }

    }
};

class Priority:public Scheduler
{
  public:
    list<Process*> *activeq;
    list<Process*> *expiredq;

    list<Process*> active[4];
    list<Process*> expired[4];

    int quantum;

    Priority(int q){quantum=q;
    activeq=active;
    expiredq=expired;
    }
    int getquantum(){
        return quantum;
    }
    void add_process(Process* p)
    {
       //if dybnamic priority -1 then put in expired queue else in active queue
        //put at the level of dynamic priority
        if(p->d_prio==-1){
            p->d_prio=p->prio-1;//reinitialize dynamic priority
            expiredq[p->d_prio].push_back(p);}
        else
            activeq[p->d_prio].push_back(p);

    }
    Process* get_next_process()
    {   int i=0;
        for(i=3;i>=0;i--)
        {
            //check all levels of active queue
            if(activeq[i].empty())
                continue;
            //return process at front of first active queue level that is not empty
            Process* p1=activeq[i].front();
            activeq[i].pop_front();
            return p1;
        }
        //if all levels of active queue empty

        list<Process*> *temp=activeq;
        //swap active queue and expired queue
        activeq=expiredq;
        expiredq=temp;
        //Check Again in active queue
        for( i=3;i>=0;i--)
        {

            if(activeq[i].empty())
                continue;
            Process* p1=activeq[i].front();
            activeq[i].pop_front();
             return p1;
        }
        return NULL;




    }
    void displayrunq()
    {   cout<<"expired"<<endl;
        list<Process*>::iterator itt;
        for (int i=0;i<4;i++){

        for (itt = expiredq[i].begin(); itt!=expiredq[i].end(); itt++) {
                cout<<i<<endl;
        cout<<(*itt)->pid<<endl;
      }}
      cout<<"active"<<endl;

        for (int i=0;i<4;i++){

        for (itt = activeq[i].begin(); itt!=activeq[i].end(); itt++) {
                cout<<i<<endl;
        cout<<(*itt)->pid<<endl;
      }}



    }
};

Scheduler* THE_SCHEDULER;


Events* getevent(){
    if (eventq.empty())
        return NULL;
    Events* e1=eventq.front();
    eventq.pop_front();
    return e1;

}

int get_next_event_time(){
    if(eventq.empty())
       return -1;
    Events* e1=eventq.front();
    return e1->timestamp;

}
void Simulation(){
    CALL_SCHEDULER=false;
    //cout<<"Simulation Starts"<<endl;
    Events* evt;
    int counter=0;//counts process in blocked
    int iobusy;
    //get next event
    while(evt=getevent()){

        Process *proc1=evt->proc;//process associated with event
        CURRENT_TIME=evt->timestamp;
        proc1->timeInPrevState=CURRENT_TIME-proc1->state_ts;


        if(proc1->state_c==STATE_RUNNING){

            cpu=cpu+proc1->timeInPrevState;

            }

        Events* e;
        //evt->displayEvent();
        switch(evt->newstate){
            case STATE_READY://to ready
                if(proc1->state_c==STATE_RUNNING)
                {//preempt case2


                CALL_SCHEDULER=true;
                CURRENT_RUNNING_PROCESS=NULL;
                proc1->rcb=proc1->rcb-proc1->timeInPrevState;
                proc1->cb=proc1->cb-proc1->timeInPrevState;
                 //cout<<" cb="<<proc1->cb<<" rem="<<proc1->rcb<<" prio="<<proc1->d_prio;
                proc1->d_prio=proc1->d_prio-1;

                proc1->state_ts=CURRENT_TIME;
                proc1->state_c=STATE_READY;
                    //add to run q
                THE_SCHEDULER->add_process(proc1);
                break;
                }

                if(proc1->state_c==STATE_BLOCKED)
                    //decrease priority level
                {   proc1->d_prio=proc1->prio-1;
                    counter--;
                    if(counter==0)
                        iou=iou+(CURRENT_TIME-iobusy);

                }
                proc1->state_ts=CURRENT_TIME;
                proc1->state_c=STATE_READY;
                //add to run q
                THE_SCHEDULER->add_process(proc1);

                CALL_SCHEDULER=true;
                break;

            case STATE_RUNNING://to run
                proc1->state_ts=CURRENT_TIME;
                proc1->state_c=STATE_RUNNING;

                if(proc1->cb==0){
                    //calculate new dpu burst
                    proc1->cb=myrandom(proc1->c_b);}
                    proc1->cw+=proc1->timeInPrevState;

                if(proc1->cb<=proc1->rcb){
                }
                else{
                    //if cpu burst greater that remaining total cpu time
                    proc1->cb=proc1->rcb;
                }
                //create new event. for round robin check quantum

                if (proc1->cb<=THE_SCHEDULER->getquantum())

                    //if cpu burst less than quantum create event running->blocked
                    e=new Events(CURRENT_TIME+proc1->cb, proc1,STATE_BLOCKED);

                //if cpu burst greater than quantum create event running->ready
                else{
                    e=new Events(CURRENT_TIME+THE_SCHEDULER->getquantum(), proc1,STATE_READY);
                }

                insertev(e);
                //cout<<" cb="<<proc1->cb<<" rem="<<proc1->rcb<<" prio="<<proc1->d_prio;

                break;

            case STATE_BLOCKED://to block

                proc1->rcb=proc1->rcb-proc1->cb;
                proc1->cb=0;
                CURRENT_RUNNING_PROCESS=NULL;
                CALL_SCHEDULER=true;

                if(proc1->rcb==0)
                {
                    proc1->ft=CURRENT_TIME;//update finishing time
                    proc1->tt=proc1->ft-proc1->a_t;//update turn around time
                    //cout<<"\n process done"<<endl;
                    break;
                }
                if(counter==0)
                {
                    iobusy=CURRENT_TIME;
                }
                counter++;
                proc1->state_ts=CURRENT_TIME;
                proc1->state_c=STATE_BLOCKED;
                //chose io burst random

                proc1->ib=myrandom(proc1->i_o);
                //create vent blocked->ready
                e=new Events(CURRENT_TIME+proc1->ib, proc1,STATE_READY);
                insertev(e);
                proc1->it+=proc1->ib;
                //cout<<" ib="<<proc1->ib<<" rem="<<proc1->rcb;

                break;

        }//end of switch
        if(CALL_SCHEDULER){

            if(get_next_event_time()==CURRENT_TIME){
                //cout<<endl;
                continue;
            }
            CALL_SCHEDULER=false;
            if(CURRENT_RUNNING_PROCESS==NULL){
                //cout<<endl;

                CURRENT_RUNNING_PROCESS=THE_SCHEDULER->get_next_process();

                if(CURRENT_RUNNING_PROCESS==NULL){

                    continue;

                }
            e=new Events(CURRENT_TIME, CURRENT_RUNNING_PROCESS,STATE_RUNNING);

            insertev(e);

            }
        }
        //cout<<endl;

    }//end of while loop
}//end of simulation



int main(int argc, char* argv[])
{
    bool v_flag=0;
    char *argSched=NULL;
    int c;
    opterr=0;
    while((c=getopt(argc, argv,"vs:"))!=-1){
        switch(c)
        {

        case 'v':
            v_flag=1;
            break;
        case 's':
            argSched=optarg;
            break;
        case '?':
            {
                if (optopt == 's')
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
    ifstream rfile;
    string nline;
     char *InputFile = argv[optind];
     char *RandFile = argv[optind + 1];
      int TQuantum = 9999999;
       if(argSched[0] == 'R' || argSched[0] == 'P')
    {
        string time = schedulerArg.substr(1);
        stringstream ttime(time);
        ttime>>TQuantum;

    }


    rfile.open(RandFile);

    if (rfile.fail()) {
        cerr << "Unable to open file for reading." << endl;
            exit(1);
    }


    rfile >> num_randoms;


        randvals = new long int[num_randoms];

        for (int i=0;i<num_randoms;i++){
            rfile >> randvals[i];
            }
    if (rfile.is_open())
        rfile.close();
    ofs=0;
    list<Events*>::iterator it;

    inFile.open(InputFile);
        if (inFile.fail()) {
        cerr << "Unable to open file for reading." << endl;
            exit(1);
    }

    int a_t1, t_c1,c_b1,i_o1;
    int id=0;
    while(true)
        {

            inFile >> a_t1 >> t_c1 >> c_b1 >> i_o1;
            if (inFile.eof()) break;
            proc_objects.push_back(Process(a_t1, t_c1,c_b1,i_o1,id));
            id++;

        }
    if (inFile.is_open())
        inFile.close();
    for(int i = 0; i <proc_objects.size(); i++){

        Events* e=new Events(proc_objects[i].a_t, &proc_objects[i],STATE_READY);
        eventq.push_back(e);
     }


    for (it = eventq.begin(); it!=eventq.end(); it++) {
        //(*it)->displayEvent();
      }

    if(argSched[0] == 'F'){

        THE_SCHEDULER = new FCFS(TQuantum);}
    else if(argSched[0] == 'S'){

        THE_SCHEDULER = new SJF(TQuantum);}
    else if(argSched[0] == 'L'){

       THE_SCHEDULER = new LCFS(TQuantum);}
    else if(argSched[0] == 'R'){

        THE_SCHEDULER = new RR(TQuantum);}
    else{
        THE_SCHEDULER = new Priority(TQuantum);

    }



    Simulation();

    if(argSched[0] == 'F'){
        cout<<"FCFS"<<endl;
        }
    else if(argSched[0] == 'S'){
            cout<<"SJF"<<endl;
        }
    else if(argSched[0] == 'L'){
        cout<<"LCFS"<<endl;
       }
    else if(argSched[0] == 'R'){
        cout<<"RR "<<TQuantum<<endl;
        }
    else{

        cout<<"PRIO "<<TQuantum<<endl;
    }
    int maxft=proc_objects[0].ft;
    double avgtat=0.0;
    double avgcw=0.0;
    int n =proc_objects.size();
    for(int i = 0; i <proc_objects.size(); i++){
        if (maxft<proc_objects[i].ft)
            maxft=proc_objects[i].ft;
        proc_objects[i].displayf();
        avgtat+=proc_objects[i].tt;
        avgcw+=proc_objects[i].cw;
     }
     avgtat=avgtat/n;//average turn arround time
     avgcw=avgcw/n;//average cpu waitint time
     double thrpt=(double(n)/maxft)*100;

     double cpuu=(double(cpu)/maxft)*100;
     double iouu=(double(iou)/maxft)*100;

     printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",maxft,cpuu,iouu,avgtat,avgcw,thrpt);
     return 0;


}
