#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <mysqlx/xdevapi.h>   // MySQL Connector/C++ X DevAPI header
#include "../include/httplib.h"
#include "../include/nlohmann/json.hpp"
#include <unordered_map>
#include <chrono>
#include <limits>

using namespace std;
using namespace mysqlx;
using json = nlohmann::json;
using namespace std::chrono;



class TimeBasedLRUCache {
private:
    struct Entry {
        std::string value;
        time_point<steady_clock> last_access;
    };

    unordered_map<std::string, Entry> cache;
    int capacity;

public:
    TimeBasedLRUCache(int cap) : capacity(cap) {}

    bool exists(const std::string &key) {
        auto it = cache.find(key);
        if (it == cache.end())
            return false;
        it->second.last_access = steady_clock::now();  // update recency on access
        return true;
    }

    std::string get(const std::string &key) {
        if (!exists(key)) return "";
        else{
            cout << "Came to cache and found the entry for: "<< key << endl;
            return cache[key].value;
        }
        
    }

    void put(const std::string &key, const std::string &value) {
        //auto now = steady_clock::now();

        // Update existing key
        if (cache.find(key) != cache.end()) {
            cache[key].value = value;
            cache[key].last_access = steady_clock::now();
            cout << "Updated in cache" << endl;
            return;
        }

        // If full, evict least recently used (oldest timestamp)
        if ((int)cache.size() >= capacity) {
            std::string oldest_key;
            auto oldest_time = time_point<steady_clock>::max();
            for (auto &p : cache) { //finding max
                if (p.second.last_access < oldest_time) {
                    oldest_time = p.second.last_access;
                    oldest_key = p.first;
                }
            }
            cout << "Evicted (least recently used): " << oldest_key << endl;
            cache.erase(oldest_key);
        }

        // Insert new entry
        cache[key] = {value, steady_clock::now()};
        cout << "Placed in cache" << endl;

        cout << "===============Cache scenario: ====================" << endl;
        print_cache();
    }

    void delete_entry(const std::string &key){
        cache.erase(key);
        cout << "Deleted entry from cache" << endl;

        cout << "===============Cache scenario: ====================" << endl;
        print_cache();
    }

    void print_cache(){
        if(cache.empty()){
            cout << "Cache is empty!" << endl;
            return;
        }
        else{
            for(auto p: cache){
                auto ms = duration_cast<milliseconds>(p.second.last_access.time_since_epoch()).count();
                cout << "Key: " << p.first << endl;
                cout << "Value: " << p.second.value << endl;
                cout << "Time-stamp: " << ms << endl;
            }
        }
    }

};



int main() {

    int choice, rating;
    char name[50], company[50];
    std::string comments;

    TimeBasedLRUCache Cache = TimeBasedLRUCache(5);

    httplib::Server svr;

    //------------------connect to DB -----------------------------------
    Session sess("localhost", 33060, "rani", "rani");
    Schema db = sess.getSchema("feedback_db");  //database
    Table feedback = db.getTable("feedback_form");  //table

    //----------------Start getting requests from http-server-------------------

    svr.Post("/insert", [&](const httplib::Request &req, httplib::Response &res) {
        try{
            json data = json::parse(req.body);

            std::string name = data["name"];
            std::string company = data["company"];
            int rating = std::stoi(data["rating"].get<std::string>());  //first get string version of rating from json, and then convert this string to integer using stoi()
            std::string comments = data["comments"];

            feedback.insert("user_name", "company_name", "rating", "comments")
                    .values(name, company, rating, comments)
                    .execute();
            res.status = 200;
            res.set_content("{\"status\":\"success\"}", "application/json");

            if(Cache.exists(name)){
                Cache.delete_entry(name);
            }
            if(Cache.exists(company)){
                Cache.delete_entry(company);
            }

        } catch (std::exception &e) {
            printf("Error: %s\n", e.what());
            json err = {{"error", e.what()}};
            res.status = 500;
            res.set_content(err.dump(), "application/json");
        }
        

    });
        
    svr.Get("/select_usecompany", [&](const httplib::Request &req, httplib::Response &res) {
        cout<<"Came to select_usecompany"<<endl;
        
            std::string company = req.get_param_value("company");

            //Find in the cache first
            if(Cache.exists(company)){
                res.status = 200;
                res.set_content(Cache.get(company), "application/json");
            }

            else{   //Else go search in the database
                json result_array = json::array();
                try{
                    RowResult rows = feedback
                    .select("user_name", "company_name", "rating", "comments", "submitted_at")
                    .where("company_name = :c")     
                    .bind("c", company)             
                    .execute();

                    cout<<"executed line after select"<< endl;
                    for(Row row: rows){ //get each value from one row, and push it in json array
                        cout << row[0].get<std::string>() << endl;
                        cout << row[1].get<std::string>() << endl;
                        cout << std::to_string(row[2].get<int>()) << endl;
                        cout << row[3].get<std::string>() << endl;

                        json r;
                        r["name"] = row[0].get<std::string>();
                        r["company"] = row[1].get<std::string>();
                        r["rating"] = std::to_string(row[2].get<int>());    //converted rating to string
                        r["comments"] = row[3].get<std::string>();

                        result_array.push_back(r);
                    }

                    res.status = 200;
                    res.set_content(result_array.dump(), "application/json");

                    //Populate the Cache
                    Cache.put(company, result_array.dump());

                } catch (std::exception &e) {

                    cout<<"Error: "<<e.what()<<endl;
                    json err = {{"error", e.what()}};
                    res.status = 500;
                    res.set_content(err.dump(), "application/json");
                }
        }
            
    });

     svr.Get("/select_usename", [&](const httplib::Request &req, httplib::Response &res) {
        cout<<"Came to select_usename"<<endl;
        
            
            std::string name = req.get_param_value("name");

            if(Cache.exists(name)){
                res.status = 200;
                res.set_content(Cache.get(name), "application/json");
            }
            
            else{
                try{
                    json result_array = json::array();
                    RowResult rows = feedback
                    .select("user_name", "company_name", "rating", "comments", "submitted_at")
                    .where("user_name = :u")     
                    .bind("u", name)            
                    .execute();

                    cout<<"executed line after select"<< endl;

                    for(Row row: rows){ //get each value from one row, and push it in json array
                        cout << row[0].get<std::string>() << endl;
                        cout << row[1].get<std::string>() << endl;
                        cout << std::to_string(row[2].get<int>()) << endl;
                        cout << row[3].get<std::string>() << endl;

                        json r;
                        r["name"] = row[0].get<std::string>();
                        r["company"] = row[1].get<std::string>();
                        r["rating"] = std::to_string(row[2].get<int>());    //converted rating to string
                        r["comments"] = row[3].get<std::string>();

                        result_array.push_back(r);
                    }

                    res.status = 200;
                    res.set_content(result_array.dump(), "application/json");

                    //Populate the Cache
                    Cache.put(name, result_array.dump());
            
                } catch (std::exception &e) {
                    cout<<"Error: "<<e.what()<<endl;
                    json err = {{"error", e.what()}};
                    res.status = 500;
                    res.set_content(err.dump(), "application/json");
                }
            }
            
        

    });

     svr.Delete("/delete_row", [&](const httplib::Request &req, httplib::Response &res) {
        
        std::string name = req.get_param_value("name");
        std::string company = req.get_param_value("company");

        if(name.empty() || company.empty()){
            res.set_content("Invalid parameter!","text/plain");
            res.status = 500;
        }      
        else
        {
            cout << "Name is: " << name << endl;
            cout << "Company is: " << company << endl;

            mysqlx::Result result = feedback
            .remove()
            .where("user_name = :u AND company_name = :c")
            .bind("u", name)
            .bind("c", company)
            .execute();

            cout<<"No. of rows deleted : " << result.getAffectedItemsCount() << endl;

            if(result.getAffectedItemsCount() > 0){

                std::string resp = "Deleted successfully! Deleted " + to_string(result.getAffectedItemsCount()) + " rows";
                res.set_content(resp,"text/plain");

            }
            else{
               res.set_content("No data found!","text/plain");
            }

            res.status = 200;

            //Delete from cache if present
            if(Cache.exists(name)){
                Cache.delete_entry(name);
            }

            if(Cache.exists(company)){
                Cache.delete_entry(company);
            }
        }
        
        
    });

    cout << "Server running on http://localhost:8000\n";
    svr.listen("0.0.0.0", 8000);
    return 0;
}