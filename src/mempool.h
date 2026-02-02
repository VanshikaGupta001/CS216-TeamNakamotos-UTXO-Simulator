#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm> 
#include <map>
#include "utxo_manager.h"
#include "transaction.h"

using namespace std;

class Mempool{
    private:
    vector<Transaction>transactions;
    set<pair<string,int>> spent_utxos; //to keep track of utxos of txns in mempool 
    int max_size;

    public:
    Mempool(int size=50):max_size(size){}


    
    pair<bool,string> add_transaction(Transaction tx,UTXOManager& utxo_manager){
        if(transactions.size()>=max_size){
            return{false,"Mempool is full"};
        }

        for(auto &it:transactions){
            if(it.tx_id==tx.tx_id){
                return {false,"transaction id aldready in mempool"};
            }
        }

        double input_sum=0.0;
        double output_sum=0.0;

        for(auto &input:tx.inputs){
            if(!utxo_manager.exists(input.prev_tx,input.index)){
                return{false,"input utxo does not exist in utxo set"};
            }

            if(spent_utxos.count({input.prev_tx,input.index})){
                return{false,"utxo aldready spent in mempool, double spending attempt."};
            }
            UTXO u = utxo_manager.get_utxo(input.prev_tx, input.index);
            if(u.owner!=input.owner){
                return {false, "not owner of input UTXO"};
            }
            input_sum+=u.amount;

        }

        for (const auto& output : tx.outputs) {
            if (output.amount < 0) return {false, "Invalid output amount"};
            output_sum += output.amount;
        }

        if (input_sum < output_sum) {
            return {false, "insufficient funds: Inputs < Outputs"};
        }

        tx.fee = input_sum - output_sum;
        transactions.push_back(tx);

        for (const auto& input : tx.inputs) {
            spent_utxos.insert({input.prev_tx, input.index});
        }

        return {true, "Transaction added successfully. Fee: " + to_string(tx.fee)};

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