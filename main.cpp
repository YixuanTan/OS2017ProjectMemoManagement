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
#include <queue>
#include <set>
using namespace std;

struct Slice{
    int arr_time, run_time, p_index; // p_index is the index of this process in all_proc[]
};

typedef vector<Slice>::iterator itr; // iterator for requests vector<> in struct Sliceocess

struct Process {
    char pid;
    int p_mem;
    vector<Slice> requests; // pair<> => {arrival time, run time}
};

struct NextFit {
    bool operator()(const itr& a, const itr& b) {
        return a->arr_time > b->arr_time || (a->arr_time == b->arr_time && a->p_index > b->p_index); // if a arrives later than b
    }
};
    
string memo;
vector<Process> all_procs;
int numOfFrame, framePerRow, numOfRows, accum_defrag_time, curr_time;

int str2int(string s) {
    int num = 0;
    for(unsigned int i = 0; i < s.length(); i++) {
        num = num * 10 + s[i] - '0';
    }
    return num;
}

struct ProcessTerminator {
    int end_time;
    int len;
    int pos;
};

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
            all_procs[i].requests.push_back({arr_time, run_time, i});
        }
    }
    read.close();
    return;
}

void output() {
    
    cout << string(framePerRow, '=') << endl;
    int i = 0;
    while(i < memo.length()) {
        cout << memo.substr(i, framePerRow) << endl;
        i += framePerRow;
    }
    cout << string(framePerRow, '=') << endl;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    numOfFrame = 256;
    framePerRow = 32;
    numOfRows = numOfFrame / framePerRow;
    
    // initialization
    memo.resize(numOfRows * numOfFrame, '.');
    
    readInput(argv[1]);
    
    priority_queue<itr, vector<itr>, NextFit> incoming_pq;
    priority_queue<pair<int, char>, vector<pair<int, char>>, greater<pair<int, char>>> terminating_pq;
    
    // initialize incoming_pq
    for(int i = 0; i < all_procs.size(); i++) incoming_pq.push(all_procs[i].requests.begin());
    
    
    while(!incoming_pq.empty() || !terminating_pq.empty()) {
        if(!terminating_pq.empty() && terminating_pq.top().first <= incoming_pq.top()->arr_time) {
            char pid = terminating_pq.top().second;
            int start = (int)memo.find_first_of(pid);
            terminating_pq.pop();
            
            int end = (int)memo.find_first_not_of(start, pid);
            if(end == string::npos) memo.replace(start, memo.length(), string(memo.length() - start, '.'));
            else memo.replace(start, end, string(end - start, '.'));
        }
        else {
            itr top_slice = incoming_pq.top();
            incoming_pq.pop();
            
            
            //-------------------------------------------
            // NEXT FIT
            //-------------------------------------------
            // find the first place in memo to fit
            int len_needed = all_procs[top_slice->p_index].p_mem;
            char pid = all_procs[top_slice->p_index].pid;
            cout << "time " << curr_time << "ms: Process " << pid << " arrived (requires " << len_needed << " frames)" << endl;
            
            int i = (int)memo.find_first_of('.'), j = i;  // two pointers, i is the begin of '.' sequene, j is the element passing the last of '.' sequence.
            int nextfit_pos = 0, best_len_sofar = INT_MAX;
            while((j = (int)memo.find_first_not_of(i + 1, '.')) != string::npos) {
                int curr_len = j - i;
                if(curr_len >= len_needed && curr_len - len_needed < best_len_sofar - len_needed) {
                    nextfit_pos = i;
                    best_len_sofar = curr_len;
                }
                if((i = (int)memo.find_first_of(j + 1, '.')) == string::npos) break;
            }
            if(i != string::npos && j == string::npos) {
                int curr_len = (int)memo.length() - i;
                if(curr_len >= len_needed && curr_len - len_needed < best_len_sofar - len_needed) {
                    nextfit_pos = i;
                    best_len_sofar = curr_len;
                }
            }
            
            if(best_len_sofar != INT_MAX) {
                memo.replace(nextfit_pos, len_needed, string(len_needed, pid));
                cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                output();
            }
            else {
                
                // need to defragment
                // first check if it is possible to defragment
                set<char> defrag_set;
                if(len_needed > count(memo.begin(), memo.end(), '.')) {
                    cout << "time " << curr_time << "ms: Cannot place process " << pid << " -- skipped" << endl;
                } else {
                    cout << "time " << curr_time << "ms: Cannot place process " << pid << " -- starting defragmentation" << endl;
                    int defrag_time = 0, i = 0, j = 0;
                    while(j < memo.length()) {
                        if(memo[j] == '.') {
                            j++;
                        } else if(i != j){
                            defrag_set.insert(memo[j]);
                            swap(memo[i++], memo[j++]);
                            defrag_time++;
                        } else {
                            i++;
                            j++;
                        }
                    }
                    cout << "time " << curr_time << "ms: Defragmentation complete (moved " << defrag_time << " frames: ";
                    for(set<char>::iterator it = defrag_set.begin(); it != defrag_set.end(); it++) {
                        if(it != defrag_set.begin()) cout << ", " << *it;
                        else cout << *it << endl;
                    }
                    cout << ")" << endl;
                    
                    memo.replace(i, i + len_needed, string(len_needed, pid));
                    memo.replace(i + len_needed, memo.length(), string(memo.length() - i - len_needed, '.'));
                    cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                    output();
                    
                    curr_time += defrag_time;
                    accum_defrag_time += defrag_time;
                    
                    // update incoming_pq;
                    vector<itr> incoming_tmp;
                    while(!incoming_pq.empty()) {
                        incoming_pq.top()->arr_time += defrag_time;
                        incoming_tmp.push_back(incoming_pq.top());
                        incoming_pq.pop();
                    }
                    for(int k = 0; k < incoming_tmp.size(); k++) incoming_pq.push(incoming_tmp[k]); // re-push
                    
                    // update terminating_pq
                    vector<pair<int, int>> terminating_tmp;
                    while(!terminating_pq.empty()) {
                        int end_time = terminating_pq.top().first + defrag_time;
                        terminating_tmp.push_back(make_pair(end_time, pid));
                        terminating_pq.pop();
                    }
                    for(int k = 0; k < terminating_tmp.size(); k++) terminating_pq.push(terminating_tmp[k]);
                    
                }
            }
            // push in incoming_pq top() -> next
            if(++top_slice != all_procs[top_slice->p_index].requests.end()) {
                // push in new process + accumulated defragmentation time
                top_slice->arr_time += accum_defrag_time;
                incoming_pq.push(top_slice);
            }

        }
        
    }
    
    cout << "time " << curr_time << "ms: Simulator ended (Contiguous -- Next-Fit)" << endl;
    return 0;
}
