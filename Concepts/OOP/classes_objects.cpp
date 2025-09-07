#include <iostream>
#include <string>
#include <memory>
#include <vector>

// Basic class definition with encapsulation
class BankAccount {
private:
    std::string account_number_;
    std::string owner_name_;
    double balance_;
    static int total_accounts_;

public:
    // Default constructor
    BankAccount() : account_number_("UNKNOWN"), owner_name_("UNKNOWN"), balance_(0.0) {
        ++total_accounts_;
        std::cout << "Default constructor called for account: " << account_number_ << std::endl;
    }
    
    // Parameterized constructor
    BankAccount(const std::string& account_number, const std::string& owner_name, double initial_balance = 0.0)
        : account_number_(account_number), owner_name_(owner_name), balance_(initial_balance) {
        ++total_accounts_;
        std::cout << "Parameterized constructor called for account: " << account_number_ << std::endl;
    }
    
    // Copy constructor
    BankAccount(const BankAccount& other) 
        : account_number_(other.account_number_ + "_COPY"), 
          owner_name_(other.owner_name_), 
          balance_(other.balance_) {
        ++total_accounts_;
        std::cout << "Copy constructor called for account: " << account_number_ << std::endl;
    }
    
    // Move constructor
    BankAccount(BankAccount&& other) noexcept
        : account_number_(std::move(other.account_number_)),
          owner_name_(std::move(other.owner_name_)),
          balance_(other.balance_) {
        other.balance_ = 0.0;
        ++total_accounts_;
        std::cout << "Move constructor called for account: " << account_number_ << std::endl;
    }
    
    // Copy assignment operator
    BankAccount& operator=(const BankAccount& other) {
        if (this != &other) {
            account_number_ = other.account_number_ + "_ASSIGNED";
            owner_name_ = other.owner_name_;
            balance_ = other.balance_;
            std::cout << "Copy assignment operator called for account: " << account_number_ << std::endl;
        }
        return *this;
    }
    
    // Move assignment operator
    BankAccount& operator=(BankAccount&& other) noexcept {
        if (this != &other) {
            account_number_ = std::move(other.account_number_);
            owner_name_ = std::move(other.owner_name_);
            balance_ = other.balance_;
            other.balance_ = 0.0;
            std::cout << "Move assignment operator called for account: " << account_number_ << std::endl;
        }
        return *this;
    }
    
    // Destructor
    ~BankAccount() {
        --total_accounts_;
        std::cout << "Destructor called for account: " << account_number_ << std::endl;
    }
    
    // Public interface methods (getters)
    std::string getAccountNumber() const { return account_number_; }
    std::string getOwnerName() const { return owner_name_; }
    double getBalance() const { return balance_; }
    
    // Public interface methods (setters with validation)
    void setOwnerName(const std::string& name) {
        if (!name.empty()) {
            owner_name_ = name;
        }
    }
    
    // Business logic methods
    bool deposit(double amount) {
        if (amount > 0) {
            balance_ += amount;
            std::cout << "Deposited $" << amount << " to account " << account_number_ 
                      << ". New balance: $" << balance_ << std::endl;
            return true;
        }
        std::cout << "Invalid deposit amount: $" << amount << std::endl;
        return false;
    }
    
    bool withdraw(double amount) {
        if (amount > 0 && amount <= balance_) {
            balance_ -= amount;
            std::cout << "Withdrew $" << amount << " from account " << account_number_ 
                      << ". New balance: $" << balance_ << std::endl;
            return true;
        }
        std::cout << "Invalid withdrawal amount: $" << amount 
                  << " (Balance: $" << balance_ << ")" << std::endl;
        return false;
    }
    
    void transfer(BankAccount& to_account, double amount) {
        if (withdraw(amount)) {
            to_account.deposit(amount);
            std::cout << "Transferred $" << amount << " from " << account_number_ 
                      << " to " << to_account.account_number_ << std::endl;
        }
    }
    
    // Static method
    static int getTotalAccounts() {
        return total_accounts_;
    }
    
    // Const member function
    void displayAccountInfo() const {
        std::cout << "Account: " << account_number_ 
                  << ", Owner: " << owner_name_ 
                  << ", Balance: $" << balance_ << std::endl;
    }
    
    // Friend function declaration
    friend std::ostream& operator<<(std::ostream& os, const BankAccount& account);
    
    // Operator overloading
    BankAccount operator+(const BankAccount& other) const {
        return BankAccount("MERGED_" + account_number_ + "_" + other.account_number_,
                          owner_name_ + "+" + other.owner_name_,
                          balance_ + other.balance_);
    }
    
    bool operator==(const BankAccount& other) const {
        return account_number_ == other.account_number_;
    }
    
    bool operator<(const BankAccount& other) const {
        return balance_ < other.balance_;
    }
};

// Static member definition
int BankAccount::total_accounts_ = 0;

// Friend function definition
std::ostream& operator<<(std::ostream& os, const BankAccount& account) {
    os << "BankAccount{" << account.account_number_ 
       << ", " << account.owner_name_ 
       << ", $" << account.balance_ << "}";
    return os;
}

// Example of composition
class Address {
private:
    std::string street_;
    std::string city_;
    std::string postal_code_;

public:
    Address(const std::string& street, const std::string& city, const std::string& postal_code)
        : street_(street), city_(city), postal_code_(postal_code) {}
    
    std::string getFullAddress() const {
        return street_ + ", " + city_ + " " + postal_code_;
    }
    
    void display() const {
        std::cout << street_ << ", " << city_ << " " << postal_code_;
    }
};

class Customer {
private:
    std::string name_;
    Address address_;  // Composition: Customer HAS-A Address
    std::vector<std::unique_ptr<BankAccount>> accounts_;

public:
    Customer(const std::string& name, const Address& address)
        : name_(name), address_(address) {}
    
    void addAccount(std::unique_ptr<BankAccount> account) {
        accounts_.push_back(std::move(account));
    }
    
    void displayCustomerInfo() const {
        std::cout << "Customer: " << name_ << std::endl;
        std::cout << "Address: ";
        address_.display();
        std::cout << std::endl;
        std::cout << "Accounts:" << std::endl;
        for (const auto& account : accounts_) {
            account->displayAccountInfo();
        }
    }
    
    double getTotalBalance() const {
        double total = 0.0;
        for (const auto& account : accounts_) {
            total += account->getBalance();
        }
        return total;
    }
};

// Nested classes example
class Bank {
private:
    std::string bank_name_;
    
public:
    class Branch {
    private:
        std::string branch_code_;
        std::string location_;
        
    public:
        Branch(const std::string& code, const std::string& location)
            : branch_code_(code), location_(location) {}
        
        std::string getBranchInfo() const {
            return "Branch " + branch_code_ + " at " + location_;
        }
    };
    
    Bank(const std::string& name) : bank_name_(name) {}
    
    Branch createBranch(const std::string& code, const std::string& location) {
        return Branch(code, location);
    }
    
    std::string getBankName() const { return bank_name_; }
};

void demonstrateBasicOOP() {
    std::cout << "=== Basic OOP Concepts Demonstration ===\n\n";
    
    // 1. Object creation and constructor calls
    std::cout << "1. Object Creation:\n";
    BankAccount account1;  // Default constructor
    BankAccount account2("ACC001", "John Doe", 1000.0);  // Parameterized constructor
    BankAccount account3("ACC002", "Jane Smith");  // Default parameter
    
    std::cout << "\nTotal accounts created: " << BankAccount::getTotalAccounts() << "\n\n";
    
    // 2. Encapsulation - using public interface
    std::cout << "2. Encapsulation (Public Interface):\n";
    account2.deposit(500.0);
    account2.withdraw(200.0);
    account2.displayAccountInfo();
    
    // Direct access to private members would cause compilation error:
    // account2.balance_ = 10000.0;  // ERROR: private member
    
    std::cout << "\n3. Object Interaction:\n";
    account2.transfer(account3, 150.0);
    
    std::cout << "\n4. Copy Operations:\n";
    BankAccount account4 = account2;  // Copy constructor
    BankAccount account5("ACC003", "Bob Wilson");
    account5 = account3;  // Copy assignment
    
    std::cout << "\n5. Move Operations:\n";
    BankAccount account6 = std::move(account4);  // Move constructor
    BankAccount account7("ACC004", "Alice Brown");
    account7 = std::move(account5);  // Move assignment
    
    std::cout << "\n6. Operator Overloading:\n";
    BankAccount merged = account2 + account3;
    std::cout << "Merged account: " << merged << std::endl;
    
    std::cout << "Account2 == Account3: " << (account2 == account3) << std::endl;
    std::cout << "Account2 < Account3: " << (account2 < account3) << std::endl;
    
    std::cout << "\nFinal total accounts: " << BankAccount::getTotalAccounts() << "\n\n";
}

void demonstrateComposition() {
    std::cout << "=== Composition Demonstration ===\n\n";
    
    Address address("123 Main St", "Springfield", "12345");
    Customer customer("John Doe", address);
    
    // Create accounts and add them to customer
    auto account1 = std::make_unique<BankAccount>("ACC001", "John Doe", 1000.0);
    auto account2 = std::make_unique<BankAccount>("ACC002", "John Doe", 2500.0);
    
    customer.addAccount(std::move(account1));
    customer.addAccount(std::move(account2));
    
    customer.displayCustomerInfo();
    std::cout << "Total customer balance: $" << customer.getTotalBalance() << "\n\n";
}

void demonstrateNestedClasses() {
    std::cout << "=== Nested Classes Demonstration ===\n\n";
    
    Bank bank("First National Bank");
    Bank::Branch branch1 = bank.createBranch("BR001", "Downtown");
    Bank::Branch branch2 = bank.createBranch("BR002", "Uptown");
    
    std::cout << "Bank: " << bank.getBankName() << std::endl;
    std::cout << branch1.getBranchInfo() << std::endl;
    std::cout << branch2.getBranchInfo() << std::endl;
    
    std::cout << std::endl;
}

void demonstrateObjectLifecycle() {
    std::cout << "=== Object Lifecycle Demonstration ===\n\n";
    
    std::cout << "Creating objects in local scope:\n";
    {
        BankAccount temp1("TEMP001", "Temporary User", 100.0);
        BankAccount temp2 = temp1;  // Copy constructor
        
        std::cout << "Objects created in scope\n";
        std::cout << "Total accounts: " << BankAccount::getTotalAccounts() << std::endl;
    }  // Destructors called here
    
    std::cout << "After scope exit, total accounts: " << BankAccount::getTotalAccounts() << std::endl;
    
    std::cout << "\nDynamic allocation:\n";
    BankAccount* dynamic_account = new BankAccount("DYN001", "Dynamic User", 500.0);
    std::cout << "Dynamic account created, total: " << BankAccount::getTotalAccounts() << std::endl;
    
    delete dynamic_account;  // Manual cleanup
    std::cout << "After delete, total accounts: " << BankAccount::getTotalAccounts() << std::endl;
    
    std::cout << "\nSmart pointer (RAII):\n";
    {
        auto smart_account = std::make_unique<BankAccount>("SMART001", "Smart User", 1500.0);
        std::cout << "Smart pointer account created, total: " << BankAccount::getTotalAccounts() << std::endl;
    }  // Automatic cleanup
    std::cout << "After smart pointer scope, total: " << BankAccount::getTotalAccounts() << std::endl;
    
    std::cout << std::endl;
}

// Example of const correctness
class ConstExample {
private:
    mutable int cache_value_;  // mutable allows modification in const methods
    int regular_value_;

public:
    ConstExample(int value) : cache_value_(0), regular_value_(value) {}
    
    // Const method - cannot modify non-mutable members
    int getValue() const {
        cache_value_ = regular_value_ * 2;  // mutable member can be modified
        return regular_value_;
    }
    
    // Non-const method
    void setValue(int value) {
        regular_value_ = value;
    }
    
    // Const overloading
    int& getReference() { return regular_value_; }
    const int& getReference() const { return regular_value_; }
};

void demonstrateConstCorrectness() {
    std::cout << "=== Const Correctness Demonstration ===\n\n";
    
    ConstExample obj(42);
    const ConstExample const_obj(84);
    
    std::cout << "Non-const object value: " << obj.getValue() << std::endl;
    std::cout << "Const object value: " << const_obj.getValue() << std::endl;
    
    // obj.setValue(100);  // OK - non-const object
    // const_obj.setValue(100);  // ERROR - const object
    
    int& ref = obj.getReference();  // Non-const version
    const int& const_ref = const_obj.getReference();  // Const version
    
    ref = 200;  // OK - can modify through non-const reference
    // const_ref = 300;  // ERROR - cannot modify through const reference
    
    std::cout << "Modified value: " << obj.getValue() << std::endl;
    
    std::cout << std::endl;
}

int main() {
    demonstrateBasicOOP();
    demonstrateComposition();
    demonstrateNestedClasses();
    demonstrateObjectLifecycle();
    demonstrateConstCorrectness();
    
    std::cout << "=== Key OOP Concepts Covered ===\n";
    std::cout << "1. Classes and Objects\n";
    std::cout << "2. Encapsulation (private/public)\n";
    std::cout << "3. Constructors and Destructors\n";
    std::cout << "4. Copy and Move Semantics\n";
    std::cout << "5. Operator Overloading\n";
    std::cout << "6. Static Members\n";
    std::cout << "7. Composition (HAS-A relationship)\n";
    std::cout << "8. Nested Classes\n";
    std::cout << "9. Const Correctness\n";
    std::cout << "10. Friend Functions\n";
    std::cout << "11. Object Lifecycle Management\n";
    
    return 0;
}
