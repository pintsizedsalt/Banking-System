#pragma once
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <iomanip>
#include <ctime>
#include "Account.h"

using namespace std;

class BankingSystem {
private:
    sqlite3* db;
    Account* loggedInAccount;

    // Kusang gagawa ng table sa unang bukas ng program
    void initializeDatabase() {
        char* errMsg = nullptr;
        string sql = "CREATE TABLE IF NOT EXISTS accounts ("
                     "account_number INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "pin TEXT NOT NULL,"
                     "balance REAL NOT NULL);";

        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            string error = errMsg;
            sqlite3_free(errMsg);
            throw runtime_error("Database Init Failed: " + error);
        }
        
        // Mag-insert ng panimulang dummy account para sa testing kung walang laman
        string checkSql = "SELECT COUNT(*) FROM accounts;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, nullptr);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) == 0) {
            sqlite3_exec(db, "INSERT INTO accounts (account_number, name, pin, balance) VALUES (1001, 'Test User', '1234', 5000.00);", nullptr, nullptr, nullptr);
        }
        sqlite3_finalize(stmt);
    }

    // Function para makuha ang kasalukuyang petsa at oras para sa resibo
    string getCurrentDateTime() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
        return string(buffer);
    }

public:
    BankingSystem() {
        loggedInAccount = nullptr;
        // Kumokonekta sa file-based SQL database (banking.db)
        if (sqlite3_open("banking.db", &db) != SQLITE_OK) {
            throw runtime_error("Failed to connect to Database server.");
        }
        initializeDatabase();
    }

    ~BankingSystem() {
        sqlite3_close(db);
        if (loggedInAccount) delete loggedInAccount;
    }

    // Requirement: Account Creation with proper input validation
    bool createAccount() {
        cout << "\n====================================\n";
        cout << "          CREATE ACCOUNT            \n";
        cout << "====================================\n";
        
        string name, pin;

        cin.ignore(10000, '\n');

        // 1. NAME LOCK
        while (true) {
            cout << "Enter Account Name: ";
            getline(cin, name);
            
            if (name.empty()) {
                cout << "[ERROR] Name cannot be empty! Please try again.\n\n";
                continue;
            }
            
            // Name Validation Update: prevent users from creating accounts with names consisting only of spaces
            bool allSpaces = true;
            for (char c : name) {
                if (!isspace(c)) {
                    allSpaces = false;
                    break;
                }
            }
            if (allSpaces) {
                cout << "[ERROR] Name cannot consist only of spaces! Please try again.\n\n";
                continue;
            }
            if (name.length() > 50) {
                cout << "[ERROR] Name is too long! Please keep it under 50 characters.\n\n";
                continue;
            }


            bool hasInvalidChar = false;
            for (char c : name) {
                if (!isalpha(c) && !isspace(c)) {
                    hasInvalidChar = true;
                    break;
                }
            }

            if (hasInvalidChar) {
                cout << "[ERROR] Name must contain letters and spaces only! Please try again.\n\n";
                continue;
            }

            // Proper capitalization: First letter of each word capitalized, rest lowercase
            bool capitalizeNext = true;
            for (char &c : name) {
                if (isspace(c)) {
                    capitalizeNext = true;
                } else if (isalpha(c)) {
                    if (capitalizeNext) {
                        c = toupper(c);
                    } else {
                        c = tolower(c);
                    }
                    capitalizeNext = false;
                }
            }
            break; 
        }
        
        // 2. PIN LOCK
        while (true) {
            cout << "Enter 4-Digit PIN: ";
            cin >> pin;

            if (pin.length() != 4 || pin.find_first_not_of("0123456789") != string::npos) {
                cout << "[ERROR] PIN must be exactly 4 digits and numeric only! Please try again.\n\n";
                continue;
            }
            break; 
        }

        // 3. DATABASE INSERTION (Automatic 0.00 Balance)
        string sql = "INSERT INTO accounts (name, pin, balance) VALUES (?, ?, 0.00);";
        sqlite3_stmt* stmt;
        bool creationSuccess = false;
        int generatedId = 0;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, pin.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                generatedId = sqlite3_last_insert_rowid(db);
                cout << "\n[SUCCESS] Account Saved!\n";
                cout << "Your Account Number is: " << generatedId << "\n";
                creationSuccess = true;
            } else {
                cout << "[ERROR] Failed to save account to database.\n";
            }
            sqlite3_finalize(stmt);
        }

        // AUTOMATIC LOGIN (Para direkta sa Transaction Menu gaya ng flowchart)
        if (creationSuccess) {
            loggedInAccount = new Account();
            loggedInAccount->accountNumber = generatedId;
            loggedInAccount->accountName = name;
            loggedInAccount->pin = pin;
            loggedInAccount->balance = 0.00;
            return true; // Sasabihin sa main.cpp na pumasok agad sa loop
        }

        return false;
    }

    // Requirement: Secure Login / Authentication
    // Requirement: Secure Login / Authentication with loop validation
    bool login() {
        while (true) {
            cout << "\n====================================\n";
            cout << "               LOGIN                \n";
            cout << "====================================\n";
            cout << "Enter Account Number (or '0' to go back): ";
            
            int accNum;
            cin >> accNum;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "[ERROR] Invalid format! Account number must be numeric.\n";
                continue;
            }

            // Exit to Main Menu
            if (accNum == 0) return false;

            // STEP 1: Check if the account exists first
            string sql = "SELECT account_number, name, pin, balance FROM accounts WHERE account_number = ?;";
            sqlite3_stmt* stmt;
            bool accountFound = false;
            
            // Temporary variables to hold DB data
            int dbAccNum;
            string dbName, dbPin;
            double dbBalance;

            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, accNum);

                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    accountFound = true;
                    dbAccNum = sqlite3_column_int(stmt, 0);
                    dbName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    dbPin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    dbBalance = sqlite3_column_double(stmt, 3);
                }
                sqlite3_finalize(stmt);
            }

            if (!accountFound) {
                cout << "[ERROR] Account Number " << accNum << " does not exist!\n";
                continue; // Stay in the Account Number entry loop
            }

            // STEP 2: Account exists, now loop for PIN
            while (true) {
                cout << "Enter PIN for " << dbName << " (or '0' to go back): ";
                string inputPin;
                cin >> inputPin;

                // Option to go back to Account Number entry
                if (inputPin == "0") break; 

                if (inputPin.length() != 4 || inputPin.find_first_not_of("0123456789") != string::npos) {
                    cout << "[ERROR] PIN must be exactly 4 digits and numeric only!\n";
                    continue;
                }

                if (inputPin == dbPin) {
                    loggedInAccount = new Account();
                    loggedInAccount->accountNumber = dbAccNum;
                    loggedInAccount->accountName = dbName;
                    loggedInAccount->pin = dbPin;
                    loggedInAccount->balance = dbBalance;

                    cout << "\n[SUCCESS] Welcome, " << loggedInAccount->accountName << "!\n";
                    return true; // Exit login and go to transactions
                } else {
                    cout << "[ERROR] Incorrect PIN. Please try again.\n";
                    // Stays in the PIN loop
                }
            }
        }
    }

    void logout() {
        if (loggedInAccount) {
            delete loggedInAccount;
            loggedInAccount = nullptr;
        }
        
        cout << "\n====================================\n";
        cout << "        LOGOUT SUCCESSFULLY!        \n";
        cout << "====================================\n";
        cout << " Thank you for banking with us!     \n";
        cout << " Secure session closed.             \n";
        cout << "====================================\n";
        
        // Estetik na pause para hindi biglang tatalon pabalik sa welcome panel
        cout << "\n[Press Enter to return to Home Page]";
        cin.ignore(10000, '\n');
        cin.get();
    }

    // Requirement: Check Balance
    // 1. AESTHETIC CHECK BALANCE
    void checkBalance() {
        cout << "\n====================================\n";
        cout << "          BALANCE INQUIRY           \n";
        cout << "====================================\n";
        cout << " Account Name   : " << loggedInAccount->accountName << "\n";
        cout << " Account Number : " << loggedInAccount->accountNumber << "\n";
        cout << " Current Balance: P" << fixed << setprecision(2) << loggedInAccount->balance << "\n";
        cout << "====================================\n";
        
        // Estetik at malinis na buffer pause para makabalik sa menu
        cout << "\n[Press Enter to return to Transaction Menu]";
        cin.ignore(10000, '\n');
        cin.get(); 
    }

    // 2. DEPOSIT
    // 2. DEPOSIT WITH AESTHETIC CONFIRMATION PROMPT
    void deposit() {
        cout << "\n====================================\n";
        cout << "              DEPOSIT               \n";
        cout << "====================================\n";
        
        double amount;
        while (true) {
            cout << "Enter Amount to Deposit: P";
            cin >> amount;

            // Clear the input stream buffer after reading amount
            cin.ignore(10000, '\n');

            // Kapag nakasalo ng letters o special characters
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "[ERROR] Invalid input! Please enter numbers only.\n\n";
                continue; 
            }

            // Kapag numero pero negative o saktong 0
            if (amount <= 0) {
                cout << "[ERROR] Deposit amount must be greater than 0! Please try again.\n\n";
                continue;
            }
            break;
        }

        char confirm;
        cout << "\nAre you sure you want to deposit P" << fixed << setprecision(2) << amount << "? [Y/N]: ";
        cin >> confirm;

        if (confirm == 'Y' || confirm == 'y') {
            string sql = "UPDATE accounts SET balance = balance + ? WHERE account_number = ?;";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_double(stmt, 1, amount);
                sqlite3_bind_int(stmt, 2, loggedInAccount->accountNumber);
                
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    loggedInAccount->balance += amount;
                    cout << "\n====================================\n";
                    cout << "        DEPOSIT SUCCESSFUL!         \n";
                    cout << "====================================\n";
                    cout << " Deposited Amount : P" << fixed << setprecision(2) << amount << "\n";
                    cout << " New Updated Bal  : P" << fixed << setprecision(2) << loggedInAccount->balance << "\n";
                    cout << "====================================\n";
                }
                sqlite3_finalize(stmt);
            }
        } else {
            cout << "\n[INFO] Deposit transaction cancelled.\n";
        }
    }

    // 3. WITHDRAW WITH INPUT LOCK & THERMAL RECEIPT
    void withdraw() {
        cout << "\n====================================\n";
        cout << "             WITHDRAW               \n";
        cout << "====================================\n";
        
        double amount;
        while (true) {
            cout << "Current Available Balance: P" << fixed << setprecision(2) << loggedInAccount->balance << "\n";
            cout << "Enter Amount to Withdraw (or type 0 to cancel): P";
            cin >> amount;

            // Clear the input stream buffer after reading amount
            cin.ignore(10000, '\n');

            // Kapag letters o special characters
            if (cin.fail()) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "[ERROR] Invalid input! Please enter numbers only.\n\n";
                continue;
            }

            if (amount == 0) {
                cout << "[INFO] Transaction cancelled.\n";
                return;
            }

            // Kapag negative ang tinype
            if (amount < 0) {
                cout << "[ERROR] Withdrawal amount must be greater than 0!\n\n";
                continue;
            }

            // Kapag kulang ang pondo
            if (amount > loggedInAccount->balance) {
                cout << "[ERROR] Insufficient balance! You only have P" << loggedInAccount->balance << ".\n";
                cout << "Please request an amount within your balance limits.\n\n";
                continue; 
            }
            break; 
        }

        char confirm;
        cout << "\nAre you sure you want to withdraw P" << fixed << setprecision(2) << amount << "? [Y/N]: ";
        cin >> confirm;

        if (confirm == 'Y' || confirm == 'y') {
            string sql = "UPDATE accounts SET balance = balance - ? WHERE account_number = ?;";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_double(stmt, 1, amount);
                sqlite3_bind_int(stmt, 2, loggedInAccount->accountNumber);
                
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    loggedInAccount->balance -= amount;
                    
                    cout << "\n------------------------------------";
                    cout << "\n       OFFICIAL WITHDRAWAL RECEIPT   ";
                    cout << "\n------------------------------------";
                    cout << "\n Date/Time : " << getCurrentDateTime(); // Use getCurrentDateTime()
                    cout << " Account   : " << loggedInAccount->accountName;
                    cout << "\n Account # : " << loggedInAccount->accountNumber;
                    cout << "\n Withdrawn : P" << fixed << setprecision(2) << amount;
                    cout << "\n Rem. Bal  : P" << fixed << setprecision(2) << loggedInAccount->balance;
                    cout << "\n------------------------------------";
                    cout << "\n [STATUS] Withdrawal Successful!\n";
                }
                sqlite3_finalize(stmt);
            }
        } else {
            cout << "\n[INFO] Withdrawal transaction cancelled.\n";
        }
    }

    // 4. TRANSFER FUND WITH SELF-BLOCK & INPUT LOCK
    // 4. FUND TRANSFER - STRICT FLOWCHART COMPLIANCE
    // 4. FUND TRANSFER - STRICT VALIDATION LOOK
    void transfer() {
        cout << "\n====================================\n";
        cout << "          FUND TRANSFER             \n";
        cout << "====================================\n";
        
        int receiverId;
        double amount;
        
        // LOOP 1: Para sa Account Exists? Validation
        while (true) {
            cout << "Enter Receiver Account Number (or '0' to cancel): "; // Added cancel option
            cin >> receiverId;
            
            // Clear the input stream buffer after reading receiverId
            cin.ignore(10000, '\n');

            // Mas mahigpit na harang: Kapag letters, special characters, OR negative/zero ang account number
            if (cin.fail() || receiverId <= 0) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "[ERROR] Invalid input! Please enter numbers only.\n\n";
                continue; // FLOWCHART COMPLIANCE: Uulit agad, bawal lumipat sa amount!
            }

            // Fix the Fund Transfer Soft-lock Loop: Allow user to cancel transaction
            if (receiverId <= 0) {
                cout << "[INFO] Fund transfer cancelled.\n";
                return; // Exit the function
            }
            
            // Bawal mag-transfer sa sarili 
            if (receiverId == loggedInAccount->accountNumber) {
                cout << "[ERROR] Cannot transfer funds to your own account!\n\n";
                continue;
            }

            // LOOP 2: Para sa Is Balance Sufficient? Validation
            while (true) { // This loop is for amount validation
                cout << "Enter Amount to Transfer (or '0' to cancel): P"; // Added cancel option
                cin >> amount;
                
                // Clear the input stream buffer after reading amount
                cin.ignore(10000, '\n');

                // Kapag letters o special characters ang hinarang sa Amount
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "[ERROR] Invalid input! Please enter numbers only.\n\n";
                    continue; // Iikot lang sa loop ng Amount
                }
                
                // Kapag totoong numero pero negative o zero
                if (amount <= 0) {
                    cout << "[ERROR] Transfer amount must be greater than 0!\n\n";
                    continue; // Iikot lang sa loop ng Amount
                }

                // Fix the Fund Transfer Soft-lock Loop: Allow user to cancel transaction
                if (amount <= 0) {
                    cout << "[INFO] Fund transfer cancelled.\n";
                    return; // Exit the function
                }
                
                // Flowchart Diamond: Is Balance Sufficient?
                if (amount > loggedInAccount->balance) {
                    cout << "[ERROR] Insufficient balance to complete transfer!\n";
                    cout << "Please try entering a different amount.\n\n";
                    continue; // FLOWCHART COMPLIANCE: Iikot lang sa paghingi ng Amount
                }
                break; // Nakalagpas sa balance validation
            }

            // Flowchart Diamond: Account Exists? (Database Check)
            string sqlCheck = "SELECT balance FROM accounts WHERE account_number = ?;";
            sqlite3_stmt* stmtCheck;
            bool accountFound = false;
            
            if (sqlite3_prepare_v2(db, sqlCheck.c_str(), -1, &stmtCheck, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmtCheck, 1, receiverId);
                if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
                    accountFound = true;
                }
                sqlite3_finalize(stmtCheck);
            }
            
            if (!accountFound) {
                cout << "[ERROR] Account Number does not exist!\n";
                cout << "Restarting transfer validation...\n\n";
                continue; // FLOWCHART COMPLIANCE: Balik sa pinakataas (Receiver Account Number at Amount)
            }
            
            break; // Parehong VALID na ang Account at sapat ang Balance!
        }

        // CONFIRMATION PROMPT bago i-save sa DB
        char confirm;
        cout << "\nAre you sure you want to transfer P" << fixed << setprecision(2) << amount << "? [Y/N]: ";
        cin >> confirm;

        if (confirm == 'Y' || confirm == 'y') {
            sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

            string sqlDed = "UPDATE accounts SET balance = balance - ? WHERE account_number = ?;"; // Deduct from sender
            string sqlCred = "UPDATE accounts SET balance = balance + ? WHERE account_number = ?;";
            sqlite3_stmt *stmtDed = nullptr, *stmtCred = nullptr; // Initialize to nullptr
            
            if (sqlite3_prepare_v2(db, sqlDed.c_str(), -1, &stmtDed, nullptr) == SQLITE_OK &&
                sqlite3_prepare_v2(db, sqlCred.c_str(), -1, &stmtCred, nullptr) == SQLITE_OK) {
                
                sqlite3_bind_double(stmtDed, 1, amount);
                sqlite3_bind_int(stmtDed, 2, loggedInAccount->accountNumber);
                
                sqlite3_bind_double(stmtCred, 1, amount);
                sqlite3_bind_int(stmtCred, 2, receiverId);
                
                if (sqlite3_step(stmtDed) == SQLITE_DONE && sqlite3_step(stmtCred) == SQLITE_DONE) {
                    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
                    loggedInAccount->balance -= amount;

                    // Generate a pseudo-reference number (e.g., TRF-12345678-1002)
                    string refNo = "TRF-" + to_string(time(0)).substr(2) + "-" + to_string(receiverId);

                    cout << "\n------------------------------------";
                    cout << "\n       FUND TRANSFER RECEIPT         ";
                    cout << "\n------------------------------------";
                    cout << "\n Ref No.    : " << refNo;
                    cout << "\n Date/Time  : " << getCurrentDateTime();
                    cout << "\n From       : " << loggedInAccount->accountName;
                    cout << "\n To Acc #   : " << receiverId;
                    cout << "\n Amount     : PHP " << fixed << setprecision(2) << amount;
                    cout << "\n Rem. Bal   : PHP " << fixed << setprecision(2) << loggedInAccount->balance;
                    cout << "\n------------------------------------";
                    cout << "\n [STATUS] Transfer Successful!\n";
                } else {
                    // If step fails, rollback
                    sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
                    cout << "\n[ERROR] Database transaction failed. Transfer cancelled.\n";
                }
                sqlite3_finalize(stmtDed);
                sqlite3_finalize(stmtCred);
            } else {
                // Secure the DB Transaction in transfer():
                // Ensure that if sqlite3_prepare_v2 fails to compile the SQL statements after BEGIN TRANSACTION;,
                // a ROLLBACK; is executed so that the database is not left in an locked/uncommitted transaction state.
                sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
                cout << "\n[ERROR] Failed to prepare SQL statements for transfer. Transfer cancelled.\n";
                // Finalize any statements that might have been prepared before the error
                if (stmtDed) {
                    sqlite3_finalize(stmtDed);
                }
                if (stmtCred) {
                    sqlite3_finalize(stmtCred);
                }
            }
        } else {
            cout << "\n[INFO] Transfer transaction cancelled.\n";
        }
    }
};