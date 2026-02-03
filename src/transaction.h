
#ifndef TRANSACTION
#define TRANSACTION

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <ctime>
#include <cstdlib>


#include"utxo_manager.h"


using namespace std;


struct Input {
    string prev_tx;
    int index;
    string owner;

    Input(string p, int i, string o) : prev_tx(p), index(i), owner(o) {}
};

struct Output {
    double amount;
    string address;

    Output(double a, string addr) : amount(a), address(addr) {}
};

struct Transaction {
    string tx_id;
    vector<Input> inputs;
    vector<Output> outputs;
    double fee;

    Transaction() : fee(0.0) {}
    Transaction(vector<Input> ins, vector<Output> outs) 
        : inputs(ins), outputs(outs), fee(0.0) {
        tx_id = "tx_" + to_string(time(0)) + "_" + to_string(rand() % 9000 + 1000);
    }

    static Transaction create(string sender, string recipient, double amount, UTXOManager& utxoManager) {
        vector<pair<string, int>> user_utxos = utxoManager.get_utxos_for_owner(sender);
        
        vector<Input> inputs;
        double current_sum = 0.0;
        double fee = 0.0; 
        
        // Coin Selection 
        for (auto& ref : user_utxos) {
            UTXO u = utxoManager.get_utxo(ref.first, ref.second);
            inputs.emplace_back(ref.first, ref.second, sender);
            current_sum += u.amount;
            
            // Stop if we have enough to cover Amount + Fee
            if (current_sum >= amount + fee) break;
        }

        // Validate 
        if (current_sum < amount + fee) {
            cout << "Error: Insufficient! Available: " << current_sum 
                 << " | Required: " << (amount + fee) << endl;
            return Transaction(); // Return invalid tx
        }

        // Create Outputs
        vector<Output> outputs;
        outputs.emplace_back(amount, recipient); //  Payment

        // Handle Change
        double change = current_sum - amount - fee;
        if (change > 0.000001) { // Avoid tiny floating point errors
            outputs.emplace_back(change, sender);
        }

        return Transaction(inputs, outputs);
    }

    // {isValid, ErrorMessage}
    pair<bool, string> validate(UTXOManager& utxoManager, const set<pair<string, int>>& mempool_spent) {
        
        double input_sum = 0.0;
        double output_sum = 0.0;
        set<pair<string, int>> temp_spent_in_this_tx; 

        for (const auto& in : inputs) {
            
            // All inputs must exist in UTXO set 
            if (!utxoManager.exists(in.prev_tx, in.index)) {
                return {false, "Rule 1 Fail: Input UTXO does not exist in UTXO set."};
            }

            // No double-spending in inputs (Same UTXO twice in THIS tx) 
            if (temp_spent_in_this_tx.count({in.prev_tx, in.index})) {
                return {false, "Rule 2 Fail: Internal double-spend detected (same UTXO used twice)."};
            }
            temp_spent_in_this_tx.insert({in.prev_tx, in.index});

            // No conflict with mempool (UTXO not already spent in unconfirmed tx)
            if (mempool_spent.count({in.prev_tx, in.index})) {
                return {false, "Rule 5 Fail: UTXO is already pending in Mempool."};
            }

            // Verify Owner(Signature check)
            UTXO u = utxoManager.get_utxo(in.prev_tx, in.index);
            if (u.owner != in.owner) {
                return {false, "Auth Fail: Sender is not the owner of the UTXO."};
            }

            input_sum += u.amount;
        }

        for (const auto& out : outputs) {
            //No negative amounts in outputs 
            if (out.amount < 0) {
                return {false, "Rule 4 Fail: Negative output amount detected."};
            }
            output_sum += out.amount;
        }

        // Sum(inputs) >= Sum(outputs)
        if (input_sum < output_sum) {
            return {false, "Rule 3 Fail: Insufficient funds (Inputs < Outputs)."};
        }

        // If all checks aree correct, calculate fee
        this->fee = input_sum- output_sum;
        return {true, "Valid. Fee: " + to_string(this->fee)};
    }

    void print_details() const {
        cout << "transaction = {" << endl;
        cout << "  \"tx_id\": \"" << tx_id << "\"," << endl;
        
        cout << "  \"inputs\": [" << endl;
        for (size_t i = 0; i < inputs.size(); ++i) {
            cout << "    {" << endl;
            cout << "      \"prev_tx\": \"" << inputs[i].prev_tx << "\"," << endl;
            cout << "      \"index\": " << inputs[i].index << "," << endl;
            cout << "      \"owner\": \"" << inputs[i].owner << "\"" << endl;
            cout << "    }" << (i < inputs.size() - 1 ? "," : "") << endl;
        }
        cout << "  ]," << endl;

        cout << "  \"outputs\": [" << endl;
        for (size_t i = 0; i < outputs.size(); ++i) {
            cout << "    {" << endl;
            cout << "      \"amount\": " << outputs[i].amount << "," << endl;
            cout << "      \"address\": \"" << outputs[i].address << "\"" << endl;
            cout << "    }" << (i < outputs.size() - 1 ? "," : "") << endl;
        }
        cout << "  ]" << endl;
        cout << "}" << endl;
    }
};

#endif