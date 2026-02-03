#ifndef MEMPOOL
#define MEMPOOL

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm> 
#include <map>


// #ifndef UTXO_MANAGER
// #define UTXO_MANAGER
#include"utxo_manager.h"
// #endif

// #ifndef TRANSACTION
// #define TRANSACTION
#include"transaction.h"
// #endif


using namespace std;

class Mempool{
    private:
    vector<Transaction>transactions;
    set<pair<string,int>> spent_utxos; //to keep track of utxos of txns in mempool 
    int max_size;

    public:
    Mempool(int size=50):max_size(size){}


    
  pair<bool, string> add_transaction(Transaction& tx, UTXOManager& utxo_manager) {
    // Check mempool capacity
    if (transactions.size() >= max_size) {
        return {false, "Mempool is full"};
    }

    // Check for duplicate transaction ID
    for (auto& it : transactions) {
        if (it.tx_id == tx.tx_id) {
            return {false, "Transaction ID already in mempool"};
        }
    }

    // ✅ CRITICAL FIX: Use Transaction::validate() 
    // This handles ALL validation including internal double-spending
    pair<bool, string> validation_result = tx.validate(utxo_manager, spent_utxos);
    
    if (!validation_result.first) {
        return validation_result;
    }

    // Add to mempool
    transactions.push_back(tx);

    // Track spent UTXOs
    for (const auto& input : tx.inputs) {
        spent_utxos.insert({input.prev_tx, input.index});
    }

    return {true, "Transaction added successfully. " + validation_result.second};
}




    void remove_transaction(string tx_id){
        auto it=transactions.begin();
        while(it!=transactions.end()){
            if(it->tx_id==tx_id){
//stop tracking it
                for (const auto& input : it->inputs) {
                    spent_utxos.erase({input.prev_tx, input.index});
                }

                transactions.erase(it);
                return;
            }
            ++it;
            }
        }




        vector<Transaction> get_top_transactions(int n){
vector<Transaction> sorted_txs = transactions;
sort(sorted_txs.begin(), sorted_txs.end(), [](const Transaction& a, const Transaction& b) {
            return a.fee > b.fee;
        });

        vector<Transaction> result;
        for (int i = 0; i < n && i < sorted_txs.size(); i++) {
            result.push_back(sorted_txs[i]);
        }
        return result;
    }



    void clear() {
        transactions.clear();
        spent_utxos.clear();
    }



    void print_status() {
        cout << "Mempool Status (" << transactions.size() << "/" << max_size << ")" << endl;
        for(const auto& tx : transactions) {
            cout << "TXID: " << tx.tx_id << " | Fee: " << tx.fee << endl;
        }
        
    }
};
#endif