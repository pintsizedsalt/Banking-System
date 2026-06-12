======================================================================
                 BANKING MANAGEMENT SYSTEM (C++)
======================================================================
Institution/Affiliation : ICCT College of Computer Studies
Developer               : Sam
Database Engine         : SQLite3 (Embedded / Local File-Based)
======================================================================

----------------------------------------------------------------------
[1] PROJECT OVERVIEW
----------------------------------------------------------------------
This Banking Management System is built using C++17 and SQLite3 to 
simulate secure, robust, and real-time banking operations. 

> 🗄️ DATABASE ARCHITECTURE NOTE:
> This application uses an embedded SQLite3 database engine. You DO NOT 
> need to install MySQL Server, MySQL Workbench, or any external 
> database servers to run this project. The program automatically 
> handles creation and initialization, spawning a local database 
> file named 'banking.db' upon its very first run.
>
> The included 'database.sql' file inside this repository is exported 
> from MySQL Workbench and serves strictly as a structural blueprint 
> and reference documentation for the database schema.

----------------------------------------------------------------------
[2] SYSTEM FEATURES & VALIDATIONS
----------------------------------------------------------------------
* Secure Authentication : Strict input validation for alphabetic-only 
                          account names and exactly 4-digit numeric PINs.
* Balance Inquiry       : Displays current funds with an organized and 
                          aesthetic ledger format.
* Deposit & Withdrawal  : Comprehensive error-trapping against non-numeric 
                          inputs, special characters, and negative values.
* Fund Transfer Logic   : Real-time database checks to verify if the 
                          recipient account exists, prevents self-transfers, 
                          and validates balance limits.
* Official Receipts     : Generates timestamped terminal thermal receipts 
                          for successful withdrawal and transfer steps.
* Session Handlers      : Clear transactional states with a clean logout 
                          prompt that cycles smoothly back to the Main Menu.

----------------------------------------------------------------------
[3] HOW TO RUN (CHOOSE YOUR OPERATING SYSTEM)
----------------------------------------------------------------------

======================================================================
METHOD A: FOR macOS USERS (Quickest & Recommended)
======================================================================
MacOS includes built-in support for the SQLite3 environment.

1. Open your VS Code Integrated Terminal inside the project directory.
2. Compile and execute the source code instantly using this command:

   g++ -std=c++17 main.cpp -lsqlite3 -o banking && ./banking

3. The system will boot up. Follow the on-screen menu instructions.

======================================================================
METHOD B: FOR WINDOWS USERS
======================================================================
Windows environments lack built-in C++ compilation tools. Please 
ensure you have a standard compiler toolchain configured (e.g., MinGW).

1. Open your Command Prompt (CMD) or PowerShell inside the folder.
2. Compile the source code into a Windows binary executable:

   g++ -std=c++17 main.cpp -lsqlite3 -o banking.exe

3. Launch the banking platform by running:

   .\banking.exe

----------------------------------------------------------------------
[4] TROUBLESHOOTING FOR WINDOWS COMPILATION ERRORS
----------------------------------------------------------------------
If your compilation step triggers a "sqlite3.h: No such file or directory" 
or linkage breakdown, your local compiler cannot find SQLite3.

QUICK 1-MINUTE PATCH FIX:
1. Visit the official SQLite Download Page: https://www.sqlite.org/download.html
2. Download the "Source Code" (sqlite-amalgamation-XXXXXXX.zip).
3. Extract and copy/drop 'sqlite3.c' and 'sqlite3.h' directly inside this 
   project folder alongside 'main.cpp'.
4. Run this foolproof command:

   g++ -std=c++17 main.cpp sqlite3.c -o banking.exe

5. Execute cleanly using:

   .\banking.exe

----------------------------------------------------------------------
[5] REPOSITORY FILE MAPPING
----------------------------------------------------------------------
 📁 BankingSystem/
  ├── main.cpp            # Driver source containing interface loops
  ├── BankingSystem.h     # Core logic (Validations, Queries, and States)
  ├── Account.h           # Structure models mapping object attributes
  ├── database.sql        # Reference model from MySQL Workbench
  └── README.md           # This deployment documentation guidelines

*Note: 'banking.db' will safely initialize inside this layout once 
the application runs for the first time.
======================================================================