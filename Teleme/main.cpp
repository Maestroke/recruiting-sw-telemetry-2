#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;

extern "C"{
    #include "fake_receiver.h"
}

int line=0;
int line_count=0;           //veriabile utilizzata per sapere quante righe ha il file candump.log
char message[20];
string s;
vector<string> scstart;     //vettore con i codici per passare allo stato Run
vector<string> scstop;      //vettore con i codici per passare allo stato Stop
FILE* file = NULL;
vector<string> vID;         //vettore contenente tutti i diversi ID che appaiono in una sessione di Run
vector<int> cID;            //vettore contenente i contatori per ciascun ID apparso in una sessione di Run

void error_state();
void run_state();

void static_data(string s){     //Controllo ID per creare il file Static.csv in seguito
    int div;

    div=s.find( '#');
    string sID;
    sID = s.substr(0, div);

    for(int i=0; i<vID.size(); i++){
        if(sID==vID[i]){
            cID[i]++;
            return;
        }
    }
    vID.push_back(sID);
    cID.push_back(1);
}

void static_compile(float mean_time){       //Creazione e compilazione file Static.csv
    string file_name;
    time_t data;
    tm *local;
    data=time(NULL);
    local=localtime(&data);
    file_name= "..\\Static\\"+to_string(local->tm_mday)+"-"+to_string(local->tm_mon+1)+"--"+to_string(local->tm_hour)+"-"+to_string(local->tm_min)+"-"+to_string(local->tm_sec)+".csv"; //Per creare un nome costantemente diverso uso Data e Ora di creazione come nome
    file = fopen(file_name.c_str(), "w");

    for(int i=0; i<vID.size(); i++){
        s=vID[i]+";"+ to_string(cID[i])+";"+ to_string(mean_time/(float)cID[i])+"\n";
        fprintf(file, s.c_str());
    }

    fclose(file);

    vID.clear();
    cID.clear();
}

void load_code(){       //Caricamento dei codici per passare tra Run state e Idle state da file salvato in locale
    char c;
    string t="";

    file = fopen("..\\code\\start_codes.txt", "r");
    while((c = fgetc(file)) != EOF) {
        while (c != '\n' && c!=EOF) {
            t.push_back(c);
            c = fgetc(file);
        }
        scstart.push_back(t);
        t="";
    }
    fclose(file);

    file = fopen("..\\code\\stop_codes.txt", "r");
    while((c = fgetc(file)) != EOF) {
        while (c != '\n' && c!=EOF) {
            t.push_back(c);
            c = fgetc(file);
        }
        scstop.push_back(t);
        t="";
    }
    fclose(file);
}

string parse(string s){     //Funzione per convertire il messaggio
    short int ID;
    int div;

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

int controllo_start(string s){  //Funzione per controllare se il messaggio è un messaggio di start
    int div;

    div=s.find( '#');
    string sID, sIDc;
    string sPL, sPLc;

    sID = s.substr(0, div);
    sPL = s.substr(div, s.length());

    for(int i=0; i<scstart.size(); i++){
        div=scstart[i].find( '#');

        sIDc = scstart[i].substr(0, div);
        sPLc = scstart[i].substr(div, s.length());

        if(sID==sIDc && sPL==sPLc){
            return 1;
        }
    }
    return 0;
}

int controllo_stop(string s){   //Funzione per controllare se il messaggio è un messaggio di stop
    int div;

    div=s.find( '#');
    string sID, sIDc;
    string sPL, sPLc;

    sID = s.substr(0, div);
    sPL = s.substr(div, s.length());

    for(int i=0; i<scstop.size(); i++){
        div=scstop[i].find( '#');

        sIDc = scstop[i].substr(0, div);
        sPLc = scstop[i].substr(div, s.length());

        if(sID==sIDc && sPL==sPLc){
            return 1;
        }
    }
    return 0;
}

void idle_state(){      //Funzione per gestire la macchina durante l'Idle state
    while(line<line_count){
        line++;
        can_receive(message);
        s=parse(message);

        if(controllo_start(s)){
            run_state();
        }
    }
}

void run_state(){       //Funzione per gestire la macchina durante il run state
    string file_name;
    time_t data, start;
    tm *local;
    start=data=time(NULL);
    local=localtime(&data);
    file_name= "..\\Log\\"+to_string(local->tm_mday)+"-"+to_string(local->tm_mon+1)+"--"+to_string(local->tm_hour)+"-"+to_string(local->tm_min)+"-"+to_string(local->tm_sec)+".log";    //Per creare un nome costantemente diverso uso Data e Ora di creazione come nome
    file = fopen(file_name.c_str(), "w");

    bool c=true;
    while(line<line_count && c){
        line++;
        can_receive(message);
        s=parse(message);
        static_data(s);
        data=time(NULL);
        local=localtime(&data);
        s="("+to_string(local->tm_hour)+"-"+to_string(local->tm_min)+"-"+to_string(local->tm_sec)+") "+s+'\n';
        fprintf(file, s.c_str());

        if(controllo_stop(s)){
            c=false;
        }
    }
    fclose(file);
    static_compile(difftime(mktime(local),start));
    idle_state();
}

void error_state(){     //La macchina entra in questo stato quando si verifica un evento indesiderato, attualmente l'unico evento è la recezione di un Payload dispari
    cout<<"E' stato rivelato un errore!!!\n";
    cout<<"Arresto macchina";
    close_can();
    line_count=0;
}

int main(void){
    load_code();
    line_count=open_can("..\\candump.log");

    idle_state();
    return 0;
}