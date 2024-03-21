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
    Node(const string& k, const string& v, const string& u) : key(k), value(v), username(u), next(nullptr) {} //parameters passed on by the constructor
};

class HashMap {
private:
    string filename;

    int hashFunction1(const string& key) {
        int hashValue = 0;
        for (char c : key) {
            hashValue += c;  // hash function that sums up the ASCII values of all characters in the key.
        }
        return hashValue % TABLE_SIZE;  // function returns the hash value modulus the table size to ensure that the  hash value falls within the range of the hash table's size.
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
                    fileOut << current->key << "," << current->value << "," << current->username << endl;  //Writes the key, value, and username of the current node to the file separated by commas.
                    current = current->next;  // Moves the pointer to the next node in the linked list
                }
            }
            fileOut.close();
        }
    }

public:
    HashMap(const string& file) : filename(file) {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = nullptr;  // Indicates that intially all slots in the hash table are empty
        }

        ifstream fileIn(filename);
        if (fileIn.is_open()) {
            string key, value, username;
            while (getline(fileIn, key, ',') && getline(fileIn, value, ',') && getline(fileIn, username)) {
                int index = hashFunction1(key); 
                Node* newNode = new Node(key, value, username);  // Creates a new node with the key, value, and username.
                if (table[index] == nullptr) {  // checks if the slot at the array 'table' is empty at index
                    table[index] = newNode;
                }
                else {
                    Node* temp = table[index];  //Initializes a temporary pointer 'temp' to point to the head of the linked list at index 'index'.
                    while (temp->next != nullptr) {  // Traverses the linked list to find the last node.
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
        while (table[index] != nullptr) {  //Starts a loop that continues until it finds an empty slot in the hash table at the calculated index
            index = (index1 + step * index2) % TABLE_SIZE;  //calculates a new index using a linear probing technique to handle collisions
            ++step;
        }
        table[index] = new Node(key, value, username);  //Once an empty slot is found, it creates a new node with the given key, value, and username, and inserts it into the hash table at the calculated index.
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
    static const int TABLE_SIZE = 100;
    Node* table[TABLE_SIZE];
};

class PasswordManager {
private:
    HashMap users;
    HashMap vault;
    string loggedInUser;

    bool isComplexPassword(const string& password) {
        bool hasAlpha = false;
        bool hasDigit = false;
        for (char c : password) {
            if (isalpha(c))
                hasAlpha = true;
            if (isdigit(c))
                hasDigit = true;
        }
        return password.length() >= 8 && hasAlpha && hasDigit;
    }

public:
    PasswordManager() : users("users.txt"), vault("vault.txt") {}

    void createAccount() {
        string username, password;
        bool uniqueUsername = false;
        bool validPassword = false;

        do {
            cout << "Enter username: ";
            cin >> username;

            if (users.contains(username, "")) {
                cout << "Username already exists. Please choose another one.\n";
            }
            else {
                uniqueUsername = true;
            }
        } while (!uniqueUsername);

        do {
            cout << "Enter password (must contain at least 8 characters including alphabetic and numeric characters): ";
            cin >> password;

            if (isComplexPassword(password)) {
                validPassword = true;
            }
            else {
                cout << "Password is not complex enough. Please try again.\n";
            }
        } while (!validPassword);

        // Encrypt the password before storing it
        string encryptedPassword = XOREncryption::encrypt(password, "KEY");

        users.insert(username, encryptedPassword, "");
        cout << "Account created successfully!\n";
    }

    void login() {
        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        string retrievedPassword = users.get(username, "");

        if (!retrievedPassword.empty() && XOREncryption::decrypt(retrievedPassword, "KEY") == password) {
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
        cout << "Enter username to remove: ";
        cin >> username;

        if (users.contains(username, "")) {
            cout << "Are you sure you want to remove user '" << username << "'? (yes/no): ";
            string confirmation;
            cin >> confirmation;

            if (confirmation == "yes") {
                // Remove the user from the users HashMap
                users.remove(username, "");
                // Remove all entries associated with the user from the vault HashMap
                for (int i = 0; i < TABLE_SIZE; ++i) {
                    Node* current = vault.getTable()[i];
                    Node* prev = nullptr;
                    while (current != nullptr) {
                        if (current->username == username) {
                            if (prev == nullptr) {
                                // The node to be removed is the head of the list
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
                // After successful removal, return to the main menu
                return;
            }
            else {
                cout << "User removal canceled.\n";
            }
        }
        else {
            cout << "User '" << username << "' not found.\n";
        }
        // Return to the main menu even if the removal was canceled or the user was not found
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
            cout << "6. Remove user\n"; // New option
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
                removeUser(); // New case
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
            // Remove the old password from the vault
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
