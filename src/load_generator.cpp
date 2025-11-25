#include "../include/httplib.h"
#include <iostream>
#include <string>
#include "../include/nlohmann/json.hpp"
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <random>
#include <cstdlib>


using namespace std;
using json = nlohmann::json;

//these atomic variables will make sure of atomicity of operations on declared variables.
atomic<long> total_requests_completed(0);
atomic<long> total_requests_made(0);
atomic<long> total_latency_ns(0);
atomic<bool> stop_flag(false);

thread_local mt19937 rng(random_device{}());

string rand_from(const vector<string>& v) {
    uniform_int_distribution<> d(0, v.size() - 1);
    return v[d(rng)];
}

vector<string> user_names = {
    "Vivaan", "Aditya", "Krishna",
    "Rohan", "Siddharth", "Kabir", "Ishaan", "Dhruv",
    "Aryan", "Kunal", "Yash", "Nikhil", "Samar",
    "Rudra", "Viraj", "Harsh", "Pranav", "Rishi",
    "Atharv", "Manav", "Shaurya", "Kartik",
    "Param", "Aniket", "Sahil", "Tejas", "Tanish",
    "Ayaan", "Keshav", "Sameer", "Ranveer", "Reyansh",
    "Hardik", "Mayank", "Ujjwal", "Abhinav", "Ritvik",
    "Ananya",
    "Priya", "Kavya", "Sneha", "Aditi",
    "Divya", "Nikita", "Rhea", "Tanvi", "Simran",
    "Pari", "Pooja", "Radhika", "Shruti", "Anika",
    "Dia", "Tara", "Kiara", "Advika", "Aisha",
    "Vaishnavi", "Ira", "Anvi", "Khushi", "Mahika",
    "Hritika", "Prisha", "Suhani", "Aayra",
    "Ritika", "Bhavya", "Jhanvi", "Srishti", "Muskan",
    "Aparna", "Pallavi", "Komal", "Deeksha", "Harini",
    "Navya", "Smriti", "Trisha", "Shreya"
};


vector<string> companies = {
    "Netflix", "Meta",
    "Apple", "IBM", "Intel", "Oracle",
    "Samsung", "Nvidia", "Qualcomm", "Cisco", "Uber",
    "Lyft", "Spotify", "Salesforce", "Zoom", "Slack",
    "Dropbox", "Red Hat", "PayPal", "Stripe", "Square",
    "Tesla", "SpaceX", "Twitter", "LinkedIn", "Pinterest",
    "Dell", "HP", "Lenovo", "Siemens", "SAP",
    "Wipro", "HCL", "Accenture",
    "Capgemini", "Mindtree", "Tech Mahindra", "Persistent", "L&T Infotech",
    "Philips", "Panasonic", "LG", "Schneider Electric", "Bosch",
    "Ford", "General Motors", "Volkswagen", "BMW", "Mercedes-Benz",
    "Honda", "Toyota", "Hyundai", "Tata Motors", "Mahindra",
    "Bharat Electronics", "ISRO", "NSO", "DRDO", "NTPC",
    "Adani", "Hindustan Unilever", "Asian Paints", "ITC",
    "Zomato", "Swiggy", "PhonePe", "Paytm",
    "Ola", "Byju's", "Unacademy", "Freshworks", "Zoho",
    "Atlassian", "GitHub",  "Canonical",
    "OpenAI", "DeepMind", "HuggingFace", "Snowflake", "Palantir"
};



vector<string> popular_names = {
    "Aarav", "Sanya", "Anushka", "Meera", "Arjun",
    "Rani", "Ayushi", "Ishita", "Dev", "Neha",
    "Sia", "Aarohi", "Mira", "Charvi", "Myra",
    "Aarini", "Inaaya", "Sanvi", "Lavanya", "Esha", "Himadri", "Vinayak", "Dakshi"
};

vector<string> popular_companies = {
    "Google", "Amazon", "Infosys", "TCS", "Reliance",
    "Adobe", "Microsoft", "Flipkart", "DigitalOcean", "VMware",
    "Ericsson", "Nokia", "Broadcom", "ARM", "TSMC",
    "Boeing", "Airbus", "Lockheed Martin", "General Electric", "Honeywell" , "RVMSSPL", "Marvel", "DC"  
};

string random_user_name(int w_type){

    if(w_type == 1){   
        //return user_names[rand() % user_names.size()];
        return rand_from(user_names);
    }
    else{
        //return popular_names[rand() % popular_names.size()];
        return rand_from(popular_names);
    }
    
}


string random_company_name(int w_type){

    if(w_type == 1){   
         //return companies[rand() % companies.size()];
         return rand_from(companies);
    }
    else{
        //return popular_companies[rand() % popular_companies.size()];
        return rand_from(popular_companies);
    }
    
}

void client_work(int id, int workload_type){
    cout << "Entered thread" << endl;
    int turn_var = 0;   //allows to make multiple types of request
    httplib::Client http_client("localhost", 8080);
    http_client.set_keep_alive(true);

     while(!stop_flag.load()) {

        string name = random_user_name(workload_type);
        string company = random_company_name(workload_type);
        bool ok = false;
        total_requests_made++;


        auto t1 = chrono::steady_clock::now();  //start timer to calculate response time
        turn_var = (turn_var + 1) % 2;

        if(workload_type == 1) {    // Disk-bound
            
           // if(turn_var == 1){  //when turn_var == 1, generate Create requests. When turn_var == 0, generate delete requests
                // CREATE request
               //cout << "Thread: "<< id << "\tEntered create request" << endl;
               json feedback ={
                    {"name", name},
                    {"company", company},   
                    {"rating", "4"},
                    {"comments", "During peak traffic intervals, the system attempts to maintain consistent throughput by dynamically adjusting the processing window. However, under extreme load, certain requests may experience delayed execution due to queue saturation and thread contention. This behavior is expected and ensures that the server preserves correctness and avoids state corruption. When the backlog clears, normal processing resumes automatically without requiring manual intervention. The objective of this test run is to observe how the backend behaves under artificially amplified concurrency. The load generator is configured to issue requests continuously without spacing delays, pushing the system into a regime where request arrival rates exceed the server’s sustainable capacity. The resulting metrics—especially tail latencies and queue growth—will be used to diagnose potential bottlenecks in the dispatcher, cache handler, and session management layer. I noticed that the response time varies significantly depending on the number During the latest round of testing, the objective was to evaluate how the service behaves under continuous high pressure without providing any cool-down period. The load generator was intentionally configured to maintain a steady flow of operations, ensuring that the interval between successive requests was effectively negligible. This setup simulates scenarios where clients issue commands at a rate far exceeding normal operational expectations. As the test progressed, we observed that the queue depth began increasing gradually, and although the system initially managed to keep up, certain components started exhibiting signs of strain. The thread-pool workers, responsible for processing incoming tasks, began spending more time waiting on shared locks, especially those guarding the database session handling routines.In addition to lock contention, disk-bound operations showed increased variance in completion times. Even though the underlying storage is designed to handle parallel requests efficiently, the cumulative effect of many simultaneous reads and writes introduced occasional delays. These delays, while short, were enough to propagate into the overall response time, creating sporadic latency spikes. Another contributing factor was the garbage collection behavior of certain memory-intensive operations, which occasionally paused execution for brief moments. When these pauses coincided with peak load, the service exhibited minor stlls.Despite these challenges, the test provided valuable insights into which areas require further optimization. For instance, implementing asynchronous request batching or introducing more granular locks may help reduce contention. Additionally, tuning the database connection pool to better accommodate bursts of activity could enhance throughput. While the service did not fail under stress, these findings emphasize the importance of designing systems with elasticity in mind"}                };
                    //{"comments" , "This is a demo entry made for experiment purposes." }};

                string json_str = feedback.dump();

                auto resp = http_client.Post("/fill", json_str, "application/json");
                if(resp && resp->status == 200){
                    //cout << "Thread: "<< id << "\t Response: " << resp->body << endl;
                    ok = true;
                }
                else{
                    cout << "Data not inserted! " << endl;
                    ok = false;
                }
                
          //  }

            // else{
            //     // DELETE request
            //     //cout << "Thread: "<< id << "\tEntered delete request" << endl;
            //     string url = "/delete_feedback?name=" + name + "&company=" + company;
            //     auto resp = http_client.Delete(url);

            //     if(resp && resp->status == 200){
            //         //cout << "Thread: "<< id << "\t Response: " << resp->body << endl;
            //         //cout << "Response: " << resp->body << endl;
            //         ok = true;
            //     }
            //     else{
            //         cout << "Error deleting your data!" << endl;
            //         ok = false;
            //     }
            // }
            
        }

        else  if(workload_type == 2){   //CPU-bound (in-memory cache access for popular keys)
            //declare an array of popular company names and user names, and send request from them
        
            if(turn_var == 1){  //Make request based on user-name
                string url = "/view_usename?name=" + name;
                auto resp = http_client.Get(url);
                    if(resp && resp->status){

                        if(resp->status == 404){
                            cout << "Data not found!" << endl;
                        }

                        else if(resp->status == 200){
                            json response = json::parse(resp->body);
                             for(auto data : response){
                                 cout << "---------------------------------------------" << endl;
                                 cout << "Name: "<< data["name"] <<endl;
                                 cout << "Company: "<< data["company"] <<endl;
                                 cout << "Rating: "<< data["rating"] <<endl;
                                 cout << "Comments: "<< data["comments"] <<endl;
                            
                             }
                        }
                        else{
                            cout << resp << endl;
                        } 
                        ok = true; 
                    }
                    else{
                        cout << "Error occured. Couldn't get the data" << endl;
                        ok = false;
                    }
            }
            
            else{   //Make request based on company-name
                string url = "/view_usecompany?company=" + company;
                auto resp = http_client.Get(url);
                //add timeout mechanism for Get()

                if(resp && resp->status){

                    if(resp->status == 404){
                        cout << "Data not found!" << endl;
                    }

                    else if(resp->status == 200){
                        json response = json::parse(resp->body);
                        // for(auto data : response){
                        //     cout << "---------------------------------------------" << endl;
                        //     cout << "Name: "<< data["name"] <<endl;
                        //     cout << "Company: "<< data["company"] <<endl;
                        //     cout << "Rating: "<< data["rating"] <<endl;
                        //     cout << "Comments: "<< data["comments"] <<endl;
                            
                        // }
                    }

                    else{
                        cout << resp << endl;
                    }

                    ok = true;
                }
                else{
                    cout << "Error occured. Couldn't get the data" << endl;
                    ok = false;
                }
                
            }
        
        }

        auto t2 = chrono::steady_clock::now();

        if(ok) {
            long latency_ns = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count();
            total_latency_ns += latency_ns;
            total_requests_completed++;
        }

     }
}


int main(int argc, char** argv){

    if(argc < 3) {
        cout << "Usage: ./load_gen <threads> <seconds>\n";
        return 0;
    }


    int choice;
    int num_threads = atoi(argv[1]);
    int duration = atoi(argv[2]);

    vector<thread> threads;

    cout << endl << "1. Disk bound (put + delete)\n2. CPU bound (get popular keys)\n3. Mixed requests (Get + Put)\n";
    cout << "Enter your choice number: " << endl;
    cin >> choice;

    cout << "Load generator starting: "<< endl;

    for(int i = 0; i < num_threads; i++){
        threads.emplace_back(client_work, i, choice);
    }

    this_thread::sleep_for(chrono::seconds(duration));
    stop_flag.store(true);

    for (auto &t : threads){
        t.join();
    }

    long requests_completed = total_requests_completed.load();
    double average_response_time = (total_latency_ns.load() / 1e6) / (requests_completed > 0 ? requests_completed : 1L); //convert from nanosec to msec, and divide by total requests completed


     cout << "\n========= RESULTS =========" << endl;
    cout << "Total requests made: " << total_requests_made.load() << endl;
    cout << "Total requests completed: " << requests_completed << endl;
    cout << "Throughput:     " << requests_completed / duration << " req/sec" << endl;
    cout << "Avg response time:    " << average_response_time << " ms" << endl;
    cout << "===========================" << endl;

    
    return 0;

}
