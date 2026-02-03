

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "transaction.h"
#include "utxo_manager.h"

using namespace std;

class Validator {
public:
    static pair<bool, string> validate(Transaction& tx, UTXOManager& utxo_manager, const set<pair<string, int>>& mempool_spent) {
        double input_sum = 0.0;
        double output_sum = 0.0;
        set<pair<string, int>> temp_spent_in_this_tx;

        for (const auto& in : tx.inputs) {
            if (!utxo_manager.exists(in.prev_tx, in.index)) {
                return {false, "REJECTED: Input UTXO does not exist."};
            }

            if (temp_spent_in_this_tx.count({in.prev_tx, in.index})) {
                return {false, "REJECTED: Simple double-spend detected (Same UTXO in one TX)."};
            }
            temp_spent_in_this_tx.insert({in.prev_tx, in.index});

            if (mempool_spent.count({in.prev_tx, in.index})) {
                return {false, "REJECTED: Mempool conflict. First-seen rule applied."};
            }

            UTXO u = utxo_manager.get_utxo(in.prev_tx, in.index);
            if (u.owner != in.owner) {
                return {false, "REJECTED: Auth Fail (Sender != Owner)."};
            }
            input_sum += u.amount;
        }

        for (const auto& out : tx.outputs) {
            if (out.amount < 0) return {false, "REJECTED: Negative output amount."};
            output_sum += out.amount;
        }

        if (input_sum < output_sum) return {false, "REJECTED: Insufficient funds."};

        tx.fee = input_sum - output_sum; 
        return {true, "VALID. Fee: " + to_string(tx.fee)};
    }
};


int main() {
    UTXOManager utxo_manager;
    utxo_manager.add_utxo("genesis", 0, 50.0, "Alice");
    utxo_manager.add_utxo("genesis", 1, 30.0, "Bob");

    cout << "\n--- Running Part 5: Double-Spending Prevention Tests ---\n" << endl;

    // Test Case 1: Simple Double-Spend Detection 
    cout << "[Test 1] Simple Double-Spend (Same TX):" << endl;
    vector<Input> inputs1 = { Input("genesis", 0, "Alice"), Input("genesis", 0, "Alice") };
    Transaction tx1(inputs1, { Output(10.0, "Bob") });
    auto res1 = Validator::validate(tx1, utxo_manager, {});
    cout << "Result: " << res1.second << "\n" << endl;

    // Test Case 2: Mempool Conflict Prevention
    cout << "[Test 2] Mempool Conflict (Different TXs):" << endl;
    set<pair<string, int>> mempool_spent = { {"genesis", 1} }; 
    Transaction tx2({Input("genesis", 1, "Bob")}, {Output(5.0, "Alice")});
    auto res2 = Validator::validate(tx2, utxo_manager, mempool_spent);
    cout << "Result: " << res2.second << "\n" << endl;

    // Test Case 3: Race Attack (First-Seen Rule)
    cout << "[Test 3] Race Attack (First-Seen Rule):" << endl;
    Transaction tx3({Input("genesis", 1, "Bob")}, {Output(1.0, "Alice")}); 
    auto res3 = Validator::validate(tx3, utxo_manager, mempool_spent);
    cout << "Result: " << res3.second << " (High fee ignored due to conflict)." << endl;

    return 0;
}
