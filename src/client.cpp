#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <msgpack.hpp>
#include<iomanip>
#include "rpc/client.h"

class File
{
public:
    // data members of a File type, adjust according to your needs
    std::string name, file_id, author, location_on_disc, last_update_time, access_to;
    size_t size;
    unsigned int num_downloads;

    // constructor to the File Class
    File()
    {
        this->access_to = "*";
        this->author = "none";
        this->file_id = "0";
        this->last_update_time = "0";
        this->location_on_disc = "null";
        this->name = "no_file";
        this->num_downloads = 0;
        this->size = 0;
    }
    File(const std::string name, const std::string file_id, const std::string author, const std::string location_on_disc, const std::string last_update_time, const size_t size, const unsigned int num_downloads, std::string access_to)
    {
        this->access_to = access_to;
        this->author = author;
        this->file_id = file_id;
        this->last_update_time = last_update_time;
        this->location_on_disc = location_on_disc;
        this->name = name;
        this->num_downloads = num_downloads;
        this->size = size;
    }
    MSGPACK_DEFINE(name, file_id, author, location_on_disc, last_update_time, size, num_downloads, access_to);
};

std::vector<std::string> split(std::string s, std::string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

class Client
{
private:
    // data members
    std::string user_id;
    bool is_signedin = false;
    // vector<string> server_ips; // list of server ips
    std::unordered_map<std::string, double> download_status; // download_status of files; file_id -> int percentage
    // own ThreadPool of threads to submit tasks to
    // ThreadPool thread_exec;

    rpc::client *client;
    std::unordered_map<std::string, File> download_list;
    std::unordered_set<std::string> downloaded_files;

    // private helper functions
    void __draw_init_pattern(){
        std::cout <<
  "_____  _  _         ____   _                   _                 ____               _                   " << std::endl <<
 "|  ___|(_)| |  ___  / ___| | |__    __ _  _ __ (_) _ __    __ _  / ___|  _   _  ___ | |_  ___  _ __ ___  " << std::endl << 
 "| |_   | || | / _ \\ \\___ \\ | '_ \\  / _` || '__|| || '_ \\  / _` | \\___ \\ | | | |/ __|| __|/ _ \\| '_ ` _ \\ " << std::endl << 
 "|  _|  | || ||  __/  ___) || | | || (_| || |   | || | | || (_| |  ___) || |_| |\\__ \\| |_|  __/| | | | | |" << std::endl << 
 "|_|    |_||_| \\___| |____/ |_| |_| \\__,_||_|   |_||_| |_| \\__, | |____/  \\__, ||___/ \\__|\\___||_| |_| |_|" << std::endl << 
 "                                                          |___/          |___/                           " << std::endl;
    }
    // contributed by @ajay
    std::string __getFileContent(const std::string &filepath, std::size_t &fileSize)
    {
        std::ifstream file(filepath, std::ios::binary);
        std::stringstream content;

        if (file.is_open())
        {
            file.seekg(0, std::ios::end);                      // Move to the end of the file
            fileSize = static_cast<std::size_t>(file.tellg()); // Get the file size
            file.seekg(0, std::ios::beg);                      // Move back to the beginning

            content << file.rdbuf();
            file.close();
        }
        else
        {
            throw std::runtime_error("Unable to open file: " + filepath);
        }

        return content.str();
    }
    // contributed by @ajay
    void __upload_file(std::string permissions, std::string path)
    {
        // upload a single file to a server
        size_t size_of_file;
        // 1. supply name, author, size, content and permissions to the API Call
        std::string content = __getFileContent(path, size_of_file);
        
        auto splitted_path = split(path, "/");
        // make an API call
        client->call("upload", splitted_path.back(), user_id, permissions, size_of_file, content);
    }
    // #TODO: @uday
    void __download_file(std::string file_id)
    {
        // just download the file and return the data

        // steps
        // 1. check permissions if given or not
        // 2. make an api call
        std::string file_content = client->call("download", file_id).as<std::string>();
        // 3. write to disc in same folder /downloads/file_id
        std::cout << "Download in progress..." << std::endl;
        std::ofstream new_file("downloads/" + file_id + "-" + download_list[file_id].name);
        new_file << file_content;
        new_file.close();
        
        std::cout << "Download complete" << std::endl;
        downloaded_files.insert(file_id);

    }
    // function to show files to the user with the list of files on the server
    // contributed by @rudra
    void __view_files()
    {
        std::cout <<">>> Showing all files. " << std::endl;
        download_list = client->call("get_files_list").as<std::unordered_map<std::string, File>>();
        std::cout << "+---------------------+--------------+-----------+-----------------------+--------+----+------------+" << std::endl <<
        "| " << std::left  << std::setw(20) << "file_id" << std::setw(1) << "| " 
                << std::setw(13) << "name" << std::setw(1) << "|"
                << std::setw(10) << "author" << std::setw(1) << "|" 
                << std::setw(24) << "last_update_time" << std::setw(1) << "|" 
                << std::setw(8) << "access?" << std::setw(1) << "|" 
                << std::setw(4) << "size" << std::setw(1) << "|"
                << std::setw(12) << "downloaded?" << std::setw(1) << "|" << std::endl;
        
        for (auto &it : download_list)
        {
            std::string file_id = it.first;
            File val = it.second;
            std::vector<std::string> accesses = split(val.access_to, "|");
            bool access = (val.access_to == "*" or find(accesses.begin(), accesses.end(), user_id) != accesses.end());
            bool downloaded = downloaded_files.find(file_id) != downloaded_files.end();

            std::cout << "+---------------------+--------------+-----------+-----------------------+--------+----+------------+" << std::endl <<
            "| " << std::left  << std::setw(20) << file_id << std::setw(1) << "| " 
                << std::setw(10) << val.name << std::setw(1) << "|"
                << std::setw(10) << val.author << std::setw(1) << "|" 
                << std::setw(24) << val.last_update_time.substr(0, 24) << std::setw(1) << "|" 
                << std::setw(8) << access << std::setw(1) << "|" 
                << std::setw(4) << val.size << std::setw(1) << "|"
                << std::setw(12) << downloaded << std::setw(1) << "|" << std::endl
            << "+---------------------+--------------+-----------+-----------------------+--------+----+------------+" << std::endl;
        }
    }

public:
    Client(rpc::client& client)
    {
        this->client = &client;
    }
    ~Client()
    {
        this->user_id = "";
        this->is_signedin = false;
    }
    // contributed by @ajay
    void upload()
    {
        // upload function to be called directly from frontend
        // these functions are directly called form UI so no arguments
        std::string path, permissions = "*";
        bool access;
        // add appropriate data input like, file path and permissions
        std::cout << "**Uploading**\nUser Options::" << std::endl;
        std::cout << "\tPath to the file: \t" ;
        std::cin >> path;
        std::cout << "\tDo you want to grant access of this file to other users (0/1): " ;

        std::cin >> access;
        if (access)
        {
            std::cout << "\t>>> Enter space seperated user names: ";
            std::cin >> permissions;
        }
        // simply send string content
        __upload_file(permissions, path);

        std::cout << "\tFile Uploaded! "<< std::endl;
    }
    // contributed by @uday
    void download()
    {
        // directly called by the user
        // get the file_id from the user by using suitable prompts and options
        __view_files();

        std::cout << "\tInput your file_id: \t" ;
        std::string inputed_fileId;
        std::cin >> inputed_fileId;

        bool access = client->call("check_access", user_id, inputed_fileId).as<bool>();

        if (!access)
        {
            std::cout << "[Error]: No Access Permissions. try contacting the owner." << std::endl;
        }
        else
        {
            __download_file(inputed_fileId);
        }
    }

    void login()
    {
        // login the user, and set user_id, and is_signedin=true
        std::string username, password;
        std::cout << "\tEnter your UserID: \t";
        std::cin >> username;
        std::cout << "\tEnter your Password: \t";
        std::cin >> password;
        bool result = client->call("signin", username, password).as<bool>();
        if (result)
            std::cout << "\n\tSuccessful sign-in.\n" << std::endl << "Welcome, " << username << "!\n\n";
        else
            std::cout << "\t\tIncorrect userId and Password.\n" << std::endl;

        if (result)
            user_id = username;
        is_signedin = result;
    }
    void signup()
    {
        // register a new user,and set user_id and is_signedin=true
        std::string username, password, name;
        std::cout << "Enter your name: ";
        std::cin >> name;
        std::cout << "Enter your UserID: ";
        std::cin >> username;
        std::cout << "Enter your Password: ";
        std::cin >> password;

        bool result = client->call("register", name, username, password).as<bool>();
        if (result)
            std::cout << "Successful registration." << std::endl;
        else
            std::cout << "Unknown failure occured." << std::endl;
        if (result)
            user_id = username;
        is_signedin = result;
    }
    // #TODO: @bikash
    void init()
    {
        // showup the menu, as soon as the object is constructed, this
        // function will be called and entered into a while loop until
        // the program is being run
        int choice;
        while (true)
        {
            if (!is_signedin)
            {
                __draw_init_pattern();
                std::cout << "Options:: " << std::endl << std::endl
                          << "1. Login\n2. Register\n3. Exit\n\nPlease enter your choice : ";
                std::cin >> choice;
                std::cout << std::endl;
                switch (choice)
                {
                case 1:
                    login();
                    break;
                case 2:
                    signup();
                    break;
                case 3:
                    std::cout << "**Thanks for using our system**" << std::endl;
                    exit(0);
                default:
                    std::cout << "Please enter valid choice!" << std::endl;
                }
            }
            else
            {
                std::cout << "Options::\n1. Upload a file \n2. Download a file" << std::endl
                          << "3. View files" << std::endl
                          << "4. Logout" << std::endl << std::endl;
                std::cout << "Please enter your choice : ";
                std::cin >> choice;
                std::cout << std::endl; 
                switch (choice)
                {
                case 1:
                    upload();
                    break;
                case 2:
                    download();
                    break;
                case 3:
                    __view_files();
                    break;
                case 4:
                    std::cout << "Logged Out Successfully. " << std::endl;
                    is_signedin = false;
                    break;
                default:
                    std::cout << "Please enter valid choice!" << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }
};

int main()
{
    rpc::client client("127.0.0.1", 8080);
    Client instance(client);
    instance.init();
}