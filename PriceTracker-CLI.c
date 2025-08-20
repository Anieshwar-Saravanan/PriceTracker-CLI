#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define structures

// Linked list node for storing expenses
struct Expense {
    char description[100];
    float amount;
    char paidBy[50];
    char splitAmong[50][50]; // Users who share the expense
    float amountOwed[50];     // Amount owed by each user
    int userCount;            // Number of users sharing the expense
    struct Expense *next;
};

// Queue node for settlement transactions
struct Settlement {
    char fromUser[50];
    char toUser[50];
    float amount;
    struct Settlement *next;
};

// Binary search tree node for user balances
struct UserBalance {
    char userName[50];
    float balance; // Positive if others owe, negative if they owe
    struct UserBalance *left, *right;
};

// Graph structure to represent users and debts
struct GraphNode {
    char userName[50];
    struct GraphEdge *edges; // List of users this user owes money to
    struct GraphNode *next;  // For the adjacency list
};

struct GraphEdge {
    char toUser[50];
    float amount;
    struct GraphEdge *next;
};

// Global variables
struct Expense *expenseHead = NULL; // Linked list head for expenses
struct Settlement *settlementFront = NULL, *settlementRear = NULL; // Queue for settlements
struct UserBalance *balanceRoot = NULL; // Root of BST for user balances
struct GraphNode *graphHead = NULL; // Head of the graph for user debts

// Function declarations
void addExpense(char *description, float amount, char *paidBy, char splitAmong[50][50], int userCount);
void printExpenses();
struct UserBalance* addUserBalance(struct UserBalance *root, char *userName, float balanceUpdate);
struct UserBalance* searchUser(struct UserBalance *root, char *userName);
void printBalances(struct UserBalance *root);
void enqueueSettlement(char *fromUser, char *toUser, float amount);
void dequeueSettlement();
void printSettlements();

// Graph functions
void addDebtToGraph(char *fromUser, char *toUser, float amount);
void printGraph();
void updateExpenseAfterSettlement(char *fromUser, char *toUser, float amount);

// Function to read a string input (removes newline character)
void readString(char *str, int size) {
    fgets(str, size, stdin);
    str[strcspn(str, "\n")] = '\0'; // Remove the trailing newline
}

// Main function
int main() {
    int choice;

    while (1) {
        printf("\n--- Expense Splitter ---\n");
        printf("1. Add Expense\n");
        printf("2. Print Expenses\n");
        printf("3. Add Settlement\n");
        printf("4. View User Balances\n");
        printf("5. Process Settlement\n");
        printf("6. View Debt Graph\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline character after scanf

        if (choice == 1) {
            // Add Expense
            char description[100];
            float amount;
            char paidBy[50];
            int userCount;

            printf("\nEnter expense description: ");
            readString(description, 100);

            printf("Enter total amount: ");
            scanf("%f", &amount);
            getchar(); // Consume newline character

            printf("Enter who paid (user): ");
            readString(paidBy, 50);

            printf("How many users are splitting the expense? ");
            scanf("%d", &userCount);
            getchar(); // Consume newline character

            char splitAmong[50][50];

            // Input users who are splitting the expense
            for (int i = 0; i < userCount; i++) {
                printf("Enter user %d name: ", i + 1);
                readString(splitAmong[i], 50);
            }

            addExpense(description, amount, paidBy, splitAmong, userCount);
        }
        else if (choice == 2) {
            // Print Expenses
            printExpenses();
        }
        else if (choice == 3) {
            // Add Settlement
            char fromUser[50], toUser[50];
            float amount;

            printf("\nEnter the user who will pay: ");
            readString(fromUser, 50);

            printf("Enter the user to receive the payment: ");
            readString(toUser, 50);

            printf("Enter the amount to settle: ");
            scanf("%f", &amount);
            getchar(); // Consume newline character

            enqueueSettlement(fromUser, toUser, amount);
        }
        /*else if (choice == 4) {
            // Print Settlements
            printSettlements();*/

        else if (choice == 4) {
            // View User Balances
            printf("\nUser Balances:\n");
            printBalances(balanceRoot);
        }
        else if (choice == 5) {
            // Process Settlement
            printf("\nProcessing settlements...\n");
            dequeueSettlement();
        }
        else if (choice == 6) {
            // View Debt Graph
            printf("\nDebt Graph:\n");
            printGraph();
        }
        else if (choice == 7) {
            // Exit
            break;
        }
        else {
            printf("Invalid choice, please try again.\n");
        }
    }

    return 0;
}

// Function to add a new expense to the linked list
void addExpense(char *description, float amount, char *paidBy, char splitAmong[50][50], int userCount) {
    struct Expense *newExpense = (struct Expense *)malloc(sizeof(struct Expense));

    // Set the description, total amount, and who paid
    strcpy(newExpense->description, description);
    newExpense->amount = amount;
    strcpy(newExpense->paidBy, paidBy);

    // Calculate the amount each user owes
    float amountPerUser = amount / (userCount + 1);  // Include the payer in the split

    // Set the payer's owed amount to 0 (since they paid)
    newExpense->amountOwed[0] = 0.0;
    strcpy(newExpense->splitAmong[0], paidBy);  // The first user is the one who paid

    // Set the amount owed by each user (including the payer)
    for (int i = 0; i < userCount; i++) {
        strcpy(newExpense->splitAmong[i + 1], splitAmong[i]);
        newExpense->amountOwed[i + 1] = amountPerUser; // Each of the others owes the calculated amount
    }

    // Set the number of users sharing the expense (including the payer)
    newExpense->userCount = userCount + 1;

    // Set the 'next' pointer to the current head of the list
    newExpense->next = expenseHead;

    // Update the head of the linked list to the new expense
    expenseHead = newExpense;

    // Update balances for the person who paid
    balanceRoot = addUserBalance(balanceRoot, paidBy, amount);  // Payer gains the amount they paid

    // Update balances for users splitting the expense
    for (int i = 0; i < userCount; i++) {
        balanceRoot = addUserBalance(balanceRoot, splitAmong[i], -amountPerUser);  // Each person owes
        addDebtToGraph(paidBy, splitAmong[i], amountPerUser);  // Add debt to the graph
    }
}


// Function to update expenses after a settlement
void updateExpenseAfterSettlement(char *fromUser, char *toUser, float amount) {
    struct Expense *temp = expenseHead;
    while (temp != NULL) {
        for (int i = 0; i < temp->userCount; i++) {
            if (strcmp(temp->splitAmong[i], fromUser) == 0) {
                // Decrease the owed amount for the person paying
                temp->amountOwed[i] -= amount;
            }
            if (strcmp(temp->splitAmong[i], toUser) == 0) {
                // Increase the owed amount for the person receiving payment
                temp->amountOwed[i] += amount;
            }
        }
        temp = temp->next;
    }
}

// Function to add debt information in the graph (adjacency list)
void addDebtToGraph(char *fromUser, char *toUser, float amount) {
    struct GraphNode *temp = graphHead;
    while (temp != NULL) {
        if (strcmp(temp->userName, fromUser) == 0) {
            // Add an edge from fromUser to toUser
            struct GraphEdge *newEdge = (struct GraphEdge *)malloc(sizeof(struct GraphEdge));
            strcpy(newEdge->toUser, toUser);
            newEdge->amount = amount;
            newEdge->next = temp->edges;
            temp->edges = newEdge;
            return;
        }
        temp = temp->next;
    }

    // If fromUser doesn't exist, create a new node for fromUser
    struct GraphNode *newNode = (struct GraphNode *)malloc(sizeof(struct GraphNode));
    strcpy(newNode->userName, fromUser);
    newNode->edges = NULL;
    newNode->next = graphHead;
    graphHead = newNode;

    // Add an edge from fromUser to toUser
    struct GraphEdge *newEdge = (struct GraphEdge *)malloc(sizeof(struct GraphEdge));
    strcpy(newEdge->toUser, toUser);
    newEdge->amount = amount;
    newEdge->next = newNode->edges;
    newNode->edges = newEdge;
}

// Function to print the graph (debt relationships)
void printGraph() {
    struct GraphNode *temp = graphHead;
    while (temp != NULL) {
        printf("%s owes:\n", temp->userName);
        struct GraphEdge *edge = temp->edges;
        while (edge != NULL) {
            printf("  - %s: %.2f\n", edge->toUser, edge->amount);
            edge = edge->next;
        }
        temp = temp->next;
    }
}

void printExpenses() {
    struct Expense *temp = expenseHead;

    if (temp == NULL) {
        printf("No expenses recorded.\n");
        return;
    }

    while (temp != NULL) {
        printf("\n--- Expense: %s ---\n", temp->description);
        printf("Amount: %.2f paid by %s\n", temp->amount, temp->paidBy);
        printf("Split among the following users:\n");

        for (int i = 0; i < temp->userCount; i++) {
            printf("\t%s owes %.2f\n", temp->splitAmong[i], temp->amountOwed[i]);
        }

        temp = temp->next;  // Move to the next expense in the list
    }
}


// Function to add/update user balance in the BST
struct UserBalance* addUserBalance(struct UserBalance *root, char *userName, float balanceUpdate) {
    if (root == NULL) {
        struct UserBalance *newNode = (struct UserBalance *)malloc(sizeof(struct UserBalance));
        strcpy(newNode->userName, userName);
        newNode->balance = balanceUpdate;
        newNode->left = newNode->right = NULL;
        return newNode;
    }
    if (strcmp(userName, root->userName) < 0)
        root->left = addUserBalance(root->left, userName, balanceUpdate);
    else if (strcmp(userName, root->userName) > 0)
        root->right = addUserBalance(root->right, userName, balanceUpdate);
    else
        root->balance += balanceUpdate;  // Update balance if user exists
    return root;
}

// Function to search for a user in the BST
struct UserBalance* searchUser(struct UserBalance *root, char *userName) {
    if (root == NULL || strcmp(root->userName, userName) == 0)
        return root;
    if (strcmp(userName, root->userName) < 0)
        return searchUser(root->left, userName);
    return searchUser(root->right, userName);
}

// Function to print all user balances (in-order traversal of BST)
void printBalances(struct UserBalance *root) {
    if (root != NULL) {
        printBalances(root->left);
        printf("%s: %.2f\n", root->userName, root->balance);
        printBalances(root->right);
    }
}

// Function to enqueue a settlement
void enqueueSettlement(char *fromUser, char *toUser, float amount) {
    struct Settlement *newSettlement = (struct Settlement *)malloc(sizeof(struct Settlement));
    strcpy(newSettlement->fromUser, fromUser);
    strcpy(newSettlement->toUser, toUser);
    newSettlement->amount = amount;
    newSettlement->next = NULL;

    if (settlementRear == NULL) {
        settlementFront = settlementRear = newSettlement;
    } else {
        settlementRear->next = newSettlement;
        settlementRear = newSettlement;
    }
}

// Function to dequeue and process a settlement
void dequeueSettlement() {
    if (settlementFront == NULL) {
        printf("No settlements to process.\n");
        return;
    }
    struct Settlement *settlement = settlementFront;
    settlementFront = settlementFront->next;
    if (settlementFront == NULL) {
        settlementRear = NULL;
    }

    printf("Settling: %s pays %s %.2f\n", settlement->fromUser, settlement->toUser, settlement->amount);

    // Update balances after settlement
    balanceRoot = addUserBalance(balanceRoot, settlement->fromUser, settlement->amount);
    balanceRoot = addUserBalance(balanceRoot, settlement->toUser, -settlement->amount);

    // Update expenses
    updateExpenseAfterSettlement(settlement->fromUser, settlement->toUser, settlement->amount);

    free(settlement);
}

// Function to print all settlements in the queue
void printSettlements() {
    struct Settlement *temp = settlementFront;
    while (temp != NULL) {
        printf("%s pays %s %.2f\n", temp->fromUser, temp->toUser, temp->amount);
        temp = temp->next;
    }
}