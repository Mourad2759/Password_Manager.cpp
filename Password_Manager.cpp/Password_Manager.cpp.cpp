#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const int TABLE_SIZE = 100;

// Encryption key for Caesar cipher
const int SHIFT = 3;

// Function to encrypt a string using Caesar cipher
string encrypt(const string& plaintext) {
    string ciphertext = "";
    for (char c : plaintext) {
        if (isalpha(c)) {
            char shifted = c + SHIFT;
            if ((isupper(c) && shifted > 'Z') || (islower(c) && shifted > 'z')) {
                shifted -= 26; // Wrap around if shifted character exceeds the alphabet range
            }
            ciphertext += shifted;
        }
        else {
            ciphertext += c; // Non-alphabetic characters remain unchanged
        }
    }
    return ciphertext;
}

// Function to decrypt a string using Caesar cipher
string decrypt(const string& ciphertext) {
    string plaintext = "";
    for (char c : ciphertext) {
        if (isalpha(c)) {
            char shifted = c - SHIFT;
            if ((isupper(c) && shifted < 'A') || (islower(c) && shifted < 'a')) {
                shifted += 26; // Wrap around if shifted character goes below the alphabet range
            }
            plaintext += shifted;
        }
        else {
            plaintext += c; // Non-alphabetic characters remain unchanged
        }
    }
    return plaintext;
}

struct Node {
    string key;
    string value;
    string username;
    Node* next;
    Node(const string& k, const string& v, const string& u) : key(k), value(v), username(u), next(nullptr) {}
};

class HashMap {
private:
    string filename;

    int hashFunction1(const string& key) {
        int hashValue = 0;
        for (char c : key) {
            hashValue += c;
        }
        return hashValue % TABLE_SIZE;
    }

    int hashFunction2(const string& key) {
        int hashValue = 0;
        for (char c : key) {
            hashValue += c * 31; // Using a different prime number as a multiplier for the second hash function
        }
        return (hashValue % 97) + 1; // Ensuring the second hash value is non-zero and less than TABLE_SIZE
    }

    // Function to save encrypted data to the file
    void saveToFile(Node* table[]) {
        // Open the file for writing
        ofstream fileOut(filename);
        if (fileOut.is_open()) {
            // Iterates through the hash table
            for (int i = 0; i < TABLE_SIZE; ++i) {
                Node* current = table[i];
                // Traverses the linked list at each index
                while (current != nullptr) {
                    // Write encrypted key, value, and username to the file
                    fileOut << encrypt(current->key) << "," << encrypt(current->value) << "," << encrypt(current->username) << endl;
                    current = current->next;
                }
            }
            fileOut.close();
        }
    }

public:
    HashMap(const string& file) : filename(file) {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = nullptr;
        }

        ifstream fileIn(filename);
        if (fileIn.is_open()) {
            string key, value, username;
            // Reads data from the file separated by commas
            while (getline(fileIn, key, ',') && getline(fileIn, value, ',') && getline(fileIn, username)) {
                key = decrypt(key);
                value = decrypt(value);
                username = decrypt(username);
                // Calculates hash indexes using hash functions as the key
                int index = hashFunction1(key);
                // Create a new node with decrypted data
                Node* newNode = new Node(key, value, username);
                // Check if the slot at hash index is empty
                if (table[index] == nullptr) {
                    table[index] = newNode;
                }
                else {
                    // Use double hashing for collision resolution
                    int index2 = hashFunction2(key);
                    int step = 1;
                    // used to find the next empty slot in the table
                    while (true) {
                        int newIndex = (index + step * index2) % TABLE_SIZE;
                        if (table[newIndex] == nullptr) {
                            table[newIndex] = newNode;
                            break;
                        }
                        ++step;
                    }
                }
            }
            fileIn.close();
        }
    }

    ~HashMap() {
        saveToFile(table);
    }

    // inserts data into the hash table
    void insert(const string& key, const string& value, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            index = (index + step * index2) % TABLE_SIZE;
            ++step;
        }
        table[index] = new Node(key, value, username);
    }

    // retrieves the value(password) associated with a given key and username
    string get(const string& key, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            if (table[index]->key == key && table[index]->username == username) {
                return table[index]->value;
            }
            index = (index + step * index2) % TABLE_SIZE;
            ++step;
        }
        return "";
    }

    // Check if a given key and username exist in the hash table
    bool contains(const string& key, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            if (table[index]->key == key && table[index]->username == username) {
                return true;
            }
            index = (index + step * index2) % TABLE_SIZE;
            ++step;
        }
        return false;
    }

    // Removes the entry with the given key and username from the hash table
    void remove(const string& key, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            if (table[index]->key == key && table[index]->username == username) {
                delete table[index];
                table[index] = nullptr;
                return;
            }
            index = (index + step * index2) % TABLE_SIZE;
            ++step;
        }
    }

    // Public method to access the table
    Node** getTable() {
        return table;
    }

private:
    Node* table[TABLE_SIZE];
};

class PasswordManager {
private:
    HashMap users;
    HashMap vault;
    string loggedInUser;

    bool isSpecialCharacter(char c) {
        // Define your set of special characters
        const string specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";

        // Check if the character is in the set of special characters
        for (char specialChar : specialChars) {
            if (c == specialChar) {
                return true;
            }
        }

        // If the character is not found in the set, it's not a special character
        return false;
    }

    bool isComplexPassword(const string& password) {
        bool hasUpper = false;
        bool hasLower = false;
        bool hasDigit = false;
        bool hasSpecial = false;
        for (char c : password) {
            if (isupper(c))
                hasUpper = true;
            if (islower(c))
                hasLower = true;
            if (isdigit(c))
                hasDigit = true;
            if (isSpecialCharacter(c))
                hasSpecial = true;
        }
        return password.length() >= 8 && hasUpper && hasLower && hasDigit && hasSpecial;
    }

public:
    PasswordManager() : users("users.txt"), vault("vault.txt") {}

    void createAccount() {
        string username, password;
        bool uniqueUsername = false;
        bool validPassword = false;

        cout << "Enter username: ";
        cin >> username;

        // Prompts the user until a unique username is entered
        while (users.contains(username, "")) {
            cout << "Username already exists. Please choose another one: ";
            cin >> username;
        }

        // Prompts the user until a complex password is entered
        while (!validPassword) {
            cout << "Enter password: ";
            cin >> password;

            if (isComplexPassword(password)) {
                validPassword = true;
            }
            else {
                cout << "Password must be at least 8 characters long and contain at least one uppercase letter, one lowercase letter, one digit, and one special character.\n";
            }
        }

        users.insert(username, password, "");
        cout << "Account created successfully!\n";
    }

    // Function to prompt user for login credentials and validate them
    void login() {
        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
        // Validates username and password using the users hash map
        if (users.get(username, "") == password) {
            cout << "Login successful!\n";
            loggedInUser = username;
            accessVault();
        }
        else {
            cout << "Invalid username or password.\n";
        }
    }

    void logout() {
        loggedInUser = "";
        cout << "Logged out successfully.\n";
    }

    // Function to remove a user account
    void removeUser() {
        string username;
        cout << "Confirm your username: ";
        cin >> username;

        if (users.contains(username, "")) {
            cout << "Are you sure you want to remove user '" << username << "'? (yes/no): ";
            string confirmation;
            cin >> confirmation;

            if (confirmation == "yes") {

                // Removes the user from the users hash map
                users.remove(username, "");

                // Removes user's entries from the vault hash map
                for (int i = 0; i < TABLE_SIZE; ++i) {
                    Node* current = vault.getTable()[i];
                    Node* prev = nullptr;
                    while (current != nullptr) {
                        if (current->username == username) {
                            if (prev == nullptr) {
                                vault.getTable()[i] = current->next;
                                delete current;
                                current = vault.getTable()[i];
                            }
                            else {
                                prev->next = current->next;
                                delete current;
                                current = prev->next;
                            }
                        }
                        else {
                            prev = current;
                            current = current->next;
                        }
                    }
                }
                cout << "User '" << username << "' removed successfully.\n";
                return;
            }
            else {
                cout << "User removal canceled.\n";
            }
        }
        else {
            cout << "User '" << username << "' not found.\n";
        }
        cout << "Returning to the main menu...\n";
    }

    void accessVault() {
        int choice;
        bool loggedIn = true;
        while (loggedIn) {
            cout << "\nVault Menu:\n";
            cout << "1. Add an application and password\n";
            cout << "2. View passwords\n";
            cout << "3. Remove an application\n";
            cout << "4. Modify a password\n";
            cout << "5. Generate a complex password\n";
            cout << "6. Remove user\n";
            cout << "7. Logout\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
            case 1:
                addApplication();
                break;
            case 2:
                viewPasswords();
                break;
            case 3:
                removeApplication();
                break;
            case 4:
                modifyPassword();
                break;
            case 5:
                generateComplexPassword();
                break;
            case 6:
                removeUser();
                break;
            case 7:
                logout();
                loggedIn = false;
                break;
            default:
                cout << "Invalid choice. Try again.\n";
            }
        }
    }

    // allows users to add an application and it's password
    void addApplication() {
        if (loggedInUser.empty()) {
            cout << "You need to log in first.\n";
            return;
        }
        string appName, password;
        cout << "Enter application name: ";
        cin >> appName;
        cout << "Enter password for " << appName << ": ";
        cin >> password;
        vault.insert(appName, password, loggedInUser);
        cout << "Password added successfully.\n";
    }

    // Let's users view their passwords by retrieving them from the vault.txt file
    void viewPasswords() {
        if (loggedInUser.empty()) {
            cout << "You need to log in first.\n";
            return;
        }
        cout << "Stored Passwords:\n";
        Node** table = vault.getTable();
        for (int i = 0; i < TABLE_SIZE; ++i) {
            Node* current = table[i];
            while (current != nullptr) {
                if (current->username == loggedInUser) {
                    cout << current->key << " : " << current->value << endl;
                }
                current = current->next;
            }
        }
    }

    // allows the users to remove passwords from their vault
    void removeApplication() {
        if (loggedInUser.empty()) {
            cout << "You need to log in first.\n";
            return;
        }
        string appName;
        cout << "Enter application name to remove: ";
        cin >> appName;
        vault.remove(appName, loggedInUser);
        cout << "Application removed successfully.\n";
    }

    // allows password modification
    void modifyPassword() {
        if (loggedInUser.empty()) {
            cout << "You need to log in first.\n";
            return;
        }
        string appName, newPassword;
        cout << "Enter application name to modify password: ";
        cin >> appName;

        string currentPassword = vault.get(appName, loggedInUser);
        if (!currentPassword.empty()) {
            vault.remove(appName, loggedInUser);
            cout << "Enter new password for " << appName << ": ";
            cin >> newPassword;
            vault.insert(appName, newPassword, loggedInUser);
            cout << "Password modified successfully.\n";
        }
        else {
            cout << "Application not found in the vault.\n";
        }
    }

    // generates a complex password
    void generateComplexPassword() {
        string appName;
        cout << "Enter application name for the new password: ";
        cin >> appName;
        string newPassword = generateRandomPassword(12);
        vault.insert(appName, newPassword, loggedInUser);
        cout << "Generated complex password for " << appName << ": " << newPassword << endl;
    }

    string generateRandomPassword(int length) {
        const string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
        string password;
        srand(static_cast<unsigned int>(time(nullptr)));
        for (int i = 0; i < length; ++i) {
            password += charset[rand() % charset.length()];
        }
        return password;
    }
};

int main() {
    PasswordManager manager;
    int choice;
    do {
        cout << "\nMain Menu:\n";
        cout << "1. Create an account\n";
        cout << "2. Log in\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            manager.createAccount();
            break;
        case 2:
            manager.login();
            break;
        case 3:
            cout << "Exiting...\n";
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 3);

    return 0;
}
