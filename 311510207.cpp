#include <iostream>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <set>
using namespace std;
// @b
bool inCommentBlock = false;
vector<double> lib_index1,lib_index2;


struct table_inf{
    double cap1 , cap2;
    vector<vector<double>> rise_power;
    vector<vector<double>> fall_power;
    vector<vector<double>> cell_rise;
    vector<vector<double>> cell_fall;
    vector<vector<double>> rise_transition;
    vector<vector<double>> fall_transition;
    table_inf() : rise_power(7, vector<double>(7)) ,
                 fall_power(7, vector<double>(7)) ,
                 cell_rise(7, vector<double>(7)) ,
                 cell_fall(7, vector<double>(7)) ,
                 rise_transition(7, vector<double>(7)) ,
                 fall_transition(7, vector<double>(7)) {}
};
table_inf INVX1,NOR2X1,NANDX1;


typedef struct gate_info gate_info;
struct node_info{
    double cap ;
    double transition;
    double acc_delay;
    bool valid ;  // transition is valid 
    bool cs ;
    gate_info* gate_ptr ;// node 指向 gate的指標
   //map<string, node_info>::iterator in_1;
};

struct gate_info{
    string name;
    string gate_type;
    double cap ;// out cap
    double power;
    double sw_power;
    double delay; //delay
    double transition;
    double toggle_time ;
    bool toggle;
    bool cs;
  //  string in_1;
    //string in_2;
   // string out;
    map<string, node_info>::iterator in_1; // gate指向 node的指標
    map<string, node_info>::iterator in_2;
    map<string, node_info>::iterator out;


};
struct gate_delay {
    double delay=0;
    double transition_time=0;
    double power ;
};


// struct nnode_size {
//     int num=0;//////////////
//     int max_id=0;
// };


 // chat gpt //
std::string removeComments(const std::string& line ,bool& inCommentBlock ) {
    std::string result;
    for (size_t i = 0; i < line.length(); ++i) {
        if (!inCommentBlock && line[i] == '/' && i + 1 < line.length() && line[i + 1] == '*') {
            inCommentBlock = true;
            ++i;
        } else if (inCommentBlock && line[i] == '*' && i + 1 < line.length() && line[i + 1] == '/') {
            inCommentBlock = false;
            ++i;
        } else if (!inCommentBlock && line[i] == '/' && i + 1 < line.length() && line[i + 1] == '/') {
            break;
        } else if (!inCommentBlock) {
            result += line[i];
        }
    }

    return result;
}

double cap_in(ifstream& lib) {  //for input capacitance
    string line ;
    string str ; 
    double db;
    char ch ;
     while (getline(lib, line)) {
        stringstream in;
        in << line;
        in >> str ; 
        if( str == "capacitance"){
            in  >> ch >> db;
           return db;  
        }
    }
}

void table_in(ifstream& lib , vector<vector<double>>& rise_power) {  //for input capacitance

    string line ;
    string str ; 
    double db;
    char ch ;
    int i=0;
     while (getline(lib, line)) {
        stringstream in;
        in << line;
        while(in >> ch){
            if(ch == '"'){
                int j = 0;
                while(in >>db){
                    rise_power[i][j]=db;
                    in >> ch;   // 忽略逗號
                    j++;
                    if(ch == '"') break; 
                }
                break;
            }
        }
        if(i>=6) break;
        i++;
    }
}

vector<gate_info*> gate_order_ptr;//排好順序的gate_ptr
map<string,gate_info*> gate_set;

void sortgate( gate_info* gate_ptr) {
    // cout << "gate now "<<gate_ptr->name  <<endl;
    map<string, node_info>::iterator node_ptr1 ,node_ptr2 , output_node;
    if (gate_ptr == nullptr) {
        return;
    }
    node_ptr1 =gate_ptr->in_1;
    if(gate_ptr->gate_type == "INVX1"){
       // cout <<node_ptr1->second.valid <<endl;
        if(node_ptr1->second.valid){//看map的地址指到gate只回node看valid   it->second指到gate的指標
                gate_ptr->out->second.valid = true;
                gate_order_ptr.push_back(gate_ptr);//把可以做的gate指標存回去 vector
                auto it = gate_set.find(gate_ptr->name);
               // cout <<"success: "<< gate_ptr->name <<endl;
                gate_set.erase(it);
                return;
        }
         // 遞歸處理in_1的gate

        auto it = gate_set.find(node_ptr1->second.gate_ptr->name);
        if (it != gate_set.end()) {
             //cout << "inv previos in_1 "<<node_ptr1->second.gate_ptr->name  <<endl;
            sortgate(node_ptr1->second.gate_ptr);
        }
        // 遞歸處理原本的gate
            sortgate(gate_ptr);
        // cout <<"gate complete:  " << gate_ptr->name<<endl ;
        // 遞歸處理原本的gate
      //  sortgate(gate_ptr);
    }
    else {
        node_ptr2 =gate_ptr->in_2;
        if(node_ptr1->second.valid){//看map的地址指到gate只回node看valid   it->second指到gate的指標
            if(node_ptr2->second.valid){
                gate_ptr->out->second.valid = true;
                gate_order_ptr.push_back(gate_ptr);//把可以做的gate指標存回去 vector
                auto it = gate_set.find(gate_ptr->name);
                // cout <<"success: "<< gate_ptr->name <<endl;
                gate_set.erase(it);
                return;
            }
            else {
                auto itt = gate_set.find(node_ptr2->second.gate_ptr->name);
                 // 遞歸處理in_2的gate
                if (itt != gate_set.end()) {
                  //  cout << "previos in_1 "<<node_ptr2->second.gate_ptr->name  <<endl;
                    sortgate(node_ptr2->second.gate_ptr);
                }
            }
        }
        else{
             // 遞歸處理in_1的gate
            auto it = gate_set.find(node_ptr1->second.gate_ptr->name);
            if (it != gate_set.end()) {
               // cout << "previos in_1 "<<node_ptr1->second.gate_ptr->name  <<endl;
                sortgate(node_ptr1->second.gate_ptr);

            }
        }

        // auto itt = gate_set.find(node_ptr2->second.gate_ptr->name);
        //  // 遞歸處理in_2的gate
        // if (itt != gate_set.end()) {
        //     cout << "previos in_1 "<<node_ptr2->second.gate_ptr->name  <<endl;
        //     sortgate(node_ptr2->second.gate_ptr);
            
        
         //遞歸處理原本的gate
       // auto ittt = gate_set.find(gate_ptr->name);
       // if (ittt != gate_set.end()) {
            sortgate(gate_ptr);
      //  }
        //cout <<"gate complete:  " << gate_ptr->name <<endl;

       
    }
}
vector<double> interp_point (vector<double> index , double in){
    vector<double> vector_double ;
    if(in < index[0]){
        vector_double.push_back(0);
        vector_double.push_back(1);
    }
    if(in > index[index.size()-1]){
        vector_double.push_back(index.size()-2); 
        vector_double.push_back(index.size()-1); 
    }
    for(int i=0 ; i< (index.size()-1) ;i++){  
        if(( index[i]<in)&&(in<index[i+1])) { 
            vector_double.push_back(i); 
            vector_double.push_back(i+1); 

            break ;
        }
        if( index[i]==in) { 
            vector_double.push_back(i); 
            vector_double.push_back(i); 
            break ;
        }
        else if( index[i+1]==in) {
            vector_double.push_back(i+1); 
            vector_double.push_back(i+1); 
            break ;
        }
    }
    return vector_double;
}

double interp1(double x0, double y0, double x1, double y1, double x) {
    double y =0  ;
    if (x0 == x1) {
        y = y0 ;
        return y ;
    }
    else {
    double y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
     return y;
    }
}

double interp2(double x0, double y0 ,double x1, double y1,double z0 ,double z1 , double z2 ,  double z3, double x ,double y ) { 
    double a0 ,a1;
    double z ;
    a0 = interp1(x0 , z0 , x1 , z2 , x) ;
    a1 = interp1(x0 , z1 , x1 , z3 , x) ;
    z  = interp1(y0 , a0 , y1 , a1 , y) ;
    return z; 
}

gate_delay caculate_delay (const double input_transition , const double cap ,const string gate_name ,const bool logic ){
    gate_delay ans;
    vector<double> i;
    vector<double> j;
    double delay , output_transition  ,power;
    double x0 , x1 , y0 , y1 , z0 , z1 , z2 , z3 , x , y;
    i =  interp_point (lib_index2 , input_transition);  //lib_index2 index_input_transition_time
    j = interp_point (lib_index1 , cap);  //index_1 total_output_net_capacitance
    x0 =  lib_index2[i[0]];
    x1 =  lib_index2[i[1]];
    y0 = lib_index1[j[0]];
    y1 = lib_index1[j[1]];
    x = input_transition;
    y= cap;
    if(gate_name == "INVX1"){
        if(logic==1){
            z0 = INVX1.cell_rise[i[0]][j[0]];
            z1 = INVX1.cell_rise[i[0]][j[1]];
            z2 = INVX1.cell_rise[i[1]][j[0]];
            z3 = INVX1.cell_rise[i[1]][j[1]];
        }
        else{
            z0 = INVX1.cell_fall[i[0]][j[0]];
            z1 = INVX1.cell_fall[i[0]][j[1]];
            z2 = INVX1.cell_fall[i[1]][j[0]];
            z3 = INVX1.cell_fall[i[1]][j[1]];
        }
        delay = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

        if(logic==1){
            z0 = INVX1.rise_transition[i[0]][j[0]];
            z1 = INVX1.rise_transition[i[0]][j[1]];
            z2 = INVX1.rise_transition[i[1]][j[0]];
            z3 = INVX1.rise_transition[i[1]][j[1]];
        }
        else {
            z0 = INVX1.fall_transition[i[0]][j[0]];
            z1 = INVX1.fall_transition[i[0]][j[1]];
            z2 = INVX1.fall_transition[i[1]][j[0]];
            z3 = INVX1.fall_transition[i[1]][j[1]];
        }
        output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

        if(logic==1){
            z0 = INVX1.rise_power[i[0]][j[0]];
            z1 = INVX1.rise_power[i[0]][j[1]];
            z2 = INVX1.rise_power[i[1]][j[0]];
            z3 = INVX1.rise_power[i[1]][j[1]];
        }
        else {
            z0 = INVX1.fall_power[i[0]][j[0]];
            z1 = INVX1.fall_power[i[0]][j[1]];
            z2 = INVX1.fall_power[i[1]][j[0]];
            z3 = INVX1.fall_power[i[1]][j[1]];
        }
        power = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
    }

    else if(gate_name == "NOR2X1"){
        if(logic==1){
            z0 = NOR2X1.cell_rise[i[0]][j[0]];
            z1 = NOR2X1.cell_rise[i[0]][j[1]];
            z2 = NOR2X1.cell_rise[i[1]][j[0]];
            z3 = NOR2X1.cell_rise[i[1]][j[1]];
        }
        else{
            z0 = NOR2X1.cell_fall[i[0]][j[0]];
            z1 = NOR2X1.cell_fall[i[0]][j[1]];
            z2 = NOR2X1.cell_fall[i[1]][j[0]];
            z3 = NOR2X1.cell_fall[i[1]][j[1]];
        }
        delay = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

        if(logic==1){
            z0 = NOR2X1.rise_transition[i[0]][j[0]];
            z1 = NOR2X1.rise_transition[i[0]][j[1]];
            z2 = NOR2X1.rise_transition[i[1]][j[0]];
            z3 = NOR2X1.rise_transition[i[1]][j[1]];
        }
        else {
            z0 = NOR2X1.fall_transition[i[0]][j[0]];
            z1 = NOR2X1.fall_transition[i[0]][j[1]];
            z2 = NOR2X1.fall_transition[i[1]][j[0]];
            z3 = NOR2X1.fall_transition[i[1]][j[1]];
        }
        output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

        if(logic==1){
            z0 = NOR2X1.rise_power[i[0]][j[0]];
            z1 = NOR2X1.rise_power[i[0]][j[1]];
            z2 = NOR2X1.rise_power[i[1]][j[0]];
            z3 = NOR2X1.rise_power[i[1]][j[1]];
        }
        else {
            z0 = NOR2X1.fall_power[i[0]][j[0]];
            z1 = NOR2X1.fall_power[i[0]][j[1]];
            z2 = NOR2X1.fall_power[i[1]][j[0]];
            z3 = NOR2X1.fall_power[i[1]][j[1]];
        }
        power = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
    }
    else if(gate_name == "NANDX1"){
        if(logic==1){
            z0 = NANDX1.cell_rise[i[0]][j[0]];
            z1 = NANDX1.cell_rise[i[0]][j[1]];
            z2 = NANDX1.cell_rise[i[1]][j[0]];
            z3 = NANDX1.cell_rise[i[1]][j[1]];
        }
        else{
            z0 = NANDX1.cell_fall[i[0]][j[0]];
            z1 = NANDX1.cell_fall[i[0]][j[1]];
            z2 = NANDX1.cell_fall[i[1]][j[0]];
            z3 = NANDX1.cell_fall[i[1]][j[1]];
        }
        delay = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

        if(logic==1){
            z0 = NANDX1.rise_transition[i[0]][j[0]];
            z1 = NANDX1.rise_transition[i[0]][j[1]];
            z2 = NANDX1.rise_transition[i[1]][j[0]];
            z3 = NANDX1.rise_transition[i[1]][j[1]];
        }
        else {
            z0 = NANDX1.fall_transition[i[0]][j[0]];
            z1 = NANDX1.fall_transition[i[0]][j[1]];
            z2 = NANDX1.fall_transition[i[1]][j[0]];
            z3 = NANDX1.fall_transition[i[1]][j[1]];
        }
        output_transition = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);

        if(logic==1){
            z0 = NANDX1.rise_power[i[0]][j[0]];
            z1 = NANDX1.rise_power[i[0]][j[1]];
            z2 = NANDX1.rise_power[i[1]][j[0]];
            z3 = NANDX1.rise_power[i[1]][j[1]];
        }
        else {
            z0 = NANDX1.fall_power[i[0]][j[0]];
            z1 = NANDX1.fall_power[i[0]][j[1]];
            z2 = NANDX1.fall_power[i[1]][j[0]];
            z3 = NANDX1.fall_power[i[1]][j[1]];
        }
        power = interp2( x0,  y0 , x1,  y1, z0 , z1 ,  z2 ,   z3,  x , y);
    }
    ans.delay = delay;
    ans.transition_time = output_transition;
    ans.power = power;
    return ans;
}

///////////////////////////////////////////////////////////////////
//                            main.cpp                           //
///////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]){
	ifstream inFile ,pat,lib;
    ofstream txt_capacitance,txt_gate_info ,txt_gate_power ,txt_coverage ; 
    string line;
    string str  ; 
    char ch;
    int num  ; 
    int node_num = 0 ;
    int id ; 
    double db;
    double arr[7][7];

    ////////////////////////////////////////////////////////////////
    // --------------       -read lib        --------------------//
    ///////////////////////////////////////////////////////////////

    
    lib.open(argv[3]);
    lib_index1.reserve(7);
    lib_index2.reserve(7);
    while (getline(lib, line)) {
       //line = removeComments(line , inCommentBlock); 
       stringstream in;
       in << line;
       in >> str  ;    
       if( str == "index_1"){
            while(in >> ch){
                if(ch == '"'){
                    while(in >>db){
                        lib_index1.push_back(db);
                        in >> ch;   // 忽略逗號
                        if(ch == '"') break; 
                    }
                }
            }
         }
       else if( str == "index_2"){
            while(in >> ch){
                if(ch == '"'){
                    while(in >>db){
                        lib_index2.push_back(db);
                        in >> ch;   // 忽略逗號
                        if(ch == '"') break; 
                    }
                }
            }
         }

        in >> str  ; // cell 拿掉
         if( str == "(NOR2X1)"){
            //cout << line <<endl;
            //getline(lib, line);
           // cout << line <<endl;
            while (getline(lib, line)) {
                //cout << line <<endl;
                stringstream in;
                in << line;
                in >> str ; 
                if(str == "pin(A1)") {
                    NOR2X1.cap1 = cap_in(lib);
                    continue;
                }
                if(str == "pin(A2)")  NOR2X1.cap2 = cap_in(lib);
                else if( str == "rise_power(table10){")        table_in(lib,NOR2X1.rise_power);
                else if( str == "fall_power(table10){")        table_in(lib,NOR2X1.fall_power);
                else if( str == "cell_rise(table10){")         table_in(lib,NOR2X1.cell_rise);
                else if( str == "cell_fall(table10){")         table_in(lib,NOR2X1.cell_fall);
                else if( str == "rise_transition(table10){")   table_in(lib,NOR2X1.rise_transition);
                 if(str == "fall_transition(table10){") {
                    table_in(lib,NOR2X1.fall_transition);
                    break;
                }

            }
        }

        else if( str == "(INVX1)"){
            while (getline(lib, line)) {
                stringstream in;
                in << line;
                in >> str ; 
                if(str == "pin(I)") {
                    INVX1.cap1 = cap_in(lib);
                    continue;
                }
                else if( str == "rise_power(table10){")        table_in(lib,INVX1.rise_power);
                else if( str == "fall_power(table10){")        table_in(lib,INVX1.fall_power);
                else if( str == "cell_rise(table10){")         table_in(lib,INVX1.cell_rise);
                else if( str == "cell_fall(table10){")         table_in(lib,INVX1.cell_fall);
                else if( str == "rise_transition(table10){")   table_in(lib,INVX1.rise_transition);
                else if(str == "fall_transition(table10){") {
                    table_in(lib,INVX1.fall_transition);
                    break;
                }

            }
        }
        else if( str == "(NANDX1)"){
            while (getline(lib, line)) {
                stringstream in;
                in << line;
                in >> str ; 
                if(str == "pin(A1)") {
                    NANDX1.cap1 = cap_in(lib);
                    continue;
                }
                if(str == "pin(A2)")  NANDX1.cap2 = cap_in(lib);
                else if( str == "rise_power(table10){")        table_in(lib,NANDX1.rise_power);
                else if( str == "fall_power(table10){")        table_in(lib,NANDX1.fall_power);
                else if( str == "cell_rise(table10){")         table_in(lib,NANDX1.cell_rise);
                else if( str == "cell_fall(table10){")         table_in(lib,NANDX1.cell_fall);
                else if( str == "rise_transition(table10){")   table_in(lib,NANDX1.rise_transition);
                else if(str == "fall_transition(table10){") {
                    table_in(lib,NANDX1.fall_transition);
                    break;
                }

            }
        }
    
    }
    lib.close();
    ///////////////////////////////////////////////////////////////
    // --------------       -read netlist    --------------------//
    ///////////////////////////////////////////////////////////////
    inFile.open(argv[1]); 
    set<string> input ,output  ;
    map<string, node_info> node;
    bool output_read, input_read,wire_read =false ;

    while (getline(inFile, line)) {
       // line = removeComments(line , inCommentBlock); 
        //cout <<line  <<endl ;
        stringstream in;
        in << line;
        in >> str ;    
       if( str == "input"){
            while (in >> ch){
                in >> num;
                str = ch + to_string(num);
                input.insert(str);
                node[str] = {0,0,0,true,false,nullptr};  // cap 、 transition、acc_delay 、 valid 、 cs 、 gate_ptr
                in >> ch ;  //逗號
                if (ch == ';')
                break;
            }
            input_read = true ;
       }
       else if( str == "output"){
            while (in >> ch){
                in >> num;
                str = ch + to_string(num);
                output.insert(str);
                node[str] = {0.0300000,0,0,false,false,nullptr};
                in >> ch ;  //逗號
                if (ch == ';')
                break;
            }
            output_read = true ;
       }
       else if( str == "wire"){
            while (in >> ch){
                in >> num;  
                str = ch + to_string(num);
                node[str] = {0,0,0,false,false,nullptr};
                in >> ch ; 
                if (ch == ';')
                break;
            }
            wire_read= true;
       }
       if(wire_read&&output_read&&input_read) break;
    } 



    // for(int i =0; i<lib_index2.size(); i++){
    //     cout <<lib_index2[i]<<endl;

    // }
    cout <<NANDX1.cap1<<endl;//NANDX1  NOR2X1 INVX1
    cout <<NANDX1.cap2<<endl;
    for(int i=0; i<NANDX1.fall_transition.size();i++ ){
        for(int j =0; j<NANDX1.fall_transition[i].size(); j++){
            cout <<NANDX1.fall_transition[i][j]<<" ";

        }
        cout<<endl;
    }

    for(auto& pair : node){
        cout << pair.first<<" " ;// easy to debug
        cout << pair.second.cap<<" " ;
        cout << pair.second.transition<<" ";
        cout << pair.second.valid <<" ";
        cout<<endl;
    }
    cout << "node num" << node.size()<<endl;
 ///////////////////////////////////////////////////////////////////
//           step 1 read the gate and update capacitance         // 
///////////////////////////////////////////////////////////////////
    vector <gate_info> gate;
    gate_info gate_tmp;
    vector <double> double_vector;
    gate.reserve( node.size());
   // 讀檔時還用不到的data 先初始 //
    gate_tmp.name = "bad";
    gate_tmp.gate_type = "bad";
    gate_tmp.cap =0;
    gate_tmp.power = 0;
    gate_tmp.delay = 0;
    gate_tmp.in_2  = node.end();//先給空的 這樣可以判斷指標是否合法
    gate_tmp.in_1  = node.end();
    gate_tmp.out  = node.end();
    gate_tmp.cs =  false;
    gate_tmp.transition = 0;
    gate_tmp.sw_power=0;
    gate_tmp.toggle = false;
    gate_tmp.toggle_time = 0;
    while (getline(inFile, line)) {
        line = removeComments(line , inCommentBlock); 
        //cout <<line <<endl;
        stringstream in;
        in << line;
        in >> str ;
       if((str == "INVX1")||(str == "NANDX1")||(str == "NOR2X1")){
            gate.push_back(gate_tmp);
            bool second_input = false;
            gate.back().gate_type=str; 
            in >>ch>> num;
            str = ch + to_string(num);
            gate.back().name=str; 
            gate_set[str] = &gate.back(); //寫這個要小心空指標
          //  cout <<   gate_tmp.name << endl;
           // cout << "str: " << str << endl;
            in >> ch>>ch ; //把 第一個'(' 拿掉  
            while(in >> ch){
                if( ch == 'Z'){
                    in >> ch >> ch; 
                    in >>ch>> num;
                    str = ch + to_string(num);
                    gate.back().out= node.find(str); // 建立gate連到node的指標
                    node[str].gate_ptr =  &gate.back(); //建立 node 到
                    //gate_tmp.out->second.gate_ptr = &; 
                    // if (gate.back().out != node.end()) {
                    //    gate.back().out->second.cap += 0.03; 
                    // }
                   // test=node.find(str);
                   // gate_tmp.it_tmp =test;
                   // cout <<"test_first: "<< test->first<<endl;
                }
                 else if(ch == '('){  
                    in >>ch>> num;
                    str = ch + to_string(num);
                //     //id = table_id(ch ,node);
                    if(second_input){
                        gate.back().in_2=node.find(str);//ptr_node->find(str);
                        //cout <<  gate.back().in_2->first <<endl;
                        //cout <<  gate_tmp.in_2->first <<endl;
                        if      (gate.back().gate_type == "INVX1")  gate.back().in_2->second.cap +=  INVX1.cap2; 
                        else if (gate.back().gate_type == "NANDX1") gate.back().in_2->second.cap +=  NANDX1.cap2; 
                        else if (gate.back().gate_type == "NOR2X1") gate.back().in_2->second.cap +=  NOR2X1.cap2; 
                    }
                    else {
                        gate.back().in_1= node.find(str);//ptr_node->find(str);
                       // cout <<  gate_tmp.in_1->first <<endl;
                        if      (gate.back().gate_type == "INVX1")  gate.back().in_1->second.cap +=  INVX1.cap1; 
                        else if (gate.back().gate_type == "NANDX1") gate.back().in_1->second.cap +=  NANDX1.cap1; 
                        else if (gate.back().gate_type == "NOR2X1") gate.back().in_1->second.cap +=  NOR2X1.cap1; 
                     }
                     second_input = true;
                 }
                //cout << gate_tmp.out->second.cap <<endl;
                // cout << it->first <<endl;

              //  cout <<"test_first: "<< test->first<<endl;

            }
            //gate.push_back(gate_tmp);
            //cout <<  gate.back().in_1->first <<endl;
            //reset pointer
            // gate_tmp.in_2  = node.end();//先給空的 這樣可以判斷指標是否合法
            // gate_tmp.in_1  = node.end();
            // gate_tmp.out  = node.end();
        }

        
            // cout << gate_tmp.in_2->second.cap <<endl;
        
 
              //  cout << "Name: " << gate[0].name <<", "
              // << "gate_type: " << gate[0].gate_type << endl;
                //cout << "In1: " <<gate[0].out->second.cap << endl;
                //<< "In2: " << gate[0].in_2->second.name << endl;
        in.clear();
    }
        
   //  紀錄 cap 到每一個gate (取小數點6位) //
    for(int i=0; i< gate.size();i++){ 
       // str  = gate[i].out->first;
       // auto it = node.find(str);
       // gate[i].cap = it->second.cap;
       db= gate[i].out->second.cap;    
       gate[i].cap = db;
   } 

    for(int i=0; i< gate.size();i++){
        cout  << "name: " << gate[i].name <<", "
              <<"gate_type: " << gate[i].gate_type <<", ";
              cout<< "in_1: " << gate[i].in_1->first <<", ";
            if (gate[i].in_2 != node.end()) {
                cout<< "in_2: " <<gate[i].in_2->first <<", ";
            }
            cout<< "cap: " <<gate[i].cap <<", ";
            cout<< "state: " <<gate[i].cs <<", ";
           // cout << "out: " << gate[i].out->first ;
           cout<<endl;
   } 
     inFile.close(); 

    ///////////////////////////////////////////////////////////////////
    //          sort                                                // 
    ///////////////////////////////////////////////////////////////////
    gate_order_ptr.reserve( node.size());
    gate_info* gate_ptr;
    map<string, node_info>::iterator node_ptr;

    // auto it = gate_set.find("g1");
    // gate_ptr = it->second;
    // node_ptr =gate_ptr->in_1;
    //  sortgate(gate_ptr);

    // auto itee = gate_set.find("g10");
    // gate_ptr = itee->second;
    // node_ptr =gate_ptr->in_1;
    //  sortgate(gate_ptr);

     cout<<"first set "<<endl;
    for (const auto& element : gate_set) {
        std::cout << element.second->name << " ";
    }

    bool done = false ;
    int i=0;
    while (!done){
        if(gate_set.empty()){
            cout << "次數"<<i <<endl;
            done = true;
            break;
        }
        else {
             i++;
            auto it = gate_set.begin();
            gate_ptr = it->second;
            sortgate(gate_ptr);
        }
    }
    //}
   //
    cout<<"last set "<<endl;
    for (const auto& element : gate_set) {
        std::cout << element.second->name << " ";
    }

   for(int i=0 ; i<gate_order_ptr.size(); i++){
        cout<<"test: "<< gate_order_ptr[i]->name << endl;
    }
    // check gate info //
     gate_info *temp_ptr;
    for(auto& pair : node){
        cout << pair.first<<" " ;// easy to debug
        temp_ptr = pair.second.gate_ptr;
        if(temp_ptr != nullptr) cout << temp_ptr->name <<" ";
        cout << pair.second.cap<<" " ;
        cout << pair.second.transition<<" ";
        cout << pair.second.valid <<" ";

        cout<<endl;
    }
    
    ///////////////////////////////////////////////////////////////////
    //         step2  delay                                      //
    ///////////////////////////////////////////////////////////////////
    cout <<endl;
    cout << "step 2" << endl;

    pat.open(argv[2]);
    string base_name = argv[2];
    base_name.pop_back();//t
    base_name.pop_back();//a
    base_name.pop_back();//p
    base_name.pop_back();//.
    txt_gate_info.open("311510207_"+base_name+"_gate_info.txt");
    txt_gate_power.open("311510207_"+base_name+"_gate_power.txt");
    txt_coverage.open("311510207_"+base_name+"_coverage");

    vector<string > pat_inorder;
    node_info node_in_1,node_in_2 ,node_out;
    map<string, node_info>::iterator node_out_ptr;
    pat_inorder.reserve(input.size());
    gate_delay gate_cal; 
    double in_transition;
    double out_cap;
    double acc_delay;
    bool    logic;
    int iii=0;


    // 第一行 //
    getline(pat, line);
    stringstream in;
    in << line;
    in >>str; // input讀出來
    while (in >> ch){
        in >> num;
        str = ch + to_string(num);
        pat_inorder.push_back(str) ;
        in >>ch; // 逗號拿掉
    }
    //開始一行一行算
    while (getline(pat, line)) {
    //for( int i=0 ; i<3 ;i++){ 
       // cout <<"pat line "<< line <<endl;
       iii++;
        if(line == ".end") break;
        stringstream in;
        in << line;
        for(int i=0; i<pat_inorder.size();i++){
            in>>num;
            str= pat_inorder[i];
            node[str].cs = (num==1);
        }
        for(int i=0; i<gate_order_ptr.size();i++){  //i<gate_order_ptr.size()  開始一個一個gate算
            str=  gate_order_ptr[i]->gate_type;
            node_ptr = gate_order_ptr[i]->in_1;
            node_in_1 = node_ptr->second;
            if(str == "INVX1"){
                logic= !node_in_1.cs;
              //  cout << " cal  INVX1 " << gate_order_ptr[i]->name <<" "<< node_ptr->first <<" " << node_in_1.cs << " out " <<logic <<endl;
                in_transition = node_in_1.transition;
                acc_delay = node_in_1.acc_delay;
            }
            else{
                node_ptr = gate_order_ptr[i]->in_2;
                node_in_2 = node_ptr->second;
                if(str == "NANDX1"){
                    logic = !(node_in_1.cs && node_in_2.cs);
                    if(logic){
                        if((node_in_1.cs==0)&&(node_in_2.cs==0))            // 00
                            if(node_in_1.acc_delay <node_in_2.acc_delay) {   // in_1<in_2
                                in_transition = node_in_1.transition;       //小的
                                acc_delay = node_in_1.acc_delay;
                            }
                            else{
                                in_transition = node_in_2.transition;       //min
                                acc_delay = node_in_2.acc_delay;
                            }
                        else if((node_in_2.cs==0)){                         //10
                            in_transition = node_in_2.transition;
                            acc_delay = node_in_2.acc_delay;
                        }
                        else {                                             //01
                            in_transition = node_in_1.transition; 
                            acc_delay = node_in_1.acc_delay;
                        }
                    }
                    else // max                                         //11
                        if(node_in_1.acc_delay >node_in_2.acc_delay){    // in_1<in_2
                            in_transition = node_in_1.transition;       //大的
                            acc_delay = node_in_1.acc_delay;
                        }
                        else{
                             in_transition = node_in_2.transition;
                             acc_delay = node_in_2.acc_delay;
                        }
                }
                else if(str == "NOR2X1"){
                    logic = !(node_in_1.cs||node_in_2.cs);
                  //  cout << " cal  NOR2X1 " << gate_order_ptr[i]->name <<" "<< node_ptr->first <<" "<<  node_in_1.cs << " " << node_in_2.cs << " out " <<logic <<endl;
                    if(logic){                                         //00
                        if(node_in_1.acc_delay >node_in_2.acc_delay){    // in_1<in_2
                            in_transition = node_in_1.transition;       //大的
                            acc_delay = node_in_1.acc_delay;
                        }
                        else{
                            in_transition = node_in_2.transition;
                            acc_delay = node_in_2.acc_delay;
                        }
                    }
                    else {// max
                        if((node_in_1.cs==1)&&(node_in_2.cs==1))            // 11 選min
                            if(node_in_1.acc_delay <node_in_2.acc_delay){    // in1<in2
                                in_transition = node_in_1.transition;       //小的
                                acc_delay = node_in_1.acc_delay;
                            }
                            else{
                                in_transition = node_in_2.transition;  
                                acc_delay = node_in_2.acc_delay;
                            }    
                        else if((node_in_2.cs==1))   {                     //01
                            in_transition = node_in_2.transition;
                            acc_delay = node_in_2.acc_delay;
                        }
                        else{                                         //10
                            in_transition = node_in_1.transition; 
                            acc_delay = node_in_1.acc_delay;
                        }
                    }
                }

            }
            node_out_ptr =  gate_order_ptr[i]->out;
            out_cap = node_out_ptr->second.cap;
            gate_cal = caculate_delay (in_transition , out_cap ,str ,logic );  // caculate_delay(transition , cap , name , logic)

            // 更新node // 
            node_out_ptr->second.transition = gate_cal.transition_time;    // 紀錄transition_time
            node_out_ptr->second.acc_delay = acc_delay+gate_cal.delay;// 更新acc_delay
            node_out_ptr->second.cs = logic;

            //cout << "after cal "<<node_out_ptr->first <<" logic  "<< node_out_ptr->second.cs<<endl;

            // 更新gate // 
            gate_order_ptr[i]->delay = gate_cal.delay;
            gate_order_ptr[i]->transition = gate_cal.transition_time;
            gate_order_ptr[i]->power = gate_cal.power;
            gate_order_ptr[i]->sw_power = out_cap*0.9*0.9*0.5; //switch power
            gate_order_ptr[i]->toggle= ( gate_order_ptr[i]->cs !=logic)?1:0;
            gate_order_ptr[i]->toggle_time= (gate_order_ptr[i]->toggle)? gate_order_ptr[i]->toggle_time+1 : gate_order_ptr[i]->toggle_time;
            gate_order_ptr[i]->cs = logic;
        }
        
        // total power // 
        double total_power=0;
        double total_toggle=0;
  

        for (int i = 0; i < gate.size(); i++) {
	    	txt_gate_info<< gate[i].name << " ";
            txt_gate_info <<fixed << setprecision(6)<< gate[i].cs << " ";
            txt_gate_info <<fixed << setprecision(6)<< gate[i].delay << " ";
            txt_gate_info <<fixed << setprecision(6)<< gate[i].transition << " ";
            txt_gate_info<< endl;
        }
        txt_gate_info<< endl;


        for (int i = 0; i < gate.size(); i++) {
            total_power =  (gate[i].toggle)? total_power+gate[i].power+gate[i].sw_power :  total_power+gate[i].power  ;
            total_toggle = (gate[i].toggle_time >20)? total_toggle:total_toggle+gate[i].toggle_time;

	    	txt_gate_power<< gate[i].name << " ";
            txt_gate_power <<fixed << setprecision(6)<< gate[i].power << " ";
            txt_gate_power <<fixed << setprecision(6)<< gate[i].sw_power << " ";
            txt_gate_power<< endl;
        }
        txt_gate_power<< endl;


        db = total_toggle/(gate.size()*40)*100;
        txt_coverage << iii  <<" "<<fixed << setprecision(6)<< total_power<< " ";
        txt_coverage <<fixed << setprecision(2)<<db <<"%"<<endl;
        txt_coverage<< endl;

	}
    txt_gate_info.close();
    txt_gate_power.close();
    txt_coverage.close();
    //cout << "迴圈次數"<<ii <<endl;


    



    cout<<"pat_input"<<endl;
    for(int i=0 ; i<pat_inorder.size(); i++){
        cout<< pat_inorder[i] << " " ;
    }
    cout << endl;
    cout << endl;

    // check node //
    cout <<"check node " <<endl;
    for(auto& pair : node){
        cout << pair.first<<" " ;// easy to debug
        cout << pair.second.cap<<" " ;
        cout << pair.second.valid <<" ";
        cout << pair.second.cs <<" ";
         cout << pair.second.acc_delay <<" ";
            cout << pair.second.transition<<" ";
        cout<<endl;
    }
    cout << "node num" << node.size()<<endl;
    cout << endl;
    cout << endl;
    cout <<"gate check " <<endl;
    for(int i=0; i< gate.size();i++){
        cout  << "name: " << gate[i].name <<", ";
            //  <<"gate_type: " << gate[i].gate_type <<", ";
              //cout<< "in_1: " << gate[i].in_1->first <<", ";
           // if (gate[i].in_2 != node.end()) {
               // cout<< "in_2: " <<gate[i].in_2->first <<", ";
           // }
           // cout<< "cap: " <<gate[i].cap <<", ";
            cout<< "state: " <<gate[i].cs <<", ";
            cout<< "delay: " <<gate[i].delay <<", ";

            cout<< "tran: " <<gate[i].transition <<", ";
           // cout << "out: " << gate[i].out->first ;
           cout<<endl;
   } 
//   string name;
//     string gate_type;
//     double cap ;// out cap
//     double rise_power;
//     double fall_power;
//     double delay_rise;
//     double delay_fall; //delay
//     string in_1;
//     string in_2;
//     string out;

   
 

    // }   
    
    //     stringstream in;                                        
    //     for (int i = 0; i < output.size(); i++) {
    //         in << output[i];
	//     	in >>ch>> num; 
    //         id = table_id(ch ,node);
    //         capacitance[id][num] += 0.03;
    //         in.clear();
	//     }

   //////////////////////////////////////////////////////////////////
    //                  write output to txt                       //
    ///////////////////////////////////////////////////////////////

     base_name = argv[1];
    base_name.pop_back();
    base_name.pop_back();
    txt_capacitance.open("311510207_"+base_name+"_load.txt");
    for (int i = 0; i < gate.size(); i++) {
	    	txt_capacitance<< gate[i].name << " ";
            txt_capacitance <<fixed << setprecision(6) << gate[i].cap;
            txt_capacitance<< endl;
	}
      txt_capacitance.close();

    return 0;

}
