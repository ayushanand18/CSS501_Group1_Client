#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <msgpack.hpp>
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
        download_list = client->call("get_files_list").as<std::unordered_map<std::string, File>>();
        std::cout << "\
        +--------------+---------+---------+-------------------+---------------+---------+------------+\
        | file_id\t\t\t| name\t\t| author\t| last_update_time\t|permissions\t\t| size \t | downloaded |\
        +--------------+---------+---------+-------------------+---------------+---------+------------+\
        " << std::endl;
        std::cout << "name\tfile_id\tauthor\tlocation_on_disc\tlast_update_time" << std::endl;
        for (auto &it : download_list)
        {
            std::string file_id = it.first;
            File val = it.second;
            std::vector<std::string> accesses = split(val.access_to, "|");
            bool access = (val.access_to == "*" or find(accesses.begin(), accesses.end(), user_id) != accesses.end());
            bool downloaded = downloaded_files.find(file_id) != downloaded_files.end();

            std::cout << "\
            +--------------+---------+---------+-------------------+---------------+---------+------------+\
            | " << file_id
                      << "\t| " << val.name << "\t\t| " << val.author << "\t| " << val.last_update_time << "\t|" << access << "\t\t| <<" << val.size << "\t | " << downloaded << " |\
            +--------------+---------+---------+-------------------+---------------+---------+------------+\
            " << std::endl;
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
        std::string path, permissions = "";
        bool access;
        // add appropriate data input like, file path and permissions
        std::cout << "User Options" << std::endl;
        std::cout << "give the path of the file to be uploaded" << std::endl;
        std::cin >> path;
        std::cout << "Do you want to grant access of this file to other users(0/1)" << std::endl;

        std::cin >> access;
        if (access)
        {
            std::cout << "enter space seperated user names" << std::endl;
            std::cin >> permissions;
        }
        // simply send string content
        __upload_file(permissions, path);
    }
    // contributed by @uday
    void download()
    {
        // directly called by the user
        // get the file_id from the user by using suitable prompts and options

        std::cout << ">>> Showing all files: " << std::endl;
        __view_files();

        std::cout << "Input your fileId" << std::endl;
        std::string inputed_fileId;
        std::cin >> inputed_fileId;

        bool access = client->call("check_access", user_id, inputed_fileId).as<bool>();

        if (!access)
        {
            std::cout << "No Access Permissions. try contacting the owner." << std::endl;
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
        std::cout << "Enter your UserID: ";
        std::cin >> username;
        std::cout << "Enter your Password: ";
        std::cin >> password;
        bool result = client->call("signin", username, password).as<bool>();
        if (result)
            std::cout << "Successful sign-in." << std::endl;
        else
            std::cout << "Incorrect userId and Password." << std::endl;

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
                std::cout << "File sharing System CLI" << std::endl
                          << "1. Login\n2. Register\n3. Exit\nPlease enter your choice : ";
                std::cin >> choice;
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
                std::cout << "1. Upload a file \n2. Download a file" << std::endl
                          << "3. View files" << std::endl
                          << "4. Logout" << std::endl;
                std::cout << "Please enter your choice : ";
                std::cin >> choice; 
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
                    is_signedin = false;
                    break;
                default:
                    std::cout << "Please enter valid choice!" << std::endl;
                }
            }
        }
    }
};

int main()
{
    rpc::client client("127.0.0.1", 8080);
    Client instance(client);
    instance.init();
}