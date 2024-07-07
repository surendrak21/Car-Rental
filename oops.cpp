#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <ctime>
#include <random>

using namespace std;


class Tools {
    public:
        static string getTodayDate() {
            time_t now = time(0);
            tm* ltm = localtime(&now);
            int year = 1900 + ltm->tm_year;
            int month = 1 + ltm->tm_mon;
            int day = ltm->tm_mday;

            string date = to_string(year) + "-" + zeroPad(month) + "-" + zeroPad(day);
            return date;
        }

        static string generateRandomID() {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<int> distrib(0, 9);

            string randomID;
            for (int i = 0; i < 5; ++i) {
                randomID += to_string(distrib(gen));
            }
            return randomID;
        }

        static int calculateDateDifference(const string& date1, const string& date2) {
            tm tm1 = parseDate(date1);
            tm tm2 = parseDate(date2);

            time_t time1 = mktime(&tm1);
            time_t time2 = mktime(&tm2);

            const int seconds_per_day = 60 * 60 * 24;
            return abs((int)difftime(time1, time2) / seconds_per_day);
        }

    private:
        static tm parseDate(const string& dateStr) {
            tm tm = {};
            sscanf(dateStr.c_str(), "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
            tm.tm_year -= 1900; 
            tm.tm_mon -= 1;      
            return tm;
        }

        static string zeroPad(int number) {
            if (number < 10) {
                return "0" + to_string(number);
            }
            return to_string(number);
        }
};


class Customer {
    protected:
        string filename;
        unordered_set<string> usernames;
        vector<string> userData;

    public:
        Customer(string filename) : filename(filename) {
            loadUsernames();
        }

        bool registerUser(const string& username, const string& password, const string& role) {
            if (usernames.find(username) != usernames.end()) {
                cout << "User with username '" << username << "' already exists" << endl;
                return false;
            }

            ofstream file(filename, ios_base::app);
            if (!file.is_open()) {
                cerr << "Error opening file!" << endl;
                return false;
            }

            file << username << "," << password << "," << role << ",\n"; // Add a comma at the end to separate car IDs
            file.close();

            cout << "User registered successfully! Username: " << username << endl;

            usernames.insert(username);
            userData.push_back(username + "," + password + "," + role + ",");
            return true;
        }

        bool loginUser(const string& username, const string& password) {
            for (const string& user : userData) {
                stringstream ss(user);
                string savedUsername, savedPassword;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                if (savedUsername == username && savedPassword == password) {
                    return true;
                }
            }
            return false;
        }

        bool rentCars(const string& username, const vector<pair<string, pair<string, pair<string, string>>>>& carRentals) {
            for (string& user : userData) {
                stringstream ss(user);
                string savedUsername, savedPassword, savedRole, savedCars;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                getline(ss, savedRole, ',');
                getline(ss, savedCars); // Read the rest of the line, containing the cars
                if (savedUsername == username) {
                    for (const auto& rental : carRentals) {
                        const string& carID = rental.first;
                        const string& rentingDate = rental.second.first;
                        const string& condition = rental.second.second.first;
                        const string& price = rental.second.second.second;
                        if (savedCars.find(carID) != string::npos) {
                            cout << "Car " << carID << " already rented by the user" << endl;
                        } else {
                            savedCars += carID + ";" + rentingDate + ";" + condition + ";" + price + ",";
                        }
                    }
                    user = savedUsername + "," + savedPassword + "," + savedRole + "," + savedCars;
                    rewriteUserDataFile();
                    return true;
                }
            }
            cout << "User not found!" << endl;
            return false;
        }

        bool updatePassword(const string& username, const string& newPassword) {
            auto it = find_if(userData.begin(), userData.end(), [username](const string& user) {
                stringstream ss(user);
                string savedUsername, savedPassword;
                getline(ss, savedUsername, ',');
                return savedUsername == username;
            });

            if (it != userData.end()) {
                size_t pos = it - userData.begin();
                stringstream ss(userData[pos]);
                string savedUsername, savedPassword, savedRole, savedCars;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                getline(ss, savedRole, ',');
                getline(ss, savedCars); // Read the rest of the line, containing the cars
                userData[pos] = savedUsername + "," + newPassword + "," + savedRole + "," + savedCars;
                rewriteUserDataFile();
                cout << "Password for user '" << username << "' updated successfully" << endl;
                return true;
            } else {
                cout << "User '" << username << "' not found" << endl;
                return false;
            }
        }

        bool deleteUser(const string& username) {
            auto it = find_if(userData.begin(), userData.end(), [username](const string& user) {
                stringstream ss(user);
                string savedUsername;
                getline(ss, savedUsername, ',');
                return savedUsername == username;
            });

            if (it != userData.end()) {
                userData.erase(it);
                usernames.erase(username);
                rewriteUserDataFile();
                cout << "User '" << username << "' deleted successfully" << endl;
                return true;
            } else {
                cout << "User '" << username << "' not found" << endl;
                return false;
            }
        }

        bool removeRentedCar(const string& username, const string& carID) {
            for (string& user : userData) {
                stringstream ss(user);
                string savedUsername, savedPassword, savedRole, savedCars;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                getline(ss, savedRole, ',');
                getline(ss, savedCars); // Read the rest of the line, containing the cars
                if (savedUsername == username) {
                    size_t pos = savedCars.find(carID + ";");
                    if (pos != string::npos) {
                        size_t endPos = savedCars.find(",", pos); // Find the end of the car's information
                        savedCars.erase(pos, endPos - pos + 1); // Remove the car's information including the comma
                        user = savedUsername + "," + savedPassword + "," + savedRole + "," + savedCars;
                        rewriteUserDataFile();
                        cout << "Car " << carID << " removed successfully from user " << username << "'s rented cars." << endl;
                        return true;
                    } else {
                        cout << "Car " << carID << " not found among user " << username << "'s rented cars." << endl;
                        return false;
                    }
                }
            }
            cout << "User not found" << endl;
            return false;
        }

        void showRentedCars(const string& username) {
            for (const string& user : userData) {
                stringstream ss(user);
                string savedUsername, savedPassword, savedRole, savedCars;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                getline(ss, savedRole, ',');
                getline(ss, savedCars); // Read the rest of the line, containing the cars
                if (savedUsername == username) {
                    stringstream carsStream(savedCars);
                    string carInfo;
                    cout << "Cars rented by user " << username << ":" << endl;
                    while (getline(carsStream, carInfo, ',')) {
                        stringstream carInfoStream(carInfo);
                        string carID, rentingDate, condition, price;
                        getline(carInfoStream, carID, ';');
                        getline(carInfoStream, rentingDate, ';');
                        getline(carInfoStream, condition, ';');
                        getline(carInfoStream, price, ';');
                        cout << "Car ID: " << carID << ", Renting Date: " << rentingDate << ", Condition: " << condition << ", Price: " << price << endl;
                    }
                    if (savedCars.empty()) {
                        cout << "User " << username << " has not rented any cars." << endl;
                    }
                    return;
                }
            }
            cout << "User not found!" << endl;
        }

         string getRentingDate(const string& username, const string& carID) {
            for (const string& user : userData) {
                stringstream ss(user);
                string savedUsername, savedPassword, savedRole, savedCars;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                getline(ss, savedRole, ',');
                getline(ss, savedCars); // Read the rest of the line, containing the cars
                if (savedUsername == username) {
                    stringstream carsStream(savedCars);
                    string carInfo;
                    while (getline(carsStream, carInfo, ',')) {
                        stringstream carInfoStream(carInfo);
                        string currentCarID, rentingDate, condition, price;
                        getline(carInfoStream, currentCarID, ';');
                        getline(carInfoStream, rentingDate, ';');
                        getline(carInfoStream, condition, ';');
                        getline(carInfoStream, price, ';');
                        if (currentCarID == carID) {
                            return rentingDate;
                        }
                    }
                    return "Car not found";
                }
            }
            return "User not found";
        }

        int calculateFine(const string& carOwnedDate) {
            string currentDate = Tools::getTodayDate();
            int daysDifference = Tools::calculateDateDifference(currentDate, carOwnedDate);
            return daysDifference * 10; 
        }


    protected:
        void loadUsernames() {
            ifstream file(filename);
            if (!file.is_open()) {
                cerr << "Error opening file" << endl;
                return;
            }
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string savedUsername, savedPassword, savedRole, savedCars;
                getline(ss, savedUsername, ',');
                getline(ss, savedPassword, ',');
                getline(ss, savedRole, ',');
                getline(ss, savedCars); // Read the rest of the line, containing the cars
                usernames.insert(savedUsername);
                userData.push_back(line);
            }
            file.close();
        }

        void rewriteUserDataFile() {
            ofstream file(filename);
            if (!file.is_open()) {
                cerr << "Error opening file" << endl;
                return;
            }
            for (const string& user : userData) {
                file << user << "\n";
            }
            file.close();
        }
};

class Time {
public:
    void tellTime() {
        // Get the current time
        std::time_t currentTime = std::time(nullptr);

        // Convert the current time to a string
        std::string timeString = std::ctime(&currentTime);

        // Print the current time
        std::cout << "The current time is: " << timeString;
    }
};

class Employee : public Customer {
public:
    Employee(string filename) : Customer(filename) {}

    bool rentCars(const string& username, const vector<pair<string, pair<string, pair<string, string>>>>& carRentals) {
        for (string& user : userData) {
            stringstream ss(user);
            string savedUsername, savedPassword, savedRole, savedCars;
            getline(ss, savedUsername, ',');
            getline(ss, savedPassword, ',');
            getline(ss, savedRole, ',');
            getline(ss, savedCars); // Read the rest of the line, containing the cars
            if (savedUsername == username) {
                for (const auto& rental : carRentals) {
                    const string& carID = rental.first;
                    const string& rentingDate = rental.second.first;
                    const string& condition = rental.second.second.first;
                    const string& price = rental.second.second.second;
                    if (savedCars.find(carID) != string::npos) {
                        cout << "Car " << carID << " already rented by the user" << endl;
                    } else {
                        // Apply discount for employees
                        double discountedPrice = stod(price) * 0.85; // 15% discount
                        savedCars += carID + ";" + rentingDate + ";" + condition + ";" + to_string(discountedPrice) + ",";
                    }
                }
                user = savedUsername + "," + savedPassword + "," + savedRole + "," + savedCars;
                rewriteUserDataFile();
                return true;
            }
        }
        cout << "User not found!" << endl;
        return false;
    }
};

class Greeting {
public:
    void greet() {
        std::cout << "Good morning! Have a wonderful day ahead" << std::endl;
    }
};


class Wish {
public:
    void wishDay() {
        std::cout << "Have a wonderful day" << std::endl;
    }
};

class Car {
    private:
        string filename;
        unordered_set<string> carIds; // To store existing car IDs
        vector<string> carData;       // To store car data temporarily

    public:
        Car(const string &filename) : filename(filename) {
            // Load existing car IDs from the file
            loadCarIds();
        }

        bool addCar(const string &id, const string &make, const string &model, int year , const string &status, float score , const string &username) {
            if (carIds.find(id) != carIds.end()) {
                cout << "Car with ID '" << id << "' already exists" << endl;
                return false;
            }

            ofstream file(filename, ios_base::app);
            if (!file.is_open()) {
                cerr << "Error opening file!" << endl;
                return false;
            }

            file << id << "," << make << "," << model << "," << year << "," << status << "," << score << "," << username << "\n";
            file.close();

            cout << "Car added successfully! ID: " << id << endl;

            // Update data structures
            carIds.insert(id);
            carData.push_back(formatCarData(id, make, model, year, status, score, username));
            rewriteCarDataFile();
            loadCarIds();
            return true;
        }

        bool deleteCar(const string &id) {
            auto it = find_if(carData.begin(), carData.end(), [id](const string &car) {
                stringstream ss(car);
                string savedId;
                getline(ss, savedId, ',');
                return savedId == id;
            });

            if (it != carData.end()) {
                size_t index = it - carData.begin();
                carData.erase(it);
                carIds.erase(id);
                rewriteCarDataFile();
                cout << "Car with ID '" << id << "' deleted successfully" << endl;
                loadCarIds();
                return true;
            } else {
                cout << "Car with ID '" << id << "' not found!" << endl;
                return false;
            }
        }

        bool updateCar(const string &id, const string &make, const string &model, int year , const string &status, float score , const string &username) {
            auto it = find_if(carData.begin(), carData.end(), [id](const string &car) {
                stringstream ss(car);
                string savedId;
                getline(ss, savedId, ',');
                return savedId == id;
            });

            if (it != carData.end()) {
                size_t index = it - carData.begin();
                carData[index] = formatCarData(id, make, model, year, status, score, username);
                rewriteCarDataFile();
                cout << "Car with ID '" << id << "' updated successfully!" << endl;
                loadCarIds();
                return true;
            } else {
                cout << "Car with ID '" << id << "' not found!" << endl;
                return false;
            }
            
        }

        void showAllCars() {
            cout << "All Cars:\n";
            loadCarIds();
            for (const string &car : carData) {
                cout << car << endl;
            }
        }

        void showAvailableCars() {
            cout << "Available Cars:\n";
            loadCarIds();
            for (const string &car : carData) {
                stringstream ss(car);
                string id, make, model, status,year;
                getline(ss, id, ',');
                getline(ss, make, ',');
                getline(ss, model, ',');
                getline(ss, year, ',');
                getline(ss, status, ',');
                if (status == "Free") {
                    cout << "Id: " << id << " Make: "<<make<<" Model: "<<model<<" Year: "<< year<<endl;
                }
            }
        }

        bool changeCarStatus(const string &id, const string &newStatus) {
            auto it = find_if(carData.begin(), carData.end(), [id](const string &car) {
                stringstream ss(car);
                string savedId;
                getline(ss, savedId, ',');
                return savedId == id;
            });

            if (it != carData.end()) {
                size_t index = it - carData.begin();
                stringstream ss(carData[index]);
                string carId, make, model, status, username;
                int year;
                float score;
                getline(ss, carId, ',');
                getline(ss, make, ',');
                getline(ss, model, ',');
                ss >> year;
                ss.ignore(); // Ignore the comma
                getline(ss, status, ',');
                ss >> score;
                ss.ignore(); // Ignore the comma
                getline(ss, username, ',');
                carData[index] = formatCarData(carId, make, model, year, newStatus, score, username);
                rewriteCarDataFile();
                cout << "Status of car with ID '" << id << "' changed successfully to '" << newStatus << "'!" << endl;
                return true;
            } else {
                cout << "Car with ID '" << id << "' not found" << endl;
                return false;
            }
        }

        string checkCarStatus(const string &id) {
            auto it = find_if(carData.begin(), carData.end(), [id](const string &car) {
                stringstream ss(car);
                string savedId, make, model, status;
                int year;
                float score;
                getline(ss, savedId, ',');
                getline(ss, make, ',');
                getline(ss, model, ',');
                ss >> year;
                ss.ignore(); // Ignore the comma
                getline(ss, status, ',');
                ss >> score;
                return savedId == id;
            });

            if (it != carData.end()) {
                stringstream ss(*it);
                string carId, make, model, status;
                int year;
                float score;
                getline(ss, carId, ',');
                getline(ss, make, ',');
                getline(ss, model, ',');
                ss >> year;
                ss.ignore(); // Ignore the comma
                getline(ss, status, ',');
                ss >> score;
                return status;
            } else {
                return "Car with ID '" + id + "' not found";
            }
        }
        bool viewCarById(const string &id) {
            auto it = find_if(carData.begin(), carData.end(), [id](const string &car){
                stringstream ss(car);
                string savedId;
                getline(ss, savedId, ',');
                return savedId == id;
            });

            if (it != carData.end()) {
                cout << "Car details for ID '" << id << "':\n";
                cout << *it << endl;
                return true;
                
            } else {
                cout << "Car with ID '" << id << "' not found" << endl;
                return false;
            }
        }

    private:
        void loadCarIds() {
            ifstream file(filename);
            if (!file.is_open()) {
                cerr << "Error opening file" << endl;
                return;
            }
            carData.clear(); // Clear previous carData
            carIds.clear(); 

            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string savedId;
                getline(ss, savedId, ',');
                carIds.insert(savedId);
                carData.push_back(line);
            }
            file.close();
        }

        void rewriteCarDataFile() {
            ofstream file(filename);
            if (!file.is_open()) {
                cerr << "Error opening file" << endl;
                return;
            }
            for (const string &car : carData) {
                file << car << "\n";
            }
            file.close();
        }

        string formatCarData(const string &id, const string &make, const string &model, int year , const string &status, float score , const string &username) {
            stringstream ss;
            ss << id << "," << make << "," << model << "," << year << "," << status << "," << score << "," << username;
            return ss.str();
        }
};

class Manager : public Customer, public Car {
    public:
        Manager(string customerFilename, string carFileName) : Customer(customerFilename), Car(carFileName){}

        bool addEmployee(const string& username, const string& password) {
            return registerUser(username, password, "employee");
        }

        bool deleteEmployee(const string& username) {
            return deleteUser(username);
        }

        bool updateEmployeePassword(const string& username, const string& newPassword) {
            return updatePassword(username, newPassword);
        }

        bool addCarData(const string &id, const string &make, const string &model, int year , const string &status, float score , const string &username) {
            return addCar(id,make,model,year,status,score,username);
        }

        bool deleteCarData(const string& id) {
            return deleteCar(id);
        }

        bool updateCarData(const string &id, const string &make, const string &model, int year , const string &status, float score , const string &username) {
            return updateCar(id,make,model,year,status,score,username);
        }

        bool checkCar(string &carid){
            // return true;
            return viewCarById(carid);
        }

    private:
        string employeeFile;
        string carFileName;
        string customerFilename; // File path for the employee database
};


Customer customer(""); // Declare Customer globally
Car carSystem("");
Employee employee("");
Manager manager("","");

void initCustomer() {
    customer = Customer("./Database/Customers.csv");
    carSystem = Car("./Database/Cars.csv");
    employee = Employee("./Database/Employees.csv") ;
    manager = Manager("./Database/Employees.csv","./Database/Cars.csv");

}

void customerInput() {
    while(true) {
        cout << "Input 1 if you want to Login" << endl;
        cout << "Input 2 if you want to Logout" << endl;
        cout << "Input 3 if you want to Register" << endl;
        cout << "Input 4 if you want to Exit" << endl;
        
        string step;
        cout<<"Choice: ";
        cin >> step;

        if(step == "1") {
            cout << "User Name: ";
            string userid;
            cin >> userid;
            cout << "Password: ";
            string userpassword;
            cin >> userpassword;
            
            if(customer.loginUser(userid, userpassword)) {
                cout << "Logged in successfully" << endl;
                // Allow other customer features after successful login
                while(true) {
                    cout << "Input 1 to rent a car" << endl;
                    cout << "Input 2 to update your password" << endl;
                    cout << "Input 3 to return a rented car"<<endl; 
                    cout << "Input 4 to see all available cars"<<endl;
                    cout << "Input 5 to view your rented car history" <<endl;
                    cout << "Input 6 to pay fine of your rented Car" << endl;
                    cout << "Input 7 to logout" << endl;
                    
                    string choice;
                    cout<<"Choice: ";
                    cin >> choice;

                    if(choice == "1") {
                        string caridstatus;
                        cout<< "Input Car id: ";
                        cin>>caridstatus;
                        if (carSystem.checkCarStatus(caridstatus)=="Free"){
                            carSystem.changeCarStatus(caridstatus,"Occupied");
                            vector<pair<string, pair<string, pair<string, string>>>> carRentals;
                            carRentals.push_back({caridstatus, {Tools::getTodayDate(), {"Good", "100"}}});
                            customer.rentCars(userid,carRentals);
                        }else {
                            cout<<"Car you seek is "<<carSystem.checkCarStatus(caridstatus)<<endl;
                        }
                    } else if(choice == "2") {
                        string newpassword;
                        cout<<"New Password: ";
                        cin>>newpassword;
                        customer.updatePassword(userid, newpassword);
                    } else if(choice == "3") {
                        string cariduser;
                        cout<<"Input Car Id: ";
                        cin>>cariduser;
                        carSystem.changeCarStatus(cariduser,"Free");
                        customer.removeRentedCar(userid, cariduser);
                    }else if(choice == "4") {
                        carSystem.showAvailableCars();
                    } else if(choice == "5") {
                        customer.showRentedCars(userid);
                    } else if(choice == "6") {
                        string carid;
                        cout<<"Rented Car Id: ";
                        cin>>carid;
                        if(customer.getRentingDate(userid,carid)== "Car not found"){
                            cout<<customer.getRentingDate(userid,carid)<<endl;
                        }else if(customer.getRentingDate(userid,carid)== "User not found"){
                            cout<<customer.getRentingDate(userid,carid)<<endl;
                        }else{
                            cout<< "Current fine is: "<<customer.calculateFine(customer.getRentingDate(userid,carid))<<endl;
                        }
                    } else if(choice == "7") {
                        cout << "Logging out" << endl;
                        break; 
                    } else {
                        cout << "Invalid choice" << endl;
                        continue;
                    }
                }
            } else {
                cout << "Login failed Incorrect username or password." << endl;
                continue;
            }
        } else if(step == "2") {
            cout << "Logging out" << endl;
            break;
        } else if(step == "3") {
            cout << "User Name: ";
            string userid;
            cin >> userid;
            cout << "Password: ";
            string userpassword;
            cin >> userpassword;
            customer.registerUser(userid, userpassword, "customer");
        } else if(step == "4") {
            cout << "Exiting" << endl;
            break;
        } else {
            cout << "Invalid choice" << endl;
        }
    }
}


void employeeInput(){
    while(true) {
        cout << "Input 1 if you want to Login" << endl;
        cout << "Input 2 if you want to Logout" << endl;
        cout << "Input 3 if you want to Exit" << endl;
        
        string step;
        cout<< "Input: ";
        cin >> step;

        if(step == "1") {
            cout << "Emploee Name: ";
            string employeeid;
            cin >> employeeid;
            cout << "Password: ";
            string userpassword;
            cin >> userpassword;
            
            if(employee.loginUser(employeeid, userpassword)) {
                cout << "Logged in successfully" << endl;
                // Allow other customer features after successful login
                while(true) {
                    cout << "Input 1 to rent a car" << endl;
                    cout << "Input 2 to update your password" << endl;
                    cout << "Input 3 to return a rented car"<<endl; 
                    cout << "Input 4 to see available cars"<<endl;
                    cout << "Input 5 to view your rented car history" <<endl;
                    cout << "Input 6 to view fine on rented car" <<endl; 
                    cout << "Input 7 to logout" << endl;
                    
                    string choice;
                    cout<<"Choice: ";
                    cin >> choice;

                    if(choice == "1") {
                        string caridstatus;
                        cout<< "Input Car id: ";
                        cin>>caridstatus;
                        if (carSystem.checkCarStatus(caridstatus)=="Free"){
                            carSystem.changeCarStatus(caridstatus,"Occupied");
                            vector<pair<string, pair<string, pair<string, string>>>> carRentals;
                            carRentals.push_back({caridstatus, {Tools::getTodayDate(), {"Good", "100"}}});
                            employee.rentCars(employeeid,carRentals);
                        }else {
                            cout<<"Car you seek is "<<carSystem.checkCarStatus(caridstatus)<<endl;
                        }
                    } else if(choice == "2") {
                        string newpassword;
                        cout<<"New Password: ";
                        cin>>newpassword;
                        employee.updatePassword(employeeid, newpassword);
                    } else if(choice == "3") {
                        string cariduser;
                        cout<<"Input Car Id: ";
                        cin>>cariduser;
                        carSystem.changeCarStatus(cariduser,"Free");
                        employee.removeRentedCar(employeeid, cariduser);
                    }else if(choice == "4") {
                        carSystem.showAvailableCars();
                    } else if(choice == "5") {
                        employee.showRentedCars(employeeid);
                    } else if(choice == "6") {
                        string carid;
                        cout<<"Rented Car Id: ";
                        cin>>carid;
                        if(customer.getRentingDate(employeeid,carid)== "Car not found"){
                            cout<<customer.getRentingDate(employeeid,carid)<<endl;
                        }else if(customer.getRentingDate(employeeid,carid)== "User not found"){
                            cout<<customer.getRentingDate(employeeid,carid)<<endl;
                        }else{
                            cout<< "Current fine is: "<<customer.calculateFine(customer.getRentingDate(employeeid,carid))<<endl;
                        }
                    } else if(choice == "7") {
                        cout << "Logging out" << endl;
                        break; 
                    } else {
                        cout << "Invalid choice" << endl;
                        continue;
                    }
                }
            } else {
                cout << "Login failed! Incorrect username or password." << endl;
            }
        } else if(step == "2") {
            cout << "Logging out" << endl;
        } else if(step == "3") {
            cout << "Exiting" << endl;
            break;
        } else {
            cout << "Invalid choice" << endl;
        }
    }
}

void managerInput(){
    while(true) {
        cout << "Input 1 if you want to Login" << endl;
        cout << "Input 4 if you want to Exit" << endl;

        string step;
        cout<<"Choice :";
        cin>>step;

        if (step == "1"){
            cout << "Manager Name: ";
            string userid;
            cin >> userid;
            cout << "Password: ";
            string userpassword;
            cin >> userpassword;
            if(userid == "admin" && userpassword == "admin"){
                while (true){
                    cout << "Input 1 to register a new Customer" << endl;
                    cout << "Input 2 to register a new Employee" << endl;
                    cout << "Input 3 to delete a Customer" << endl;
                    cout << "Input 4 to delete an Employee" << endl; 
                    cout << "Input 5 to insert a Car"<<endl; 
                    cout << "Input 6 to see all Cars in the database"<<endl;
                    cout << "Input 7 to remove a Car" <<endl;
                    cout << "Input 8 to update a Car's Data" <<endl;
                    cout << "Input 9 to logout of this window" << endl;    

                    string choice;
                    cout<<"Choice: ";
                    cin >> choice;

                    if(choice == "1"){
                        cout << "User Name: ";
                        string userid;
                        cin >> userid;
                        cout << "Password: ";
                        string userpassword;
                        cin >> userpassword;
                        customer.registerUser(userid, userpassword, "customer");
                    } else if(choice == "2"){
                        cout << "Employee Name: ";
                        string empid;
                        cin >> empid;
                        cout << "Password: ";
                        string emppassword;
                        cin >> emppassword;
                        manager.addEmployee(empid,emppassword);
                    } else if(choice == "3"){
                        cout << "User Name: ";
                        string userid;
                        cin >> userid;
                        customer.deleteUser(userid);
                    }else if(choice == "4"){
                        cout << "Employee Name: ";
                        string empid;
                        cin >> empid;
                        manager.deleteEmployee(empid);
                    }else if(choice == "5"){
                        string carName,companyName;
                        int carYear,carRent;
                        cout<<"Car Name: ";
                        cin>>carName;
                        cout<<"Company Name: ";
                        cin>>companyName;
                        cout<<"Car Year: ";
                        cin>>carYear;
                        cout<<"Car Rent: ";
                        cin>>carRent;
                        manager.addCarData("C"+Tools::generateRandomID(), carName, companyName, carYear, "Free", carRent, "None");
                    } else if(choice == "6"){
                        carSystem.showAllCars();
                    } else if(choice == "7"){
                        string carid;
                        cout<<"Carid: ";
                        cin>>carid;
                        manager.deleteCarData(carid);
                    } else if(choice == "8"){
                        string carid;
                        cout<<"Car id: "<<endl;
                        cin>>carid;
                        if(manager.checkCar(carid)){
                            string carName,companyName;
                            int carYear,carRent;
                            cout<<"Car Name: ";
                            cin>>carName;
                            cout<<"Company Name: ";
                            cin>>companyName;
                            cout<<"Car Year: ";
                            cin>>carYear;
                            cout<<"Car Rent: ";
                            cin>>carRent;
                            manager.updateCarData(carid,carName,companyName,carYear,"Free",carRent,"None");
                        }
                    } else if(choice == "9"){
                        cout << "Logging out" << endl;
                        break;
                    } else {
                        cout << "Invalid choice" << endl;
                        continue;
                    }

                }
            }
        }  else if(step == "4") {
            cout << "Exiting" << endl;
            break;
        } else {
            cout << "Invalid choice" << endl;
            continue;
        }
    }
}

int main() {
    // Initializing the user system
    initCustomer();

    cout<<"Choose the option"<<endl;
    while(true){
        cout<<"Input 1 if you are Customer"<<endl;
        cout<<"Input 2 if you are Employee"<<endl;
        cout<<"Input 3 if you are Manager"<<endl;
        cout<<"Input 4 if you want to exit"<<endl;
        string step;
        cout<<"Choice: ";
        cin>>step;
        // move to different function based on the choice made by the user
        if(step == "1"){
            customerInput();
        }else if(step == "2"){
            employeeInput();
        }else if(step == "3"){
            managerInput();
        }else if(step == "4"){
            cout<<"Exiting ....";
            break;
        }else{
            continue;
        }
    }

    return 0;
}