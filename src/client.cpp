#include "../include/httplib.h"
#include <iostream>
#include <string>
#include "../include/nlohmann/json.hpp"


using namespace std;
using json = nlohmann::json;

int main(){

    httplib::Client http_client("localhost", 8080);   //connect to db_server as an http client

    int choice;
    string name;
    string company;
    string rating;
    string comment;
    int temp;


    cout << "1. Fill a feedback\n2. Read feedback (filter: User)\n3. Read feedback (filter: Company)\n4. Delete feedback\n";
    

    while(1){

        name.clear();
        company.clear();
        rating.clear();
        comment.clear();

        cout << "Enter your choice number: " << endl;
        cin >> choice;
        if(choice == 1){
                cout << "Enter name: ";
                cin.ignore();
                getline(cin, name);

                cout << endl << "Enter company: ";
                cin >> company;

                cout << endl <<"Enter rating (number between 1 - 5): ";
                cin >> rating;

                cin.ignore();
                try{
                    temp = stoi(rating);
                }
                catch(const invalid_argument &e){
                    cerr << "Not a number" << endl;
                    break;
                }

                cout << "Enter comments: ";
                getline(cin, comment);

                json feedback ={
                    {"name", name},
                    {"company", company},
                    {"rating", rating},
                    {"comments", comment}
                };

                string json_str = feedback.dump();

                auto resp = http_client.Post("/fill", json_str, "application/json");
                if(resp && resp->status == 200){
                    cout << "Response: " << resp->body << endl;
                }
                else{
                    cout << "Data not inserted! " << endl;
                }
            }
    
        else if(choice == 2){
            cout << "Enter name: ";
            cin.ignore();
            getline(cin, name);

            string url = "/view_usename?name=" + name;
            auto resp = http_client.Get(url);

            if(resp && resp->status){
                //cout << "Response: " << resp->body << endl;

                json response = json::parse(resp->body);

                //cout << response << endl;

                for(auto data : response){
                    cout << "---------------------------------------------" << endl;
                    cout << "Name: "<< data["name"] <<endl;
                    cout << "Company: "<< data["company"] <<endl;
                    cout << "Rating: "<< data["rating"] <<endl;
                    cout << "Comments: "<< data["comments"] <<endl;
                    
                }
            }
            else{
                cout << "Error occured. Couldn't get the data" << endl;
            }

        }
    
        else if(choice == 3){
            cout << "Enter company: ";
            cin.ignore();
            getline(cin, company);

            string url = "/view_usecompany?company=" + company;
            auto resp = http_client.Get(url);

            if(resp && resp->status == 200){
                //cout << "Response: " << resp->body << endl;

                json response = json::parse(resp->body);

                //cout << response << endl;

                for(auto data : response){
                    cout << "---------------------------------------------" << endl;
                    cout << "Name: "<< data["name"] <<endl;
                    cout << "Company: "<< data["company"] <<endl;
                    cout << "Rating: "<< data["rating"] <<endl;
                    cout << "Comments: "<< data["comments"] <<endl;
                    
                }
            }
            else{
                cout << "Error occured. Couldn't get the data" << endl;
            }
        }

        else if(choice == 4){
            cout << "Enter name: ";
            cin.ignore();
            getline(cin, name);

            cout << "Enter company: ";
            //cin.ignore();
            getline(cin, company);

            cout << "Name: " << name <<"\t Company: " << company << endl;
            if(name.empty() || company.empty()){
                cout << "Invalid input of name or company!" << endl;
                break;
            }

            string url = "/delete_feedback?name=" + name + "&company=" + company;
            auto resp = http_client.Delete(url);

            if(resp && resp->status == 200){
                cout << "Response: " << resp->body << endl;
            }
            else{
                cout << "Error deleting your data!" << endl;
            }

        }
    }
   
}