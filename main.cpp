#include <iostream>
#include <string>
#include "BankingSystem.h" 

using namespace std;

void showMainMenu();
void showTransactionMenu();
void fakeLoading();

void showMainMenu() {
    cout << "\n====================================\n";
    cout << "      WELCOME TO THE BANK SYSTEM     \n";
    cout << "====================================\n";
    cout << "[1] Create Account\n";
    cout << "[2] Login\n";
    cout << "------------------------------------\n";
}

void showTransactionMenu() {
    cout << "\n====================================\n";
    cout << "          TRANSACTION MENU          \n";
    cout << "====================================\n";
    cout << "[1] Check Balance\n";
    cout << "[2] Deposit\n";
    cout << "[3] Withdraw\n";
    cout << "[4] Transfer Fund\n";
    cout << "[5] Logout\n";
    cout << "------------------------------------\n";
}

void fakeLoading() {
    cout << "\nProcessing transaction... Please wait.\n";
}

int main() {
    try {
        BankingSystem bank;
        int choice;

        while (true) {
            showMainMenu();
            cout << "Choose an option: ";
            cin >> choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "[ERROR] Invalid option! Please enter numbers only (1 or 2).\n";
                continue;
            }

            bool runTransactions = false;

            if (choice == 1) {
                // Kung nakagawa ng account, magiging true ito at mag-o-auto login
                if (bank.createAccount()) {
                    runTransactions = true;
                }
            } 
            else if (choice == 2) {
                // Kung successful ang login, magiging true ito
                if (bank.login()) {
                    runTransactions = true;
                }
            } 
            else {
                cout << "[ERROR] Invalid choice. Please select 1 or 2 only.\n";
                continue;
            }

            // DRETSO TRANSACTION MENU (Kahit galing sa Create Account o sa Login)
            if (runTransactions) {
                int txChoice;
                bool inTransaction = true;
                
                while (inTransaction) {
                    showTransactionMenu();
                    cout << "Choose an option: ";
                    cin >> txChoice;

                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "[ERROR] Invalid option! Please enter numbers only (1-4).\n";
                        continue;
                    }

                    switch (txChoice) {
                        case 1:
                            bank.checkBalance();
                            break;
                        case 2:
                            bank.deposit();
                            break;
                        case 3:
                            bank.withdraw();
                            break;
                        case 4:
                            bank.transfer();
                            break;
                        case 5:
                            fakeLoading();
                            bank.logout();
                            inTransaction = false; 
                            break;
                        default:
                            cout << "[ERROR] Invalid option. Choose from 1 to 5 only.\n";
                    }
                }
            }
        }
    } catch (const exception& e) {
        cerr << "\n[CRITICAL RUNTIME ERROR]: " << e.what() << endl;
    }
    return 0;
}