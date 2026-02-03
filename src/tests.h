#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath> 
#include <iomanip>

#include "utxo_manager.h"
#include "mempool.h"
#include "transaction.h"
#include "miner.h"

using namespace std;


bool is_equal(double a, double b) {
    return std::abs(a - b) < 0.000001;
}

void log_test(int id, string name, bool passed, string failure_reason = "") {
    if (passed) 
        cout << "[PASS] Test " << id << ": " << name << endl;
    
    else 
        cout << "[FAIL] Test " << id << ": " << name << "\n       Reason: " << failure_reason << endl;
    

}

void reset_genesis(UTXOManager& um) {
    std::ios_base::iostate original_state = std::cout.rdstate();
    std::cout.setstate(std::ios_base::failbit); // Disable cout

    um = UTXOManager(); 
    um.add_utxo("genesis", 0, 50.0, "Alice");    
    um.add_utxo("genesis", 1, 30.0, "Bob");     
    um.add_utxo("genesis", 2, 20.0, "Charlie"); 
    um.add_utxo("genesis", 3, 10.0, "David");    
    um.add_utxo("genesis", 4, 5.0, "Eve");       

    std::cout.clear(); // Enable cout
}

Transaction create_test_tx(UTXOManager& um, string sender, string recipient, double amount, double fee) {
    vector<pair<string, int>> available = um.get_utxos_for_owner(sender);
    vector<Input> inputs;
    double total_input_val = 0;

    for (auto& utxo_ref : available) {
        UTXO u = um.get_utxo(utxo_ref.first, utxo_ref.second);
        inputs.push_back(Input(utxo_ref.first, utxo_ref.second, sender));
        total_input_val += u.amount;
        if (total_input_val >= amount + fee) break;
    }

    vector<Output> outputs;
    outputs.push_back(Output(amount, recipient));
    
    if (total_input_val > (amount + fee)) {
        outputs.push_back(Output(total_input_val - amount - fee, sender));
    }
    
    return Transaction(inputs, outputs);
}


void run_test_scenarios() {
    cout << "\n============================================\n";
    cout << "      RUNNING 10 STRICT TEST SCENARIOS      \n";
    cout << "============================================\n";

 
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        Transaction tx = create_test_tx(um, "Alice", "Bob", 10.0, 0.001);
        pair<bool, string> res = mp.add_transaction(tx, um);
        
        // Validation Logic
        bool accepted = res.first;
        bool correct_fee = is_equal(tx.fee, 0.001);
        bool has_change = (tx.outputs.size() == 2); 
        
        // Dynamic Output
        cout << "Test 1 Data: Fee=" << tx.fee << " | Outputs=" << tx.outputs.size() << " | Status=" << (accepted ? "OK" : "REJ") << endl;

        if (accepted && correct_fee && has_change) {
            log_test(1, "Basic Valid Transaction", true);
        } else {
            log_test(1, "Basic Valid Transaction", false, "Missing change output or incorrect fee");
        }
        cout<<endl;
    }

    // Test 2: Multiple Inputs
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        // Setup: Give Alice 20 more
        Transaction t1 = create_test_tx(um, "Charlie", "Alice", 20.0, 0.0);
        mp.add_transaction(t1, um);
        
        // Silence mining
        std::cout.setstate(std::ios_base::failbit);
        mine_block(mp, um, "Miner1"); 
        std::cout.clear();

        // Actual Test: Alice sends 60
        Transaction t2 = create_test_tx(um, "Alice", "Bob", 60.0, 0.001);
        pair<bool, string> res = mp.add_transaction(t2, um);

        cout << "Test 2 Data: Inputs Used=" << t2.inputs.size() << " | Amount=" << 60.0 << endl;

        if (res.first && t2.inputs.size() >= 2) {
            log_test(2, "Multiple Inputs Aggregation", true);
        } else {
            log_test(2, "Multiple Inputs Aggregation", false, "Failed to aggregate inputs");
        }
            cout<<endl;
    }

    // Test 3: Double-Spend in Same Transaction
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        vector<pair<string, int>> utxos = um.get_utxos_for_owner("Alice");
        if(utxos.empty()) { log_test(3, "Double-Spend in Same TX", false, "Setup Error"); }
        else {
            Input in1(utxos[0].first, utxos[0].second, "Alice");
            
            // Construct TX with DUPLICATE inputs manually
            vector<Input> bad_inputs; 
            bad_inputs.push_back(in1); 
            bad_inputs.push_back(in1);
            
            vector<Output> outputs; 
            outputs.push_back(Output(10.0, "Bob"));

            Transaction tx(bad_inputs, outputs);
            pair<bool, string> res = mp.add_transaction(tx, um);
            
            cout << "Test 3 Data: Result=" << (res.first ? "Accepted" : "Rejected") << " | Msg=\"" << res.second << "\"" << endl;

            if (!res.first) {
                log_test(3, "Double-Spend in Same Transaction", true);
            } else {
                log_test(3, "Double-Spend in Same Transaction", false, "System accepted duplicate inputs!");
            }
        }
            cout<<endl;
    }

    // Test 4: Mempool Double-Spend
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        Transaction tx1 = create_test_tx(um, "Alice", "Bob", 50.0, 0.0);
        Transaction tx2 = create_test_tx(um, "Alice", "Charlie", 50.0, 0.0);
        
        bool r1 = mp.add_transaction(tx1, um).first;
        pair<bool, string> r2 = mp.add_transaction(tx2, um);
        
        cout << "Test 4 Data: TX1=" << r1 << " | TX2=" << r2.first << " | Error=\"" << r2.second << "\"" << endl;

        if (r1 && !r2.first) {
            log_test(4, "Mempool Double-Spend", true);
        } else {
            log_test(4, "Mempool Double-Spend", false, "Failed to detect conflict in mempool");
        }
            cout<<endl;
    }

    // Test 5: Insufficient Funds
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        Transaction tx = create_test_tx(um, "Bob", "Alice", 35.0, 0.001);
        tx.outputs[0].amount = 35.0; // Force amount > balance (30)
        
        pair<bool, string> res = mp.add_transaction(tx, um);
        
        cout << "Test 5 Data: Result=" << res.first << " | Msg=\"" << res.second << "\"" << endl;

        if (!res.first && (res.second.find("nsufficient") != string::npos || res.second.find("funds") != string::npos)) {
            log_test(5, "Insufficient Funds", true);
        } else {
            log_test(5, "Insufficient Funds", false, "Wrong error message or transaction accepted");
        }
            cout<<endl;
    }

    // Test 6: Negative Amount
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        vector<pair<string, int>> utxos = um.get_utxos_for_owner("Alice");
        vector<Input> inputs; 
        inputs.push_back(Input(utxos[0].first, utxos[0].second, "Alice"));
        
        vector<Output> outputs; 
        outputs.push_back(Output(-5.0, "Bob")); // Negative

        Transaction tx(inputs, outputs);
        pair<bool, string> res = mp.add_transaction(tx, um);
        
        cout << "Test 6 Data: Result=" << res.first << " | Msg=\"" << res.second << "\"" << endl;

        if (!res.first) log_test(6, "Negative Amount", true);
        else log_test(6, "Negative Amount", false, "Accepted negative output");
            cout<<endl;
    }

    // Test 7: Zero Fee Transaction
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        vector<pair<string, int>> utxos = um.get_utxos_for_owner("Alice");
        vector<Input> inputs; 
        inputs.push_back(Input(utxos[0].first, utxos[0].second, "Alice"));
        
        vector<Output> outputs; 
        outputs.push_back(Output(50.0, "Bob")); // Exact match

        Transaction tx(inputs, outputs);
        pair<bool, string> res = mp.add_transaction(tx, um);
        
        cout << "Test 7 Data: Fee=" << tx.fee << " | Result=" << res.first << endl;

        if (res.first && is_equal(tx.fee, 0.0)) log_test(7, "Zero Fee Transaction", true);
        else log_test(7, "Zero Fee Transaction", false, "Fee not zero or rejected");
            cout<<endl;
    }

    // Test 8: Race Attack Simulation
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        Transaction tx1 = create_test_tx(um, "Alice", "Bob", 50.0, 0.0);
        Transaction tx2 = create_test_tx(um, "Alice", "Eve", 50.0, 0.0);
        
        bool r1 = mp.add_transaction(tx1, um).first;
        pair<bool, string> r2 = mp.add_transaction(tx2, um);
        
        cout << "Test 8 Data: TX1(LowFee)=" << r1 << " | TX2(HighFee)=" << r2.first << endl;

        if (r1 && !r2.first) log_test(8, "Race Attack (First-Seen Rule)", true);
        else log_test(8, "Race Attack", false, "First-seen rule failed");
            cout<<endl;
    }

    // Test 9: Complete Mining Flow
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
       Transaction tx1 = create_test_tx(um, "Alice", "Bob", 10.0, 0.0);
        mp.add_transaction(tx1, um);
         Transaction tx2 = create_test_tx(um, "Charlie", "David", 5.0, 0.0);
        mp.add_transaction(tx2, um);
        
        std::cout.setstate(std::ios_base::failbit);
        mine_block(mp, um, "MinerDave");
        std::cout.clear();
        
        // Validation 
        bool mempool_empty = mp.get_top_transactions(1).empty();
        double bob_bal = um.get_balance("Bob");
        double miner_bal = um.get_balance("MinerDave");

        cout << "Test 9 Data: MempoolEmpty=" << boolalpha << mempool_empty 
             << " | BobBal=" << bob_bal << " | MinerBal=" << miner_bal << endl;

        if (mempool_empty && is_equal(bob_bal, 40.0) && miner_bal > 0) {
            log_test(9, "Complete Mining Flow", true);
        } else {
            log_test(9, "Complete Mining Flow", false, "Balances incorrect or mempool not clear");
        }
            cout<<endl;
    }

    // Test 10: Unconfirmed Chain
    {
        UTXOManager um; reset_genesis(um);
        Mempool mp(10);
        
        Transaction tx1 = create_test_tx(um, "Alice", "Bob", 10.0, 0.0);
        mp.add_transaction(tx1, um); 

        Input unconfirmedInput(tx1.tx_id, 0, "Bob"); 
        
        vector<Input> inputs; inputs.push_back(unconfirmedInput);
        vector<Output> outputs; outputs.push_back(Output(10.0, "Eve"));
        Transaction tx2(inputs, outputs);
        
        pair<bool, string> res = mp.add_transaction(tx2, um);
        
        cout << "Test 10 Data: Spending Unconfirmed UTXO Result=" << res.first << endl;

        if (!res.first) log_test(10, "Unconfirmed Chain", true);
        else log_test(10, "Unconfirmed Chain", false, "System accepted unmined input (Design Violation)");
            cout<<endl;
    }

    cout << "============================================\n\n";
}

#endif