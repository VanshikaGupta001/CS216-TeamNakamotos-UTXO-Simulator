#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>

using namespace std;

struct UTXO {
    double amount;
    string owner;
};

class UTXOManager {
private:
    // Map key: <Transaction ID, Output Index>, Map value: <Amount, Owner>
    map<pair<string, int>, UTXO> utxo_set;

public:
    UTXOManager() {}

    void add_utxo(string tx_id, int index, double amount, string owner) {
        utxo_set[{tx_id, index}] = {amount, owner};
        cout << "Added UTXO: (" << tx_id << ", " << index << ") -> " 
             << amount << " BTC owned by " << owner << endl;
    }

    bool remove_utxo(string tx_id, int index) {
        if (utxo_set.count({tx_id, index})) {
            utxo_set.erase({tx_id, index});
            return true;
        }
        return false;
    }

    double get_balance(string owner) {
        double balance = 0.0;
        for (auto it : utxo_set) {
            if (it.second.owner == owner) {
                balance += it.second.amount;
            }
        }
        return balance;
    }
   
    bool exists(string tx_id, int index) {
        return utxo_set.count({tx_id, index}) > 0;
    }

    vector<pair<string, int>> get_utxos_for_owner(string owner) {
        vector<pair<string, int>> user_utxos;
        for (auto it : utxo_set) {
            if (it.second.owner == owner) {
                user_utxos.push_back(it.first);
            }
        }
        return user_utxos;
    }

    UTXO get_utxo(string tx_id, int index) {
        if (utxo_set.count({tx_id, index})) {
            return utxo_set[{tx_id, index}];
        }
        return {-1.0, ""}; 
    }

    void print_utxo_set() {
        cout << "\n=== Current UTXO Set ===" << endl;
        if (utxo_set.empty()) {
            cout << "No UTXOs available." << endl;
        } else {
            for (auto it : utxo_set) {
                cout << "Ref: (" << it.first.first << ", " << it.first.second << ") | "
                     << "Owner: " << it.second.owner << " | "
                     << "Amount: " << it.second.amount << " BTC" << endl;
            }
        }
        cout << "========================" << endl;
    }
};
