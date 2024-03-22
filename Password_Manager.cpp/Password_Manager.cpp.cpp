
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const int TABLE_SIZE = 100;

class XOREncryption {
public:
    static string encrypt(const string& plaintext, const string& key) {
        string ciphertext = "";
        for (size_t i = 0; i < plaintext.length(); ++i) {
            ciphertext += plaintext[i] ^ key[i % key.length()];
        }
        return ciphertext;
    }

    static string decrypt(const string& ciphertext, const string& key) {
        string plaintext = "";
        for (size_t i = 0; i < ciphertext.length(); ++i) {
            plaintext += ciphertext[i] ^ key[i % key.length()];
        }
        return plaintext;
    }
};

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
        return hashValue % TABLE_SIZE;
    }

    void saveToFile(Node* table[]) {
        ofstream fileOut(filename);
        if (fileOut.is_open()) {
            for (int i = 0; i < TABLE_SIZE; ++i) {
                Node* current = table[i];
                while (current != nullptr) {
                    fileOut << current->key << "," << current->value << "," << current->username << endl;
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
            while (getline(fileIn, key, ',') && getline(fileIn, value, ',') && getline(fileIn, username)) {
                int index = hashFunction1(key); // For simplicity, using only one hash function for loading
                Node* newNode = new Node(key, value, username);
                if (table[index] == nullptr) {
                    table[index] = newNode;
                }
                else {
                    Node* temp = table[index];
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = newNode;
                }
            }
            fileIn.close();
        }
    }


    ~HashMap() {
        saveToFile(table);
    }

    void insert(const string& key, const string& value, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            index = (index1 + step * index2) % TABLE_SIZE;
            ++step;
        }
        table[index] = new Node(key, value, username);
    }

    string get(const string& key, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            if (table[index]->key == key && table[index]->username == username) {
                return table[index]->value;
            }
            index = (index1 + step * index2) % TABLE_SIZE;
            ++step;
        }
        return "";
    }

    bool contains(const string& key, const string& username) {
        int index1 = hashFunction1(key);
        int index2 = hashFunction2(key);
        int index = index1;
        int step = 1;
        while (table[index] != nullptr) {
            if (table[index]->key == key && table[index]->username == username) {
                return true;
            }
            index = (index1 + step * index2) % TABLE_SIZE;
            ++step;
        }
        return false;
    }

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
            index = (index1 + step * index2) % TABLE_SIZE;
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
        return password.length() >= 8 && hasUpper && hasLower && hasDigit;
    }

public:
    PasswordManager() : users("users.txt"), vault("vault.txt") {}

    void createAccount() {
        string username, password;
        bool uniqueUsername = false;
        bool validPassword = false;

        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
        if (users.contains(username, "")) {
            cout << "Username already exists. Please choose another one.\n";
        }
        else {
            users.insert(username, password, "");
            cout << "Account created successfully!\n";
        }
    }

    void login() {
        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
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
    void removeUser() {
        string username;
        cout << "Confirm your username: ";
        cin >> username;

        if (users.contains(username, "")) {
            cout << "Are you sure you want to remove user '" << username << "'? (yes/no): ";
            string confirmation;
            cin >> confirmation;

            if (confirmation == "yes") {
                users.remove(username, "");
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
