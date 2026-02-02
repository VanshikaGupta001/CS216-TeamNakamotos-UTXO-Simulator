#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm> 
#include <map>
#include <ctime>
#include <cstdlib>

#ifndef UTXO_MANAGER
#define UTXO_MANAGER
#include"utxo_manager.h"
#endif

#ifndef MEMPOOL
#define MEMPOOL
#include"mempool.h"
#endif

#ifndef TRANSACTION
#define TRANSACTION
#include"transaction.h"
#endif


using namespace std;

string generate_tx_id() {
    return "tx_" + to_string(time(0)) + "_" + to_string(rand() % 9000 + 1000);
}

void mine_block(Mempool& mempool, UTXOManager& utxo_manager, string miner_address) {
    cout << "\n---------------- MINING START ----------------" << endl;
    
    const int BLOCK_SIZE = 3;
    const double BLOCK_REWARD = 12.5;

    vector<Transaction> txs_to_mine = mempool.get_top_transactions(BLOCK_SIZE);

    if (txs_to_mine.empty()) {
        cout << "Mempool empty. No transactions to mine." << endl;
        cout << "---------------- MINING END ------------------" << endl;
        return;
    }

    double total_fees = 0.0;
    int mined_count = 0;

    cout << "Miner: " << miner_address << endl;
    cout << "Attempting to mine " << txs_to_mine.size() << " transactions..." << endl;

    for (const auto& tx : txs_to_mine) {
        bool inputs_valid = true;
        for(const auto& input : tx.inputs) {
            if(!utxo_manager.exists(input.prev_tx, input.index)) {
                inputs_valid = false;
                cerr << "  [!] Error: UTXO missing for TX " << tx.tx_id << ". Skipping." << endl;
                break;
            }
        }

        if(!inputs_valid) {
            mempool.remove_transaction(tx.tx_id); 
            continue; 
        }

        for (const auto& input : tx.inputs) {
            utxo_manager.remove_utxo(input.prev_tx, input.index);
        }

        int output_index = 0;
        for (const auto& output : tx.outputs) {
            utxo_manager.add_utxo(tx.tx_id, output_index, output.amount, output.address);
            output_index++;
        }

        total_fees += tx.fee;
        mempool.remove_transaction(tx.tx_id);
        cout << "  -> Mined TX: " << tx.tx_id << " (Fee: " << tx.fee << ")" << endl;
        mined_count++;
    }

    double total_payout = BLOCK_REWARD + total_fees;
    string coinbase_tx_id = "coinbase_" + to_string(time(0)) + "_" + to_string(rand() % 1000);
    
    // Add reward to UTXO set owned by miner
    utxo_manager.add_utxo(coinbase_tx_id, 0, total_payout, miner_address);

    cout << "\nBlock Mined Successfully!" << endl;
    cout << "  Transactions: " << mined_count << endl;
    cout << "  Block Reward: " << BLOCK_REWARD << endl;
    cout << "  Total Fees:   " << total_fees << endl;
    cout << "  Miner Payout: " << total_payout << " -> " << miner_address << endl;
    cout << "---------------- MINING END ------------------" << endl;
}
