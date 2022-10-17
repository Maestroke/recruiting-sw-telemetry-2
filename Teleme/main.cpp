#include <stdio.h>
#include <iostream>
using namespace std;

extern "C"{
    #include "fake_receiver.h"
}

int line=0;
int line_count=0;
char message[20];
string s;
//string scstart="160#1021";
//string scstop="160#102255";
string scstart[5];
string scstop[5];
int i_start=0;
int i_stop=0;
time_t now;
FILE* file = NULL;

void error_state();

void load_code(){
    char c;
    string t="";

    file = fopen("..\\code\\start_codes.txt", "r");
    while((c = fgetc(file)) != EOF) {
        while (c != '\n' && c!=EOF) {
            t.push_back(c);
            c = fgetc(file);
        }
        scstart[i_start] = t;
        t="";
        i_start++;
    }
    fclose(file);

    file = fopen("..\\code\\stop_codes.txt", "r");
    while((c = fgetc(file)) != EOF) {
        while (c != '\n' && c!=EOF) {
            t.push_back(c);
            c = fgetc(file);
        }
        scstop[i_stop] = t;
        t="";
        i_stop++;
    }
    fclose(file);
}

string parse(string s){
    short int ID;
    int Payload, div;

    div=s.find( '#');
    if((s.length()-div-1)%2==0){
        string sID;
        string sPL, sPLT;
        int t;
        sPL="";

        sID = s.substr(0, div);
        ID=stoi (sID,nullptr,16);
        sID= to_string(ID);

        for(int i=div+1; i<s.length(); i=i+2){
            sPLT=s.substr(i, 2);
            t=stoi (sPLT,nullptr,16);
            sPL=sPL+ to_string(t);
        }

         return sID+"#"+sPL;
    }
    else{
        error_state();
    }
}

int controllo_start(string s){
    int div;

    div=s.find( '#');
    string sID, sIDc;
    string sPL, sPLc;

    sID = s.substr(0, div);
    sPL = s.substr(div, s.length());

    for(int i=0; i<i_start; i++){
        div=scstart[i].find( '#');

        sIDc = scstart[i].substr(0, div);
        sPLc = scstart[i].substr(div, s.length());

        if(sID==sIDc && sPL==sPLc){
            return 1;
        }
    }
    return 0;
}

int controllo_stop(string s){
    int div;

    div=s.find( '#');
    string sID, sIDc;
    string sPL, sPLc;

    sID = s.substr(0, div);
    sPL = s.substr(div, s.length());

    for(int i=0; i<i_stop; i++){
        div=scstop[i].find( '#');

        sIDc = scstop[i].substr(0, div);
        sPLc = scstop[i].substr(div, s.length());

        if(sID==sIDc && sPL==sPLc){
            return 1;
        }
    }
    return 0;
}

void run_state();

void idle_state(){
    while(line<line_count){
        line++;
        can_receive(message);
        s=parse(message);
        cout<<s<<endl;

        if(controllo_start(s)){
            cout<<"Passagio a Run State\n";
            run_state();
        }
    }
}

void run_state(){
    string file_name;
    struct tm *local = localtime(&now);
    file_name= "..\\Log\\"+to_string(local->tm_mday)+"-"+to_string(local->tm_mon)+"--"+to_string(local->tm_hour)+"-"+to_string(local->tm_min)+"-"+to_string(local->tm_sec)+".log";
    file = fopen(file_name.c_str(), "w");

    while(line<line_count){
        line++;
        can_receive(message);
        s=parse(message);
        cout<<s<<endl;
        s=s+'\n';
        fprintf(file, s.c_str());

        if(controllo_stop(s)){
            cout<<"Passaggio a Idle State\n";
            fclose(file);
            idle_state();
        }
    }
}

void error_state(){
    cout<<"E' stato rivelato un errore!!!\n";
    cout<<"Arresto macchina";
    close_can();
    line_count=0;
}

int main(void){
    load_code();
    line_count=open_can("..\\candump.log");
    cout<<line_count<<endl;

    idle_state();
    return 0;
}