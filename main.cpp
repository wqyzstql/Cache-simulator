#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <map>
#include <queue>
#include <fstream>

using namespace std;

struct Operation{
    bool op; // op == 0 read, op == 1 write; 
    int tar, idx; // tar is the target core of this operation
    long long addr;
    string origin_add;
    bool operator < (const Operation &x) const{
        return idx < x.idx;
    }
    Operation(){
        this->op = 0;
        this->addr = 0;
    }
    Operation(bool op, int tar, int idx, string add){
        this->op = op;
        this->tar = tar;
        this->idx = idx;
        int len = add.size();
        long long base = 1, addr = 0;
        for(int i=len - 1; i >= 0; i--){
            char x = add[i];
            int val;
            if(x >= 'A' && x <= 'Z')
                x = x - 'A' + 'a';
            if(x >= 'a' && x <= 'z')
                val = (int)x - 'a' + 10;
            if( x>= '0' && x <= '9')
                val = (int)x - '0';
            addr += base * val;
            base *= 16;
        }
        this->addr = addr;
        this->origin_add = add;
    }
};
vector <Operation> v;
enum Status{
    M,
    E,
    S,
    I
};
char status_char[] = {'M', 'E', 'S', 'I'};
struct Core{
    long long cache_start;
    Status status;
    Core(){}
    Core(long long start, Status status){
        this->cache_start = start;
        this->status = status;
    }
}core[2];
void Deal(Operation op){
	cout<<string(50,'-')<<endl;
    int op_core = op.tar;
    //cout<<op.idx<<" ";
    cout<<"Currently operating on core "<< op_core << endl;
    cout<<"Currently operating address is "<< op.origin_add << ", Operating type is ";
    if(op.op == 0)  cout<<"read.\n";
    else cout<<"write.\n";
    long long cache_start = op.addr / 64 * 64;
    core[op_core].cache_start = cache_start;
    char s_0 = status_char[core[0].status], s_1 = status_char[core[1].status];
    if(core[op_core ^ 1].cache_start == cache_start){ // this segment of memory is occupyed by other core
        if(op.op == 0){ //read
            core[op_core ^ 1].status = S;
            core[op_core].status = S;
        }else {
            core[op_core ^ 1].status = I;
            core[op_core ^ 1].cache_start = -1;
            core[op_core].status = M;
        }
    }else {//this cache line's status is I by default
        core[op_core].status = E; //read
        if(op.op == 1){
            core[op_core].status = M; //write
        }
    }
	char fs_0 = status_char[core[0].status], fs_1 = status_char[core[1].status];
	if(core[0].cache_start != -1)
	cout<<"Core 0's cache save the memory from address "
		<<core[0].cache_start<<" to address "<<(core[0].cache_start+64)<<endl;
	else cout<<"There are no memory in Core 0's cache!\n";
	if(core[1].cache_start != -1)
	cout<<"Core 1's cache save the memory from address "
		<<core[1].cache_start<<" to address "<<(core[1].cache_start+64)<<endl;
	else cout<<"There are no memory in Core 1's cache!\n";
	cout<<"Core 0's status from "<<s_0<<" to "<<fs_0<<endl;
    cout<<"Core 1's status from "<<s_1<<" to "<<fs_1<<endl;
	return;
}

int main(){
    bool op;
    int idx = 0;
    string add;
    ifstream file0("trace0.txt");
    while(file0 >> op){
        file0 >> add;
        v.push_back(Operation(op, 0, idx++, add));
    }
    file0.close();
    ifstream file1("trace1.txt");
    idx=0;
    while(file1 >> op){
        file1 >> add;
        v.push_back(Operation(op, 1, idx++, add));
    }
    file1.close();
    //memory address used Byte, a cache line's length is 64B, 0040H
    core[0] = Core(-1, I);
    core[1] = Core(-1, I);
    sort(v.begin(), v.end());
    for(int i = 0; i < v.size(); i++)
        Deal(v[i]);
    return 0;
}
