
#include <iostream>  // For input and output (cin, cout)
#include <fstream>   // For reading and writing files
#include <vector>    // To store multiple farmer records
#include <string>    // To use strings
#include <iomanip>   // For formatting queue tokens
#include <limits>    // To clear invalid input
#include <sstream>   // To create token strings
using namespace std;

// Class to store all details of one farmer
class Farmer {
public:
    string id, name, village, crop;
    string slot, token, status;
    double quantity;  // Quantity of crop in kilograms

    // Constructor: gives default values to a new farmer
    Farmer() {
        quantity = 0;
        slot = "Not Booked";
        token = "Not Assigned";
        status = "Not Booked";
    }
};

// Vector stores all registered farmers
vector<Farmer> farmers;

// Stores the number for the next queue token
int nextToken = 1;

// Find a farmer using their unique ID
// Returns the farmer's position in the vector
// Returns -1 if the farmer does not exist
int findFarmer(string id) {
    for (int i = 0; i < (int)farmers.size(); i++) {
        if (farmers[i].id == id)
            return i;
    }
    return -1;
}

// Save all farmer records in a text file
// This allows the records to remain after closing the program
void saveData() {
    ofstream file("farmers.txt");

    for (const auto &f : farmers) {
        // quoted() safely stores strings containing spaces
        file << quoted(f.id) << " "
             << quoted(f.name) << " "
             << quoted(f.village) << " "
             << quoted(f.crop) << " "
             << f.quantity << " "
             << quoted(f.slot) << " "
             << quoted(f.token) << " "
             << quoted(f.status) << "\n";
    }

    file.close(); // Close the file after saving
}

// Load previously saved farmer records
// This function runs when the program starts
void loadData() {
    ifstream file("farmers.txt");
    Farmer f;

    // Read each saved farmer record from the file
    while (file >> quoted(f.id)
                >> quoted(f.name)
                >> quoted(f.village)
                >> quoted(f.crop)
                >> f.quantity
                >> quoted(f.slot)
                >> quoted(f.token)
                >> quoted(f.status)) {

        farmers.push_back(f); // Add farmer to the vector

        // Find the largest existing token number
        // so that token numbers are not reused after restart
        if (f.token != "Not Assigned" &&
            f.token.size() > 1) {
            try {
                int n = stoi(f.token.substr(1));

                if (n >= nextToken)
                    nextToken = n + 1;
            } catch (...) {
                // Ignore an invalid token in the saved file
            }
        }
    }
}

// Register a new farmer
void registerFarmer() {
    Farmer f; // Create a new farmer object

    cout << "Enter Farmer ID: ";
    getline(cin, f.id);

    // Do not allow an empty or duplicate farmer ID
    if (f.id.empty() || findFarmer(f.id) != -1) {
        cout << "Invalid or duplicate Farmer ID!\n";
        return; // Stop this function
    }

    // Get the farmer's personal and crop details
    cout << "Enter Name: ";
    getline(cin, f.name);

    cout << "Enter Village: ";
    getline(cin, f.village);

    cout << "Enter Crop: ";
    getline(cin, f.crop);

    cout << "Enter Quantity in kg: ";

    // Check that quantity is a valid positive number
    if (!(cin >> f.quantity) || f.quantity <= 0) {
        cout << "Invalid quantity!\n";

        // Clear the input error
        cin.clear();

        // Remove invalid input from the input buffer
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    // Remove the leftover newline before using getline again
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    farmers.push_back(f); // Add farmer to the vector
    saveData();            // Save records to the file

    cout << "Farmer registered successfully!\n";
}

// Book a procurement slot and generate a token
void bookSlot() {
    string id;

    cout << "Enter Farmer ID: ";
    getline(cin, id);

    int i = findFarmer(id); // Find the farmer's position

    if (i == -1) {
        cout << "Farmer not found!\n";
        return;
    }

    // Prevent the same farmer from getting another token
    if (farmers[i].token != "Not Assigned") {
        cout << "Token already assigned: "
             << farmers[i].token << "\n";
        return;
    }

    // Ask the farmer to select a time slot
    cout << "1. Morning\n2. Afternoon\n";
    cout << "Choose slot: ";

    int choice;

    // Accept only 1 or 2
    if (!(cin >> choice) || (choice != 1 && choice != 2)) {
        cout << "Invalid slot!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // Set the selected time slot
    farmers[i].slot =
        (choice == 1) ? "Morning" : "Afternoon";

    // Generate tokens such as T001, T002, T003
    ostringstream os;
    os << "T" << setw(3) << setfill('0') << nextToken++;

    farmers[i].token = os.str();

    // A newly booked farmer starts in the waiting queue
    farmers[i].status = "Waiting";

    saveData(); // Save the updated record

    cout << "Slot booked successfully!\n";
    cout << "Queue Token: " << farmers[i].token << "\n";
    cout << "Slot: " << farmers[i].slot << "\n";
}

// Display farmers who are still in the active queue
void viewQueue() {
    cout << "\n--- PROCUREMENT QUEUE ---\n";
    bool found = false;

    for (const auto &f : farmers) {
        // Exclude farmers who have been paid or rejected
        if (f.token != "Not Assigned" &&
            f.status != "Paid" &&
            f.status != "Rejected") {

            cout << "Token: " << f.token
                 << " | Name: " << f.name
                 << " | Slot: " << f.slot
                 << " | Status: " << f.status << "\n";

            found = true;
        }
    }

    if (!found)
        cout << "No farmers in the active queue.\n";
}

// Search for a farmer and display their details
void trackFarmer() {
    string id;

    cout << "Enter Farmer ID: ";
    getline(cin, id);

    int i = findFarmer(id);

    // Check whether the farmer exists
    if (i == -1) {
        cout << "Farmer not found!\n";
        return;
    }

    // Copy the farmer record to a local object
    Farmer f = farmers[i];

    // Display the farmer's complete information
    cout << "\n--- FARMER DETAILS ---\n";
    cout << "Name: " << f.name << "\n";
    cout << "Village: " << f.village << "\n";
    cout << "Crop: " << f.crop << "\n";
    cout << "Quantity: " << f.quantity << " kg\n";
    cout << "Slot: " << f.slot << "\n";
    cout << "Token: " << f.token << "\n";
    cout << "Status: " << f.status << "\n";
}

// Update the farmer's procurement status
void updateStatus() {
    string id;

    cout << "Enter Farmer ID: ";
    getline(cin, id);

    int i = findFarmer(id);

    if (i == -1) {
        cout << "Farmer not found!\n";
        return;
    }

    // A slot must be booked before status updates
    if (farmers[i].token == "Not Assigned") {
        cout << "Book a slot before updating status.\n";
        return;
    }

    // Display the available procurement stages
    cout << "1. Waiting\n";
    cout << "2. Weighing\n";
    cout << "3. Accepted\n";
    cout << "4. Payment Pending\n";
    cout << "5. Paid\n";
    cout << "6. Rejected\n";
    cout << "Choose new status: ";

    int choice;

    // Validate the selected status
    if (!(cin >> choice) || choice < 1 || choice > 6) {
        cout << "Invalid choice!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // Store all status names in an array
    string statuses[] = {
        "Waiting", "Weighing", "Accepted",
        "Payment Pending", "Paid", "Rejected"
    };

    // Array indexes start at zero, so subtract one
    farmers[i].status = statuses[choice - 1];

    saveData(); // Save the updated status

    cout << "Status updated to: "
         << farmers[i].status << "\n";
}

// Display statistics about all registered farmers
void dashboard() {
    int waiting = 0, weighing = 0, accepted = 0;
    int payment = 0, paid = 0, rejected = 0;

    // Count farmers according to their current status
    for (const auto &f : farmers) {
        if (f.status == "Waiting") waiting++;
        else if (f.status == "Weighing") weighing++;
        else if (f.status == "Accepted") accepted++;
        else if (f.status == "Payment Pending") payment++;
        else if (f.status == "Paid") paid++;
        else if (f.status == "Rejected") rejected++;
    }

    // Print the dashboard summary
    cout << "\n--- PROCUREMENT DASHBOARD ---\n";
    cout << "Registered Farmers: " << farmers.size() << "\n";
    cout << "Waiting: " << waiting << "\n";
    cout << "Weighing: " << weighing << "\n";
    cout << "Accepted: " << accepted << "\n";
    cout << "Payment Pending: " << payment << "\n";
    cout << "Paid: " << paid << "\n";
    cout << "Rejected: " << rejected << "\n";
}

// Program execution starts from main()
int main() {
    loadData(); // Restore records from the previous session

    int choice;

    // Repeat the menu until the user selects Exit
    do {
        cout << "\n===== SMART FARMER PROCUREMENT =====\n";
        cout << "1. Register Farmer\n";
        cout << "2. Book Procurement Slot\n";
        cout << "3. View Queue\n";
        cout << "4. Track Farmer\n";
        cout << "5. Update Procurement Status\n";
        cout << "6. View Dashboard\n";
        cout << "7. Exit\n";
        cout << "Enter choice: ";

        // Ensure the menu choice is a number
        if (!(cin >> choice)) {
            cout << "Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue; // Display the menu again
        }

        // Remove the newline before calling getline
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Call the function selected by the user
        switch (choice) {
            case 1: registerFarmer(); break;
            case 2: bookSlot(); break;
            case 3: viewQueue(); break;
            case 4: trackFarmer(); break;
            case 5: updateStatus(); break;
            case 6: dashboard(); break;
            case 7: cout << "Thank you!\n"; break;
            default: cout << "Invalid choice!\n";
        }

    } while (choice != 7);

    return 0; // End the program successfully
}

