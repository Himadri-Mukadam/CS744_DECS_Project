// #include "../include/httplib.h"
// #include <iostream>
// #include <string>


// frontend_server.cpp
#include "../include/httplib.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include "../include/nlohmann/json.hpp"


using namespace std;
using json = nlohmann::json;


int main() {
    httplib::Server svr;
   


    svr.Post("/fill", [](const httplib::Request &req, httplib::Response &res) {
        string data_raw = req.body;
        cout<<"Request sssss is: " << data_raw <<endl;

        // cout << "Raw request (hex): ";
        // for (unsigned char c : req.body) printf("%02x ", c);
        // cout << endl;

        json data_json = json::parse(req.body);

        string name = data_json["name"].get<std::string>();
        string company = data_json["company"].get<std::string>();
        int rating = std::stoi(data_json["rating"].get<std::string>());  //first get string version of rating from json, and then convert this string to integer using stoi()
        string comments = data_json["comments"].get<std::string>();
        
        cout << "Name is: " << name << endl;
        cout << "Company is: " << company << endl;
        cout << "Rating is: " << rating << endl;

         if(company.empty() || name.empty()){   //name and company are compulsory fields
            res.status = 500;
            res.set_content("Invalid parameters! Must give customer-name and company-name","text/plain");
         }        
        else    //client gave correct request
        {
            json json_data = {
                {"name", name},
                {"company", company},
                {"rating", data_json["rating"]},
                {"comments", comments}
            };
            
            //made connection to db_server inside a function, so that connection is made only when needed to do some DB operations
            //@Doubt: ask ma'am or TAs, that is this the correct way? But first do multi-threading and then ask
            string json_str = json_data.dump(); //convert json_data to json string, using dump()
            httplib::Client db_client("localhost", 8000);   //connect to db_server as an http client
            auto res_db = db_client.Post("/insert", json_str, "application/json");  //send POST request to db_server

            if(res_db && res_db -> status == 200){
                res.status = 200;
                res.set_content("Inserted your feedback", "text/plain");
            }
            else{
                res.status = 500;
                res.set_content("Error occured while inserting your feedback", "text/plain");
            }
        }
        
    });


    svr.Get("/view_usecompany", [](const httplib::Request &req, httplib::Response &res) {
        string data = req.body;
        cout<<"Request is: " << data <<endl;


        string param_company = req.get_param_value("company");
        if(param_company.empty())
            res.set_content("Invalid parameter! Provide company-name","text/plain");
        else
        {
            cout << "Company is: " << param_company << endl;
            std::string url = "/select_usecompany?company=" + param_company;
    
            httplib::Client db_client("localhost",8000);
            auto res_db = db_client.Get(url);
            
            if(res_db && res_db -> status == 200){
                res.status = 200;
                //res_db->body;
                cout<<"------------Got response from Database!-----------------"<<endl;
                json response_from_db = json::parse(res_db->body);

                cout << response_from_db;

                for(auto data : response_from_db){
                    cout << "Name: "<< data["name"] <<endl;
                    cout << "Company: "<< data["company"] <<endl;
                    cout << "Rating: "<< data["rating"] <<endl;
                    cout << "Comments: "<< data["comments"] <<endl;
                    cout << "---------------------------------------------" << endl;
                }

                string response_to_client = response_from_db.dump();
                res.set_content(response_to_client, "text/plain");
            }
            else{
                res.status = 500;
                res.set_content("Error occured while inserting your feedback", "text/plain");
            }
        }
                
    });

    svr.Get("/view_usename", [](const httplib::Request &req, httplib::Response &res) {
        string data = req.body;
        cout<<"Request is: " << data <<endl;

        string param_name = req.get_param_value("name");
        if(param_name.empty())
            res.set_content("Invalid parameter!","text/plain");
        else
        {
            cout << "Name is: " << param_name << endl;

            std::string url = "/select_usename?name=" + param_name;
    
            httplib::Client db_client("localhost",8000);
            auto res_db = db_client.Get(url);
            
            if(res_db && res_db -> status == 200){
                res.status = 200;
                //res_db->body;
                cout<<"------------Got response from Database!-----------------"<<endl;
                json response_from_db = json::parse(res_db->body);

                cout << response_from_db;

                for(auto data : response_from_db){
                    cout << "Name: "<< data["name"] <<endl;
                    cout << "Company: "<< data["company"] <<endl;
                    cout << "Rating: "<< data["rating"] <<endl;
                    cout << "Comments: "<< data["comments"] <<endl;
                    cout << "---------------------------------------------" << endl;
                }

                string response_to_client = response_from_db.dump();
                res.set_content(response_to_client, "text/plain");
            }
            else{
                res.status = 500;
                res.set_content("Error occured while inserting your feedback", "text/plain");
            }
        }
        
    });

    svr.Delete("/delete_feedback", [](const httplib::Request &req, httplib::Response &res) {
        string data = req.body;
        cout<<"Request is: " << data <<endl;

        string param_name = req.get_param_value("name");
        string param_company = req.get_param_value("company");
        if(param_name.empty() || param_company.empty())
            res.set_content("Invalid parameter!","text/plain");
        else
        {
            cout << "Name is: " << param_name << endl;
            cout << "Company is: " << param_company << endl;

            std::string url = "/delete_row?name=" + param_name + "&company=" + param_company;
    
            httplib::Client db_client("localhost",8000);
            auto res_db = db_client.Delete(url);
            
            if(res_db && res_db -> status == 200){
                res.status = 200;
                cout<<"------------Got response from Database!-----------------"<<endl;
                res.set_content(res_db->body, "text/plain");
            }
            else{
                res.status = 500;
                res.set_content("Error occured while inserting your feedback", "text/plain");
            }
        }
        
    });

    cout << "Server running on http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}
