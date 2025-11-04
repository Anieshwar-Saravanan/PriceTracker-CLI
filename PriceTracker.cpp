#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <iomanip>
using namespace std;

// ------------------------------
// Structures & Classes
// ------------------------------

struct Expense {
    string description;
    float amount;
    string paidBy;
    vector<string> splitAmong;
    vector<float> amountOwed;
};

struct Settlement {
    string fromUser;
    string toUser;
    float amount;
};

struct UserBalance {
    string userName;
    float balance = 0.0f;
};

struct GraphEdge {
    string toUser;
    float amount;
};

struct GraphNode {
    string userName;
    vector<GraphEdge> edges;
};

// ------------------------------
// Global Data Structures
// ------------------------------
vector<Expense> expenses;                          // List of expenses
queue<Settlement> settlements;                     // Queue for settlements
map<string, float> userBalances;                   // User balances (acts like BST)
map<string, vector<GraphEdge>> debtGraph;          // Graph for user debts

// ------------------------------
// Function Prototypes
// ------------------------------
void addExpense();
void printExpenses();
void enqueueSettlement();
void processSettlement();
void printBalances();
void printSettlements();
void addDebtToGraph(const string &fromUser, const string &toUser, float amount);
void printGraph();
void updateExpenseAfterSettlement(const string &fromUser, const string &toUser, float amount);

// ------------------------------
// Helper: Read line input
// ------------------------------
string readString(const string &prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

// ------------------------------
// Add Expense
// ------------------------------
void addExpense() {
    Expense newExpense;
    cout << "\nEnter expense description: ";
    getline(cin, newExpense.description);

    cout << "Enter total amount: ";
    cin >> newExpense.amount;
    cin.ignore();

    cout << "Enter who paid (user): ";
    getline(cin, newExpense.paidBy);

    int userCount;
    cout << "How many users are splitting the expense? ";
    cin >> userCount;
    cin.ignore();

    newExpense.splitAmong.push_back(newExpense.paidBy);
    newExpense.amountOwed.push_back(0.0f); // Payer owes nothing

    for (int i = 0; i < userCount; i++) {
        string name;
        cout << "Enter user " << i + 1 << " name: ";
        getline(cin, name);
        newExpense.splitAmong.push_back(name);
    }

    float amountPerUser = newExpense.amount / (userCount + 1);
    for (int i = 0; i < userCount; i++) {
        newExpense.amountOwed.push_back(amountPerUser);
    }

    expenses.push_back(newExpense);

    // Update balances
    userBalances[newExpense.paidBy] += newExpense.amount;
    for (int i = 0; i < userCount; i++) {
        const string &debtor = newExpense.splitAmong[i + 1];
        userBalances[debtor] -= amountPerUser;
        addDebtToGraph(newExpense.paidBy, debtor, amountPerUser);
    }

    cout << "Expense added successfully.\n";
}

// ------------------------------
// Print Expenses
// ------------------------------
void printExpenses() {
    if (expenses.empty()) {
        cout << "No expenses recorded.\n";
        return;
    }

    for (const auto &exp : expenses) {
        cout << "\n--- Expense: " << exp.description << " ---\n";
        cout << "Amount: " << fixed << setprecision(2) << exp.amount
             << " paid by " << exp.paidBy << "\n";
        cout << "Split among:\n";
        for (size_t i = 0; i < exp.splitAmong.size(); ++i) {
            cout << "\t" << exp.splitAmong[i]
                 << " owes " << exp.amountOwed[i] << "\n";
        }
    }
}

// ------------------------------
// Add Settlement (enqueue)
// ------------------------------
void enqueueSettlement() {
    Settlement s;
    cout << "\nEnter the user who will pay: ";
    getline(cin, s.fromUser);
    cout << "Enter the user to receive the payment: ";
    getline(cin, s.toUser);
    cout << "Enter the amount to settle: ";
    cin >> s.amount;
    cin.ignore();

    settlements.push(s);
    cout << "Settlement added to queue.\n";
}

// ------------------------------
// Process Settlement (dequeue)
// ------------------------------
void processSettlement() {
    if (settlements.empty()) {
        cout << "No settlements to process.\n";
        return;
    }

    Settlement s = settlements.front();
    settlements.pop();

    cout << "Settling: " << s.fromUser << " pays " << s.toUser
         << " " << fixed << setprecision(2) << s.amount << "\n";

    // Update balances
    userBalances[s.fromUser] += s.amount;
    userBalances[s.toUser] -= s.amount;

    updateExpenseAfterSettlement(s.fromUser, s.toUser, s.amount);
}

// ------------------------------
// Print All Settlements
// ------------------------------
void printSettlements() {
    if (settlements.empty()) {
        cout << "No settlements in the queue.\n";
        return;
    }

    queue<Settlement> temp = settlements;
    while (!temp.empty()) {
        Settlement s = temp.front();
        cout << s.fromUser << " pays " << s.toUser
             << " " << fixed << setprecision(2) << s.amount << "\n";
        temp.pop();
    }
}

// ------------------------------
// Update Expenses After Settlement
// ------------------------------
void updateExpenseAfterSettlement(const string &fromUser, const string &toUser, float amount) {
    for (auto &exp : expenses) {
        for (size_t i = 0; i < exp.splitAmong.size(); ++i) {
            if (exp.splitAmong[i] == fromUser)
                exp.amountOwed[i] -= amount;
            if (exp.splitAmong[i] == toUser)
                exp.amountOwed[i] += amount;
        }
    }
}

// ------------------------------
// Add Debt To Graph
// ------------------------------
void addDebtToGraph(const string &fromUser, const string &toUser, float amount) {
    debtGraph[fromUser].push_back({toUser, amount});
}

// ------------------------------
// Print Graph
// ------------------------------
void printGraph() {
    if (debtGraph.empty()) {
        cout << "No debts recorded.\n";
        return;
    }

    for (const auto &node : debtGraph) {
        cout << node.first << " owes:\n";
        for (const auto &edge : node.second) {
            cout << "  - " << edge.toUser << ": " << fixed << setprecision(2)
                 << edge.amount << "\n";
        }
    }
}

// ------------------------------
// Print Balances
// ------------------------------
void printBalances() {
    if (userBalances.empty()) {
        cout << "No balances recorded.\n";
        return;
    }

    cout << "\nUser Balances:\n";
    for (const auto &pair : userBalances) {
        cout << pair.first << ": " << fixed << setprecision(2)
             << pair.second << "\n";
    }
}

// ------------------------------
// Main Menu
// ------------------------------
int main() {
    int choice;

    while (true) {
        cout << "\n--- Expense Splitter ---\n"
             << "1. Add Expense\n"
             << "2. Print Expenses\n"
             << "3. Add Settlement\n"
             << "4. View User Balances\n"
             << "5. Process Settlement\n"
             << "6. View Debt Graph\n"
             << "7. Exit\n"
             << "Enter your choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: addExpense(); break;
            case 2: printExpenses(); break;
            case 3: enqueueSettlement(); break;
            case 4: printBalances(); break;
            case 5: processSettlement(); break;
            case 6: printGraph(); break;
            case 7: cout << "Exiting...\n"; return 0;
            default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}
