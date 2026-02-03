# Bitcoin UTXO Simulator (C++)

**Team Name:** Nakamotos

**Course:** CS 216: Introduction to Blockchain

**Date:** February 3, 2025

## Team Members

| Name | Roll Number |
| --- | --- |
| Alaya DCruz | 240001007 |
| Anushka Krishan | 240001012 |
| Prakrati Pawar | 240001053 |
| Vanshika Gupta | 240001076 |

---

## Project Overview

This project is a C++ simulation of Bitcoin's transaction lifecycle. It implements the **UTXO (Unspent Transaction Output)** model to strictly enforce ownership, prevent double-spending, and simulate the mining process.

The system is built using standard C++ STL containers (`std::map`, `std::vector`, `std::set`) to manage the blockchain state and the memory pool (mempool) efficiently. It includes an interactive CLI to create transactions, mine blocks, and inspect the state, along with an automated test suite.

### Key Features

* **UTXO Set Management:** Uses a `std::map` to track unspent outputs, allowing  lookup and removal.
* **Mempool Logic:** Stores unconfirmed transactions and enforces "First-Seen" rules by tracking spent inputs in a `std::set` before mining.
* **Miner Simulation:** Selects transactions based on the highest fee (Greedy approach), validates them against the current UTXO set, and awards block rewards (Coinbase + Fees).
* **Automated Testing:** Includes a test file (`tests.h`) covering various scenarios like double spending, race attacks, unconfirmed chains, etc.

---

## File Structure

```text
CS216-TeamName-UTXO-Simulator/
│
├── main.cpp            # Entry point. Handles the CLI menu and system initialization.
├── utxo_manager.h      # Manages the Global UTXO set (The "State" of the ledger).
├── transaction.h       # Defines Input/Output structs and Transaction validation logic.
├── mempool.h           # Manages unconfirmed transactions and detects double-spends.
├── miner.h             # Implements block mining, fee collection, and coinbase rewards.
├── tests.h             # Contains the mandatory test scenarios.
└── README.md           # Project documentation.

```

---

## Setup & Usage

### Prerequisites

**Compiler:** G++ (supports C++11 or later)

### Compilation

To compile the simulator, run the following command in your terminal:

```bash
g++ main.cpp -o bitcoin_sim

```

### Execution

Run the executable:

```bash
./bitcoin_sim

```

### Interactive Menu

Upon running, you will see the following options:

1. **Create Transaction:** Manually transfer coins between users (Alice, Bob, Charlie, etc.).
2. **View UTXO Set:** Display the current state of all unspent coins.
3. **View Mempool:** Check pending transactions waiting to be mined.
4. **Mine Block:** Trigger the mining process to confirm transactions and earn rewards.
5. **Run Tests:** Execute the automated test scenarios defined in the test file.

---

## Implementation Details

### 1. The UTXO Manager (`utxo_manager.h`)

We simulate the blockchain state using a map:

* **Key:** `pair<string, int>` representing `{TransactionID, OutputIndex}`.
* **Value:** `UTXO` struct containing `{Amount, Owner}`.
This allows us to instantly verify if an input exists and if the spender owns it.

### 2. Transaction Validation (`transaction.h`)

Every transaction undergoes strict checks before entering the mempool:

* **Existence:** Do the referenced inputs exist in the UTXO set?
* **Ownership:** Does the sender match the owner of the UTXO?
* **Balance:** Is `Sum(Inputs) >= Sum(Outputs)`?
* **Double Spend:** Have these inputs already been referenced by another transaction in the mempool?

### 3. Mempool & Conflict Resolution (`mempool.h`)

To prevent double-spending *before* mining (0-conf), the mempool maintains a `spent_utxos` set.

* When a transaction is added, its inputs are marked as "spent" in the mempool scope.
* If a second transaction tries to use the same inputs, it is rejected immediately (Race Attack protection).

### 4. Mining (`miner.h`)

The miner simulates a simplified Proof-of-Work result:

1. It fetches the top  transactions from the mempool (sorted by highest fee).
2. It re-verifies inputs (in case of chain reorgs or conflicts).
3. It removes consumed inputs from the `UTXOManager` and adds new outputs.
4. It generates a **Coinbase Transaction** (Block Reward 12.5 + Fees) assigned to the miner.

---

## Test Coverage

The `tests.h` file implements logic to verify the following scenarios:

1. **Valid Transaction:** Successful transfer with correct change.
2. **Double Spend (Mempool):** Attempting to spend the same UTXO twice in rapid succession.
3. **Insufficient Funds:** Inputs less than outputs.
4. **Ownership Mismatch:** Alice trying to spend Bob's UTXO.
5. **Mining Flow:** Verifying UTXO updates after a block is mined.
6. **Unconfirmed Chaining:** Attempting to spend an output that is still in the mempool (not yet mined).

---

## Assumptions & Notes

* **Cryptography:** We use string comparisons for addresses/signatures as per the assignment constraints.
* **Networking:** All nodes (Miners, Users) share the same local `UTXOManager` instance to simulate a synchronized network state.
* **Genesis Block:** The system initializes with pre-funded accounts (Alice: 50, Bob: 30, etc.) for testing purposes.