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
vector<string> memo;
int numOfFrame, framePerRow, numOfRows;

struct Process {
    char pid;
    int p_mem;
    vector<pair<int, int>> requests;
};

int str2int(string s) {
    int num = 0;
    for(unsigned int i = 0; i < s.length(); i++) {
        num = num * 10 + s[i] - '0';
    }
    return num;
}


bool readInput(vector<Process>& input, string inputfile){
    ifstream read(inputfile.c_str());
    if (!read.good()) {
        std::cerr << "Can not open " << inputfile << " to read." << std::endl;
        return 1;
    }
    
    string line, token;
    while(read.peek() == '#') {
        getline(read, line);
    }
    
    while(getline(read, line)) {
        stringstream ss(line);
        if(!getline(ss, token, '|')) return false;
        string proc_id = token;
        if(!getline(ss, token, '|')) return false;
        int ini_arr_time = str2int(token);
        if(!getline(ss, token, '|')) return false;
        int burst_time = str2int(token);
        if(!getline(ss, token, '|')) return false;
        int num_bursts = str2int(token);
        if(!getline(ss, token, '|')) return false;
        int io_time = str2int(token);
        input.push_back(Process(proc_id, ini_arr_time, burst_time, num_bursts, io_time));
    }
    
    read.close();
    return true;
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
