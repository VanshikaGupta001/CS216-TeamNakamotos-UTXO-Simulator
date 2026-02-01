#include <string>
#include <vector>
#include <ctime>  
#include <cstdlib> 

using namespace std;

struct Input {
    string prev_tx;  
    int index;      
    string owner;  
};

struct Output {
    double amount;   
    string address;  
};

struct Transaction {
    string tx_id;
    vector<Input> inputs;
    vector<Output> outputs;
    double fee;

    Transaction() : fee(0.0) {}
};

string generate_tx_id() {
    return "txn_" + to_string(time(0)) + "_" + to_string(rand() % 9000 + 1000);
}