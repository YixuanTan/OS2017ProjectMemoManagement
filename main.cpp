//
//  OS2017
//  project2
//
//

#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

using namespace std;

struct Process {
    char pid;
    int p_mem;
    vector<pair<int, int>> requests;
};

vector<string> memo;
vector<Process> all_procs;
int numOfFrame, framePerRow, numOfRows, accum_defrag_time;



int str2int(string s) {
    int num = 0;
    for(unsigned int i = 0; i < s.length(); i++) {
        num = num * 10 + s[i] - '0';
    }
    return num;
}


void readInput(string inputfile){
    ifstream read(inputfile.c_str());
    if (!read.good()) {
        cerr << "Can not open " << inputfile << " to read." << endl;
        return;
    }
    
    string line, token, word;
    while(read.peek() == '#') {
        getline(read, line);
    }
    
    getline(read, line, '\n');
    int num_of_procs = str2int(line);
    if(num_of_procs > 26) {
        cerr << "Max num of simulation allowed is 26" << endl;
    }
    all_procs.resize(num_of_procs);
    
    for(int i = 0; i < num_of_procs; i++) {
        getline(read, line, '\n');
        stringstream thisline(line);
        getline(thisline, word, ' ');
        all_procs[i].pid = word[0];
        getline(thisline, word, ' ');
        all_procs[i].p_mem = str2int(word);
        while(getline(thisline, word, ' ')) {
            int split_pos = (int)word.find_first_of('/');
            int arr_time = str2int(word.substr(0, split_pos));
            int run_time = str2int(word.substr(split_pos + 1));
            all_procs[i].requests.push_back({arr_time, run_time});
        }
    }
    read.close();
    return;
}

void output() {
    
    cout << string(numOfFrame, '=') << endl;
    for(int i = 0; i < numOfRows; i++) {
        cout << memo[i] << endl;
    }
    cout << string(numOfFrame, '=') << endl;
}
int main(int argc, const char * argv[]) {
    // insert code here...
    numOfFrame = 256;
    framePerRow = 32;
    numOfRows = numOfFrame / framePerRow;
    
    // initialization
    memo.resize(numOfRows, string(numOfFrame, '.'));
    
    readInput(argv[1]);
    
    return 0;
}
