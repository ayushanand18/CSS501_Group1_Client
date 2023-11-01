#include <iostream>
#include "rpc/client.h"

class Client {
private:
    // data members
    int user_id;
    bool is_signedin = false;
    vector<string> server_ips; // list of server ips
    unordered_map<string, double> download_status; // download_status of files; file_id -> int percentage
    // own ThreadPool of threads to submit tasks to
    ThreadPool thread_exec;

    // private helper functions
    void __upload_file(string file_name, string perimissions) {
        // upload a single file to a server
        random_index = random(0, this->server_ips.size())
        server = this->server_ips[random_index]
        
        // logic of socket connection to follow
        socket_connection = setup_socket(server)
        file_bytes = read(file_name)

        file_hash = hash(file_bytes)

        // check with server if hash is present or not
        is_present = socket_connection.send('/check_if_present', file_hash)

        if (is_present == true)
            return
        
        file_metadata  = {
            'name': file_name,
            'size': get_size(file_name)
            'author': this->user_id
            'content': file_bytes,
            'permissions': permissions
        }

        socket_connection.send('upload', file_metadata)
    }
    void __download(string file_id) {
        // just download the file and return the data
    }
public:
    Client(){
        // initialise all data
    }
    ~Client(){
        // destructor, delete all user data
        // signout the user also,and break the socket connection (if any)
    }
    void upload() {
        // upload function to be called directly from frontend
        // these functions are directly called form UI so no arguments
        cout << "User Options" << endl; // add appropriate data input like, file path and permissions
        
        cin >>permissions >> path;
        
        if path is a folder:
            // maintain a list of promises
            promises = {}
            for file in folder:
                // submit each file to the thread pool
                promise = ThreadPool.submit(this->__upload_file, args={file, permissions})
                promises.push(promise)

            for promise in promises:
                // to resolve each promise
                promise.result()
        else:
            ThreadPool.submit(this->__upload_file, args={path, permissions}).result()
    }
    void download() {
        // directly called by the user
        // get the file_id from the user by using suitable prompts and options
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
    }
    void register(){
        // register a new user,and set user_id and is_signedin=true
    }
    void init() {
        // showup the menu, as soon as the object is constructed, this
        // function will be called and entered into a while loop until 
        // the program is being run
    }
}

int main() {
    // Creating a client that connects to the localhost on port 8080
    rpc::client client("127.0.0.1", 8080);

    // Calling a function with paramters and converting the result to int
    auto result = client.call("add", 5, 3).as<int>();
    std::cout << "The result is: " << result << std::endl;
    return 0;
}