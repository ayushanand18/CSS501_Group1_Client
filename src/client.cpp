#include <iostream>
#include <unordered_map>
#include "rpc/client.h"

class File{
public:
    // data members of a File type, adjust according to your needs
    string name, file_id, author, location_on_disc, last_update_time;
    size_t size;
    unsigned int num_downloads;

    // constructor to the File Class
    File(const string name, const string file_id, const string author, const string location_on_disc, const string last_update_time, const size_t size, const unsigned int num_downloads) {
      this->author = author;
      this->file_id = file_id;
      this->last_update_time = last_update_time;
      this->location_on_disc = location_on_disc;
      this->name = name;
      this->num_downloads = num_downloads;
      this->size = size;
    }
};

class Client {
private:
    // data members
    int user_id;
    bool is_signedin = false;
    // vector<string> server_ips; // list of server ips
    unordered_map<string, double> download_status; // download_status of files; file_id -> int percentage
    // own ThreadPool of threads to submit tasks to
    // ThreadPool thread_exec;
    rpc::client client;
    unordered_map<string, File> download_list;

    // #TODO: @ajay
    // private helper functions
    void __upload_file(string file_name, string perimissions) {
        // upload a single file to a server
        
        // 1. supply name, author, size, content and permissions to the API Call
        content = read(file_name);
        
        // make an API call
        client.call("upload", name, author, permissions, size, content);
    }
    // #TODO: @uday
    void __download_file(string file_id) {
        // just download the file and return the data
        
        // steps
        // 1. check permissions if given or not
        // 2. make an api call
        string file_content = client.call("download", file_id).as<string>();
        // 3. write to disc in same folder /downloads/file_id
        ofstream new_file(loc_on_disc/file_id.txt);
        new_file << file_content;
        // 4. show to user the progress (fake progress) with a progress bar
        // 5. update to download_list
    }
    // #TODO: @rudra
    void __view_files() {
        // iterate over the download_list map, and print out files in a tabular fashion
        // +-------------+---------+
        // | string name, file_id, author, location_on_disc, last_update_time|

    }
public:
    // #TODO: @ayushanand18
    Client(rpc::client client){
        // initialise all data
        this->client=client;
    }
    // #TODO: @ayushanand18
    ~Client(){
        // destructor, delete all user data
        // signout the user also,and break the socket connection (if any)
    }
    // #TODO: @ajay
    void upload() {
        // upload function to be called directly from frontend
        // these functions are directly called form UI so no arguments
        cout << "User Options" << endl; // add appropriate data input like, file path and permissions
        
        cin >> permissions >> path;
        
        // simply send string content
        __upload_file(file_name, permissions);
    }
    // #TODO: @uday
    void download() {
        // directly called by the user
        // get the file_id from the user by using suitable prompts and options
        
        // Steps
        // 1. show all files to users
        // 2. input file_id from user
        // 3. call the helper __download_file()

        cout<<"Input your fileId"<<endl;

        cin >> file_id;

        // get access for the particular file, check with server 
        // for an exposed endpoint to do this
     
        access = check_access(file_id, this->user_id)
        // if no access just return
        
        if (!access) throw Exception("No Access Permissions. try contacting the owner.")

        file_data = this->__download_file(file_id)
        
        // IO write the content to disc
        write(file_data.name, file_data.content)
    }
    void login() {
        // login the user, and set user_id, and is_signedin=true
        string user, password;
        cout<<"Enter your UserID: ";
        cin>>user;
        cout<<"Enter your Password: ";
        cin>>password;
        bool result = client.call("signin", user, password).as<bool>();
        if(result){
            cout<<"Successful sign-in."<<endl;
        }
        else{
            cout<<"Incorrect userId and Password."<<endl;
        }
        is_signedin= result;
    }
    void register(){
        // register a new user,and set user_id and is_signedin=true
        string user, password, name;
        cout<<"Enter your name: ";
        cin>>name;
        cout<<"Enter your UserID: ";
        cin>>user;
        cout<<"Enter your Password: ";
        cin>>password;

        bool result = client.call("register",name,user, password).as<bool>();
        if(result){
            cout<<"Successful registration."<<endl;
        }
        else{
            cout<<"Unknown failure occured."<<endl;
        }
        is_signedin = result;
    }
    // #TODO: @bikash
    void init() {
        // showup the menu, as soon as the object is constructed, this
        // function will be called and entered into a while loop until 
        // the program is being run
        int choice;
        while(true){
            if(!is_signedin){
                cout<<"File sharing System CLI"<<endl<<"1. Login\n 2. Register\n 3. Exit\n Please enter your choice : ";
                cin>>choice;
                switch(choice){
                    case 1: login();
                    break;
                    case 2: register();
                    break;
                    case 3: cout<<"**Thanks for using our system**"<<endl;
                    break;
                    default: cout<<"Please enter valid choice!"<<endl;
                }
            }
            else{
                cout<<"1. Upload a file \n2. Download a file"<<endl
                    <<"3. View files"<<endl
                    <<"4. Logout"<<endl;
                // switch for these options
            }
        }
    }
}

int main() {
    // Creating a client that connects to the localhost on port 8080
    rpc::client client("127.0.0.1", 8080);

    Client instance(client);
    instance.init();
}