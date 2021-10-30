#include <common.h>
#include "FIFOreqchannel.h"
#include <chrono>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace std;
using namespace chrono;

int main(int argc, char *argv[]){
    
    int opt;
    int p = 0;
    int e = 1;
    double t = -1.0;
    int buffS = MAX_MESSAGE; //buffer size
    string filename = "";
    bool DP = false; //data point section
    bool NC = false; //new channel section
    bool RF= false; //requesting files section
    
    while ((opt = getopt (argc, argv, "p:t:e:m:f:c")) != -1)
    {
        switch (opt)
        {
            case 'p':
                p = atoi (optarg);
                DP = true;
                break;
            case 't':
                t = atof (optarg);
                break;
            case 'e':
                e = atoi (optarg);
                break;
            case 'm':
                buffS = atoi (optarg);
                break;
            case 'f':
                RF = true;
                filename = optarg;
                break;
            case 'c':
                NC = true;
                break;
            
        }
    }
    auto timeStart = high_resolution_clock::now();
    
    if (fork() == 0) // child server creation using fork() and execvp()
    {
          cout << "Using child process" << endl;
        char* args [] = {"./server", "-m", (char *)to_string(buffS).c_str()};
        execvp("./server", args);
        exit(0);
    }
    
    FIFORequestChannel* chan = new FIFORequestChannel ("control", FIFORequestChannel::CLIENT_SIDE);
    FIFORequestChannel* currentChan = chan;

    if (NC)
    {
        //creates new channel, saves old channel in mem
        MESSAGE_TYPE m = NEWCHANNEL_MSG;
        char newChanBuf[MAX_MESSAGE];
        chan->cwrite (&m, sizeof (m));
        chan->cread (newChanBuf, sizeof(newChanBuf));
        currentChan = new FIFORequestChannel (newChanBuf, FIFORequestChannel::CLIENT_SIDE);
        cout << "New channel created " << endl;;
        datamsg* x = new datamsg(7, 0.00, 1);
        chan->cwrite(x,sizeof(datamsg));
        double reply;
        chan->cread(&reply, sizeof(double));
        cout << "Test point when new channel is created: " << reply << endl;
    }
    
    else if(DP){
    datamsg x(p, t, e);
    //3 arrays that will store 1000 values 
    double timeT [1000];
    double ecg1T [1000];
    double ecg2T [1000];
    //timeCounter will store the new times as the loop goes over
    double timeCounter = 0;
    //initalizing i for loop
    int i = 0;

    //Enter in a negative value for t to request 1000 points
    if(t < 0){
        while (i < 1000){
            datamsg temp1 (p, timeCounter, 1);
            chan->cwrite (&temp1, sizeof (datamsg)); // question
            double reply1;
            int nbytes = chan->cread (&reply1, sizeof(double)); //answer
            timeT [i] = timeCounter; //storing current time into timeT array
            ecg1T [i] = reply1; //storing value of ecg1 at the current time into the ecg1 array
            datamsg temp2 (p, timeCounter, 2);
            chan->cwrite (&temp2, sizeof (datamsg)); // question
            double reply2;
            nbytes = chan->cread (&reply2, sizeof(double)); //answer
            ecg2T [i] = reply2; 

            timeCounter += 0.004; 
            i = i + 1; 
        }
        i = 0; 
       ofstream myfile("./recieved/x1.csv");
        while (i < 1000)
        {
            myfile << timeT[i] << "," << ecg1T[i] << "," << ecg2T[i] << "\n"; //stores values from array into csv

            i = i + 1;
        }
        myfile.close();
    }
    else if (t > 0) {
        chan->cwrite (&x, sizeof (datamsg)); // question
        double reply;
        int nbytes = chan->cread (&reply, sizeof(double)); //answer
        cout << "For person " << p <<", at time " << t << ", the value of ecg "<< e <<" is " << reply << endl;
    }}
    
    //Requesting Data Files
    else if (RF)
    {
    //Finding file length factoring in buffer size
    filemsg f (0,0);
    char* buf = new char [sizeof(filemsg) + filename.size() +1]; //buffer
    memcpy (buf, &f, sizeof(filemsg));
    memcpy (buf+sizeof(filemsg), filename.c_str(),filename.size() + 1);
    chan->cwrite(buf, sizeof(filemsg) + filename.size()+1);
    __int64_t fileLen; //file length variable
    chan->cread(&fileLen, sizeof (__int64_t));
    cout << "File Length: " << fileLen << endl;
    delete buf;

    filemsg *file = new filemsg (0,0);
    int set = sizeof(filemsg) + filename.size() + 1;
    char* buff_1 = new char [set];
    memcpy(buff_1, file, sizeof(filemsg));
    strcpy(buff_1+sizeof(filemsg), filename.c_str());
    chan->cwrite(buff_1, set);

    __int64_t size;

    chan->cread(&size, sizeof(__int64_t));
    
    //Creating new file in recieved directory
    string openFile = "./recieved/" + filename;
    FILE* ff = fopen(openFile.c_str(), "wb");

    char* handler = new char[buffS];

    while(size > 0)
    {
        int len = min((__int64_t)buffS, size);
        ((filemsg*)buff_1)->length = len;
        chan->cwrite(buff_1, set);
        chan->cread(handler, len);
        fwrite(handler, 1, len, ff);
        ((filemsg*)buff_1)->offset+= len;
        size-=len;
    }
    //closing
    fclose(ff);
    delete buff_1;
    delete handler;

    }
    //closing channel 
        MESSAGE_TYPE quit = QUIT_MSG;
        currentChan->cwrite (&quit, sizeof (MESSAGE_TYPE));
        if (currentChan != chan)
        { 
        cout << "New channel has been closed" << endl;
        chan->cwrite (&quit, sizeof (MESSAGE_TYPE));
        }
        sleep(2);
        
    auto timeEnd = high_resolution_clock::now();
    // time process creation 
    auto processDuration = duration_cast<microseconds>(timeEnd - timeStart).count();
    cout << "Time taken: " << (double)processDuration / 1000 << " ms." << endl;
    
}
