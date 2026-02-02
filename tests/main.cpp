#include <iostream>
#include <vector>
#include <string>

#ifndef UTXO_MANAGER
#define UTXO_MANAGER
#include "../src/utxo_manager.h"
#endif

#ifndef MEMPOOL
#define MEMPOOL
#include"../src/mempool.h"
#endif

#ifndef TRANSACTION
#define TRANSACTION
#include"../src/transaction.h"
#endif

#ifndef MINER
#define MINER
#include "../src/miner.h"
#endif
using namespace std;

void initialize_genesis_block(UTXOManager& utxo_manager) {
    utxo_manager.add_utxo("genesis", 0, 50.0, "Alice");    
    utxo_manager.add_utxo("genesis", 1, 30.0, "Bob");     
    utxo_manager.add_utxo("genesis", 2, 20.0, "Charlie"); 
    utxo_manager.add_utxo("genesis", 3, 10.0, "David");    
    utxo_manager.add_utxo("genesis", 4, 5.0, "Eve");       
    cout << "Genesis UTXOs initialized successfully.\n";
}

int main() {
    srand(time(0)); // seed for random transaction IDs
    UTXOManager utxo_manager;
    Mempool mempool(50); // max size 50 
    
    initialize_genesis_block(utxo_manager);

    int choice;
    while (true) {
        cout << "\n=== Bitcoin Transaction Simulator ===" << endl;
        cout << "1. Create new transaction" << endl;
        cout << "2. View UTXO set" << endl;
        cout << "3. View mempool" << endl;
        cout << "4. Mine block" << endl;
        cout << "5. Run test scenarios" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            string sender, recipient;
            double amount;
            cout << "Enter sender: "; cin >> sender;
            cout << "Available balance: " << utxo_manager.get_balance(sender) << " BTC" << endl; 
            cout << "Enter recipient: "; cin >> recipient;
            cout << "Enter amount: "; cin >> amount;

            vector<pair<string, int>> available = utxo_manager.get_utxos_for_owner(sender);
            vector<Input> inputs;
            double total_input_val = 0;

            for (auto& utxo_ref : available) {
                UTXO u = utxo_manager.get_utxo(utxo_ref.first, utxo_ref.second);
                inputs.push_back(Input(utxo_ref.first, utxo_ref.second, sender));
                total_input_val += u.amount;
                if (total_input_val >= amount + 0.001) break; 
            }

            vector<Output> outputs;
            outputs.push_back(Output(amount, recipient));
            
            double fee = 0.001; 
            if (total_input_val > (amount + fee)) {
                outputs.push_back(Output(total_input_val - amount - fee, sender));
            }

            Transaction tx(inputs, outputs);
            pair<bool, string> result = mempool.add_transaction(tx, utxo_manager);
            
            if (result.first) {
                cout << "Transaction valid! " << result.second << endl; 
                cout << "Transaction ID: " << tx.tx_id << endl;
            } else {
                cout << "Transaction rejected: " << result.second << endl;
            }

        } else if (choice == 2) {
            utxo_manager.print_utxo_set(); 
        } else if (choice == 3) {
            mempool.print_status(); 
        } else if (choice == 4) {
            string miner;
            cout << "Enter miner name: "; cin >> miner; 
            mine_block(mempool, utxo_manager, miner); 
        } else if (choice == 5) {
            cout << "Running mandatory test scenarios..." << endl;
            //todo
        } else if (choice == 6) {
            break;
        } else {
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
}