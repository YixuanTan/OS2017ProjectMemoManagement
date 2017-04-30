//  OS2017
//  project2
//  By: Yixuan Tan, Zhengneng Chen, Wen Cai

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <set>
#include <limits.h>
using namespace std;

// --------------------
// data structures
// --------------------

struct Slice{
    int arr_time, run_time, p_index; // p_index is the index of this process in all_proc[]
    Slice(int arr_time_, int run_time_, int p_index_) : arr_time(arr_time_), run_time(run_time_), p_index(p_index_) {};
};

typedef vector<Slice>::iterator itr; // iterator for requests vector<> in struct Sliceocess

struct Process {
    char pid;
    int p_mem;
    vector<Slice> requests; // pair<> => {arrival time, run time}
};

string memo;
vector<Process> all_process_backup, all_procs;
int numOfFrame, framePerRow, numOfRows, accum_defrag_time, curr_time;

struct Comp {
    bool operator()(const itr& a, const itr& b) {
        return a->arr_time > b->arr_time || (a->arr_time == b->arr_time && all_procs[a->p_index].pid > all_procs[b->p_index].pid); // if a arrives later than b
    }
};


struct CompTermin {
    bool operator()(const pair<int, int>& a, const pair<int, int>& b) {
        //return a.first > b.first || (a.first == b.first && all_procs[a.second].pid > all_procs[b.second].pid);
        return a.first > b.first || (a.first == b.first && a.second > b.second);
    }
};


// --------------------
// utility functions
// --------------------
int count(const string::iterator& i, const string::iterator& j, char target) {
    int ans = 0;
    string::iterator i_cpy = i;
    while(i_cpy != j) {
        ans += (*i_cpy == target);
        i_cpy++;
    }
    return ans;
}

void reset() {
    memo = string(memo.length(), '.');
    accum_defrag_time = curr_time = 0;
    all_procs = all_process_backup;
}

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
            all_procs[i].requests.push_back(Slice(arr_time, run_time, i));
        }
    }
    read.close();
    all_process_backup = all_procs;
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


// --------------------
// algorithm functions
// --------------------
void nextFit() {
    reset();
    priority_queue<itr, vector<itr>, Comp> incoming_pq;
    priority_queue<pair<int, int>, vector< pair<int, int> >, CompTermin> terminating_pq;
    //priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> terminating_pq;
    int memo_ptr = 0;
    
    // initialize incoming_pq
    for(int i = 0; i < all_procs.size(); i++) {
        incoming_pq.push(all_procs[i].requests.begin());
    }
    cout << "time 0ms: Simulator started (Contiguous -- Next-Fit)" << endl;
    
    while(!incoming_pq.empty() || !terminating_pq.empty()) {
        if((!terminating_pq.empty() && terminating_pq.top().first <= incoming_pq.top()->arr_time) || incoming_pq.empty()) {
            curr_time = terminating_pq.top().first;
            char pid = all_procs[terminating_pq.top().second].pid;
            int start = (int)memo.find_first_of(pid);
            terminating_pq.pop();
            
            int end = (int)memo.find_first_not_of(pid, start);
            if(end == string::npos) {
                memo.replace(start, memo.length() - start, string(memo.length() - start, '.'));
            } else {
                memo.replace(start, end - start, string(end - start, '.'));
            }
            //memo_ptr = end;

            cout << "time " << curr_time << "ms: Process " << pid << " removed:" << endl;
            output();
        }
        else {
            itr top_slice = incoming_pq.top();
            incoming_pq.pop();
            
            //-------------------------------------------
            // NEXT FIT
            //-------------------------------------------
            // find the first place in memo to fit
            curr_time = top_slice->arr_time;
            int len_needed = all_procs[top_slice->p_index].p_mem;
            char pid = all_procs[top_slice->p_index].pid;
            cout << "time " << curr_time << "ms: Process " << pid << " arrived (requires " << len_needed << " frames)" << endl;

            int i = (int)memo.find_first_of('.', memo_ptr), j = i;  // two pointers, i is the begin of '.' sequene, j is the element passing the last of '.' sequence.
            int nextfit_pos = INT_MIN;
            while(i != string::npos && ((j = (int)memo.find_first_not_of('.', i)) != string::npos)) {
                int curr_len = j - i;
                if(curr_len >= len_needed) {
                    nextfit_pos = i;
                    break;
                }
                if((i = (int)memo.find_first_of('.', j)) == string::npos) break;
            }
            if(i != string::npos && j == string::npos) {
                int curr_len = (int)memo.length() - i;
                if(curr_len >= len_needed) {
                    nextfit_pos = i;
                }
            }

            if(nextfit_pos == INT_MIN) {
                i = j = 0;  // two pointers, i is the begin of '.' sequene, j is the element passing the last of '.' sequence.
                while(i != string::npos && ((j = (int)memo.find_first_not_of('.', i)) != string::npos)) {
                    int curr_len = j - i;
                    if(curr_len >= len_needed) {
                        nextfit_pos = i;
                        break;
                    }
                    if((i = (int)memo.find_first_of('.', j)) == string::npos) break;
                }
                if(i != string::npos && j == string::npos) {
                    int curr_len = (int)memo.length() - i;
                    if(curr_len >= len_needed) {
                        nextfit_pos = i;
                    }
                }
            }
                
                
            if(nextfit_pos != INT_MIN) {
                memo.replace(nextfit_pos, len_needed, string(len_needed, pid));
                memo_ptr = nextfit_pos + len_needed;
                cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                output();
                terminating_pq.push(make_pair(top_slice->arr_time + top_slice->run_time, top_slice->p_index));
            }
            else {
                // need to defragment
                // first check if it is possible to defragment
                set<char> defrag_set;
                if(len_needed > count(memo.begin(), memo.end(), '.')) {
                    cout << "time " << curr_time << "ms: Cannot place process " << pid << " -- skipped!" << endl;
                    output();
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
                    
                    curr_time += defrag_time;
                    accum_defrag_time += defrag_time;
                    
                    cout << "time " << curr_time << "ms: Defragmentation complete (moved " << defrag_time << " frames: ";
                    for(set<char>::iterator it = defrag_set.begin(); it != defrag_set.end(); it++) {
                        if(it != defrag_set.begin()) cout << ", " << *it;
                        else cout << *it;
                    }
                    cout << ")" << endl;
                    
                    output();
                    
                    memo.replace(i, len_needed, string(len_needed, pid));
                    memo_ptr = i + len_needed;
                    memo.replace(i + len_needed, memo.length() - i - len_needed, string(memo.length() - i - len_needed, '.'));
                    cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                    output();
                    terminating_pq.push(make_pair(top_slice->arr_time + top_slice->run_time, top_slice->p_index));
                    
                    
                    // update incoming_pq;
                    vector<itr> incoming_tmp;
                    while(!incoming_pq.empty()) {
                        incoming_pq.top()->arr_time += defrag_time;
                        incoming_tmp.push_back(incoming_pq.top());
                        incoming_pq.pop();
                    }
                    for(int k = 0; k < incoming_tmp.size(); k++) incoming_pq.push(incoming_tmp[k]); // re-push
                    
                    // update terminating_pq
                    vector< pair<int, int> > terminating_tmp;
                    while(!terminating_pq.empty()) {
                        int end_time = terminating_pq.top().first + defrag_time;
                        terminating_tmp.push_back(make_pair(end_time, terminating_pq.top().second));
                        terminating_pq.pop();
                    }
                    for(int k = 0; k < terminating_tmp.size(); k++) terminating_pq.push(terminating_tmp[k]);
                    
                }
            }
            // push in incoming_pq top() -> next
            if((top_slice + 1) != all_procs[top_slice->p_index].requests.end()) {
                // push in new process + accumulated defragmentation time
                (++top_slice)->arr_time += accum_defrag_time;
                incoming_pq.push(top_slice);
            }
            
        }
        
    }
    
    cout << "time " << curr_time << "ms: Simulator ended (Contiguous -- Next-Fit)" << endl;
}


void bestFit() {
    reset();
    priority_queue<itr, vector<itr>, Comp> incoming_pq;
    priority_queue< pair<int, int>, vector< pair<int, int> >, greater< pair<int, int> > > terminating_pq;
    
    
    // initialize incoming_pq
    for(int i = 0; i < all_procs.size(); i++) {
        incoming_pq.push(all_procs[i].requests.begin());
    }
    cout << "time 0ms: Simulator started (Contiguous -- Best-Fit)" << endl;
    
    while(!incoming_pq.empty() || !terminating_pq.empty()) {
        if((!terminating_pq.empty() && terminating_pq.top().first <= incoming_pq.top()->arr_time) || incoming_pq.empty()) {
            curr_time = terminating_pq.top().first;
            char pid = all_procs[terminating_pq.top().second].pid;
            int start = (int)memo.find_first_of(pid);
            terminating_pq.pop();
            
            int end = (int)memo.find_first_not_of(pid, start);
            if(end == string::npos) memo.replace(start, memo.length() - start, string(memo.length() - start, '.'));
            else memo.replace(start, end - start, string(end - start, '.'));
            
            cout << "time " << curr_time << "ms: Process " << pid << " removed:" << endl;
            output();
        }
        else {
            itr top_slice = incoming_pq.top();
            incoming_pq.pop();
            
            
            //-------------------------------------------
            // BEST FIT
            //-------------------------------------------
            // find the first place in memo to fit
            curr_time = top_slice->arr_time;
            int len_needed = all_procs[top_slice->p_index].p_mem;
            char pid = all_procs[top_slice->p_index].pid;
            cout << "time " << curr_time << "ms: Process " << pid << " arrived (requires " << len_needed << " frames)" << endl;
            
            int i = (int)memo.find_first_of('.'), j = i;  // two pointers, i is the begin of '.' sequene, j is the element passing the last of '.' sequence.
            int bestfit_pos = 0, best_len_sofar = INT_MAX;
            while((j = (int)memo.find_first_not_of('.', i + 1)) != string::npos) {
                int curr_len = j - i;
                if(curr_len >= len_needed && curr_len - len_needed < best_len_sofar - len_needed) {
                    bestfit_pos = i;
                    best_len_sofar = curr_len;
                }
                if((i = (int)memo.find_first_of('.', j + 1)) == string::npos) break;
            }
            if(i != string::npos && j == string::npos) {
                int curr_len = (int)memo.length() - i;
                if(curr_len >= len_needed && curr_len - len_needed < best_len_sofar - len_needed) {
                    bestfit_pos = i;
                    best_len_sofar = curr_len;
                }
            }
            
            if(best_len_sofar != INT_MAX) {
                memo.replace(bestfit_pos, len_needed, string(len_needed, pid));
                cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                output();
                terminating_pq.push(make_pair(top_slice->arr_time + top_slice->run_time, top_slice->p_index));
            }
            else {
                
                // need to defragment
                // first check if it is possible to defragment
                set<char> defrag_set;
                if(len_needed > count(memo.begin(), memo.end(), '.')) {
                    cout << "time " << curr_time << "ms: Cannot place process " << pid << " -- skipped!" << endl;
                    output();
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
                    
                    curr_time += defrag_time;
                    accum_defrag_time += defrag_time;
                    
                    cout << "time " << curr_time << "ms: Defragmentation complete (moved " << defrag_time << " frames: ";
                    for(set<char>::iterator it = defrag_set.begin(); it != defrag_set.end(); it++) {
                        if(it != defrag_set.begin()) cout << ", " << *it;
                        else cout << *it;
                    }
                    cout << ")" << endl;
                    
                    output();
                    
                    memo.replace(i, len_needed, string(len_needed, pid));
                    memo.replace(i + len_needed, memo.length() - i - len_needed, string(memo.length() - i - len_needed, '.'));
                    cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                    output();
                    terminating_pq.push(make_pair(top_slice->arr_time + top_slice->run_time, top_slice->p_index));
                    
                    
                    // update incoming_pq;
                    vector<itr> incoming_tmp;
                    while(!incoming_pq.empty()) {
                        incoming_pq.top()->arr_time += defrag_time;
                        incoming_tmp.push_back(incoming_pq.top());
                        incoming_pq.pop();
                    }
                    for(int k = 0; k < incoming_tmp.size(); k++) incoming_pq.push(incoming_tmp[k]); // re-push
                    
                    // update terminating_pq
                    vector< pair<int, int> > terminating_tmp;
                    while(!terminating_pq.empty()) {
                        int end_time = terminating_pq.top().first + defrag_time;
                        terminating_tmp.push_back(make_pair(end_time, terminating_pq.top().second));
                        terminating_pq.pop();
                    }
                    for(int k = 0; k < terminating_tmp.size(); k++) terminating_pq.push(terminating_tmp[k]);
                    
                }
            }
            // push in incoming_pq top() -> next
            if((top_slice + 1) != all_procs[top_slice->p_index].requests.end()) {
                // push in new process + accumulated defragmentation time
                (++top_slice)->arr_time += accum_defrag_time;
                incoming_pq.push(top_slice);
            }
            
        }
        
    }
    
    cout << "time " << curr_time << "ms: Simulator ended (Contiguous -- Best-Fit)" << endl;
}

void worstFit() {
    reset();
    priority_queue<itr, vector<itr>, Comp> incoming_pq;
    priority_queue< pair<int, int>, vector< pair<int, int> >, greater< pair<int, int> > > terminating_pq;
    
    // initialize incoming_pq
    for(int i = 0; i < all_procs.size(); i++) {
        incoming_pq.push(all_procs[i].requests.begin());
    }
    cout << "time 0ms: Simulator started (Contiguous -- Worst-Fit)" << endl;
    
    while(!incoming_pq.empty() || !terminating_pq.empty()) {
        if((!terminating_pq.empty() && terminating_pq.top().first <= incoming_pq.top()->arr_time) || incoming_pq.empty()) {
            curr_time = terminating_pq.top().first;
            char pid = all_procs[terminating_pq.top().second].pid;
            int start = (int)memo.find_first_of(pid);
            terminating_pq.pop();
            
            int end = (int)memo.find_first_not_of(pid, start);
            if(end == string::npos) memo.replace(start, memo.length() - start, string(memo.length() - start, '.'));
            else memo.replace(start, end - start, string(end - start, '.'));
            
            cout << "time " << curr_time << "ms: Process " << pid << " removed:" << endl;
            output();
        }
        else {
            itr top_slice = incoming_pq.top();
            incoming_pq.pop();
            
            
            //-------------------------------------------
            // WORST FIT
            //-------------------------------------------
            // find the first place in memo to fit
            curr_time = top_slice->arr_time;
            int len_needed = all_procs[top_slice->p_index].p_mem;
            char pid = all_procs[top_slice->p_index].pid;
            cout << "time " << curr_time << "ms: Process " << pid << " arrived (requires " << len_needed << " frames)" << endl;
            
            int i = (int)memo.find_first_of('.'), j = i;  // two pointers, i is the begin of '.' sequene, j is the element passing the last of '.' sequence.
            int bestfit_pos = 0, best_len_sofar = 0;
            while((j = (int)memo.find_first_not_of('.', i + 1)) != string::npos) {
                int curr_len = j - i;
                if(curr_len >= len_needed && curr_len - len_needed > best_len_sofar - len_needed) {
                    bestfit_pos = i;
                    best_len_sofar = curr_len;
                }
                if((i = (int)memo.find_first_of('.', j + 1)) == string::npos) break;
            }
            if(i != string::npos && j == string::npos) {
                int curr_len = (int)memo.length() - i;
                if(curr_len >= len_needed && curr_len - len_needed > best_len_sofar - len_needed) {
                    bestfit_pos = i;
                    best_len_sofar = curr_len;
                }
            }
            
            if(best_len_sofar != 0) {
                memo.replace(bestfit_pos, len_needed, string(len_needed, pid));
                cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                output();
                terminating_pq.push(make_pair(top_slice->arr_time + top_slice->run_time, top_slice->p_index));
            }
            else {
                
                // need to defragment
                // first check if it is possible to defragment
                set<char> defrag_set;
                if(len_needed > count(memo.begin(), memo.end(), '.')) {
                    cout << "time " << curr_time << "ms: Cannot place process " << pid << " -- skipped!" << endl;
                    output();
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
                    
                    curr_time += defrag_time;
                    accum_defrag_time += defrag_time;
                    
                    cout << "time " << curr_time << "ms: Defragmentation complete (moved " << defrag_time << " frames: ";
                    for(set<char>::iterator it = defrag_set.begin(); it != defrag_set.end(); it++) {
                        if(it != defrag_set.begin()) cout << ", " << *it;
                        else cout << *it;
                    }
                    cout << ")" << endl;
                    
                    output();
                    
                    memo.replace(i, len_needed, string(len_needed, pid));
                    memo.replace(i + len_needed, memo.length() - i - len_needed, string(memo.length() - i - len_needed, '.'));
                    cout << "time " << curr_time << "ms: Placed process " << pid << ":" << endl;
                    output();
                    terminating_pq.push(make_pair(top_slice->arr_time + top_slice->run_time, top_slice->p_index));
                    
                    
                    // update incoming_pq;
                    vector<itr> incoming_tmp;
                    while(!incoming_pq.empty()) {
                        incoming_pq.top()->arr_time += defrag_time;
                        incoming_tmp.push_back(incoming_pq.top());
                        incoming_pq.pop();
                    }
                    for(int k = 0; k < incoming_tmp.size(); k++) incoming_pq.push(incoming_tmp[k]); // re-push
                    
                    // update terminating_pq
                    vector< pair<int, int> > terminating_tmp;
                    while(!terminating_pq.empty()) {
                        int end_time = terminating_pq.top().first + defrag_time;
                        terminating_tmp.push_back(make_pair(end_time, terminating_pq.top().second));
                        terminating_pq.pop();
                    }
                    for(int k = 0; k < terminating_tmp.size(); k++) terminating_pq.push(terminating_tmp[k]);
                    
                }
            }
            // push in incoming_pq top() -> next
            if((top_slice + 1) != all_procs[top_slice->p_index].requests.end()) {
                // push in new process + accumulated defragmentation time
                (++top_slice)->arr_time += accum_defrag_time;
                incoming_pq.push(top_slice);
            }
            
        }
        
    }
    
    cout << "time " << curr_time << "ms: Simulator ended (Contiguous -- Worst-Fit)" << endl;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    numOfFrame = 256;
    framePerRow = 32;
    numOfRows = numOfFrame / framePerRow;
    
    // initialization
    memo.resize(numOfFrame, '.');
    
    readInput(argv[1]);
    
    nextFit();
    cout << endl;
    bestFit();
    cout << endl;
    worstFit();
    
    return 0;
}
