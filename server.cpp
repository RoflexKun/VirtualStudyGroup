#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <mutex>

using namespace std;

#define port 2606
extern int errno;

struct Account
{
    string username;
    string password;
    bool connected;
    int descriptor;
};

typedef struct thData
{
    int cl;
    bool error = false;
    string error_message;
    string finish_message;
    vector<int> *users_connected;
    string sender = "";
    string username = "";
    string password = "";
    string vpassword = "";
    string file_name = "";
    string message = "";
    vector<string> *username_list_td;
    bool done = false;
    mutex *mutx;
} thData;

vector<Account> listOfUsers()
{
    ifstream file("/home/razvan/Desktop/Proiect_RC/accounts.json");
    if (!file.is_open())
    {
        perror("Eroare la deschiderea accounts.json");
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    string data_from_json = buffer.str();
    size_t start = 0, end = 0;

    vector<Account> users;
    while ((start = data_from_json.find("{", end)) != string::npos)
    {
        Account temp_account;

        start = data_from_json.find("\"username\":", start);
        if (start == string::npos)
            break;
        start = data_from_json.find("\"", start + 11) + 1;
        end = data_from_json.find("\"", start);
        temp_account.username = data_from_json.substr(start, end - start);

        start = data_from_json.find("\"password\":", start);
        if (start == string::npos)
            break;
        start = data_from_json.find("\"", start + 11) + 1;
        end = data_from_json.find("\"", start);
        temp_account.password = data_from_json.substr(start, end - start);

        start = data_from_json.find("\"connected\":", start);
        if (start == string::npos)
            break;
        start = data_from_json.find(":", start) + 1;
        end = data_from_json.find(",", start);
        string temp_connected = data_from_json.substr(start, end - start);
        temp_account.connected = (temp_connected.find("true") != string::npos);

        start = data_from_json.find("\"descriptor\":", start);
        if (start == string::npos)
            break;
        start = data_from_json.find(":", start) + 1;
        end = data_from_json.find("}", start);
        string temp_desc = data_from_json.substr(start, end - start);
        temp_account.descriptor = stoi(temp_desc);

        users.push_back(temp_account);
    }

    return users;
}

void modify_json(vector<Account> users)
{
    ofstream file("/home/razvan/Desktop/Proiect_RC/accounts.json");
    if (!file.is_open())
    {
        perror("Eroare la deschiderea accounts.json");
    }

    file << "[\n";

    for (int i = 0; i < users.size(); i++)
    {
        Account temp_users = users[i];
        file << "  {\n";
        file << "    \"username\": \"" << temp_users.username << "\",\n";
        file << "    \"password\": \"" << temp_users.password << "\",\n";
        file << "    \"connected\": " << (temp_users.connected ? "true" : "false") << ",\n";
        file << "    \"descriptor\": " << temp_users.descriptor << "\n";

        if (i < users.size() - 1)
        {
            file << "  },\n";
        }
        else
        {
            file << "  }\n";
        }
    }

    file << "]\n";
    file.close();
}

void *numberOfUsers(void *arg)
{
    struct thData td;
    td = *((struct thData *)arg);
    td.error = false;

    string all_usernames = "";

    vector<Account> temp_username_list = listOfUsers();
    for (auto i : temp_username_list)
    {
        if (i.connected == true)
        {
            all_usernames += i.username;
            all_usernames += ";";
        }
    }
    if (write(td.cl, all_usernames.c_str(), all_usernames.size()) == -1)
    {
        perror("Erroare la write()");
    }
    return NULL;
}

void *verifyLogin(void *arg)
{
    thData *td = (thData *)arg;
    td->finish_message = "";
    vector<Account> users = listOfUsers();

    for (auto &i : users)
    {
        if (i.username == td->username && i.password == td->password && !i.connected)
        {
            i.connected = true;
            td->finish_message = "connected";
            td->done = true;
            i.descriptor = td->cl;
            cout << "Utilizatorul s-a conectat cu succes!" << '\n';
            modify_json(users);
            return NULL;
        }
        else if (i.username == td->username && i.password == td->password && i.connected)
        {
            td->error = true;
            td->done = true;
            td->error_message = "This account is currently logged in";
            return NULL;
        }
        else if (i.username == td->username && i.password != td->password)
        {
            td->error = true;
            td->done = true;
            td->error_message = "Your password is invalid. Maybe a typo?";
            return NULL;
        }
    }

    td->error = true;
    td->done = true;
    td->error_message = "There are no account with this credential! Create an account?";
    return NULL;
}

void *verifyRegister(void *arg)
{
    thData *td = (thData *)arg;
    if (td->password != td->vpassword)
    {
        td->error = true;
        td->error_message = "Passwords don't match!";
        td->done = true;
        return NULL;
    }

    vector<Account> users = listOfUsers();

    bool ok = true;
    for (auto i : users)
    {
        if (i.username == td->username)
        {
            ok = false;
            break;
        }
    }

    if (ok)
    {
        Account new_account;
        new_account.connected = false;
        new_account.username = td->username;
        new_account.password = td->password;
        users.push_back(new_account);
        td->done = true;
        td->finish_message = "Account created succesfully!";
        modify_json(users);
        return NULL;
    }
    else
    {
        td->error = true;
        td->error_message = "There is already an user with the same name!";
        td->done = true;
        return NULL;
    }
}

void *downloadFile(void *arg)
{
    thData *td = (thData *)arg;
    string path_to_file = "/home/razvan/Desktop/Proiect_RC/file_cloud/";
    string total_path = path_to_file + td->file_name;
    FILE *file = fopen(total_path.c_str(), "rb");

    if (!file)
    {
        perror("Eroare la deschiderea fisierului!");
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    cout << "Size-ul trimis de server: " << file_size << '\n';

    if (write(td->cl, &file_size, sizeof(file_size)) == -1)
    {
        perror("Eroare la write()");
        return NULL;
    }

    char buffer[1024];
    size_t current_bytes_sent = 0;
    while (current_bytes_sent < file_size)
    {
        unsigned long tmp = 1024;
        size_t remaining_bytes = min(tmp, file_size - current_bytes_sent);
        fread(buffer, 1, remaining_bytes, file);

        if (write(td->cl, buffer, remaining_bytes) == -1)
        {
            perror("Eroare la write()");
        }

        current_bytes_sent += remaining_bytes;
        cout << current_bytes_sent << '/' << file_size << '\n';
    }
    fclose(file);
}

void openListFile(vector<string> &files_name)
{
    ifstream file("/home/razvan/Desktop/Proiect_RC/file_cloud/list_of_file.json");
    if (!file.is_open())
    {
        perror("Eroare la deschiderea list_of_file.json");
        return;
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    string list_of_files = buffer.str();
    size_t start = 0, end = 0;
    while ((start = list_of_files.find("{", end)) != string::npos)
    {
        string temp_name;
        temp_name.clear();
        start = list_of_files.find("\"name\":", start);
        if (start == string::npos)
            break;
        start = list_of_files.find("\"", start + 7) + 1;
        end = list_of_files.find("\"", start);
        temp_name = list_of_files.substr(start, end - start);
        files_name.push_back(temp_name);
    }
}

void *fileNameList(void *arg)
{
    thData *td = (thData *)arg;
    vector<string> temp_files_name;

    openListFile(temp_files_name);

    string message_to_send = "";
    for (auto i : temp_files_name)
    {
        message_to_send += i;
        message_to_send += ";";
    }
    if (write(td->cl, message_to_send.c_str(), message_to_send.size()) == -1)
    {
        perror("Eroare la write()");
    }
    return NULL;
}

void updateJSON(vector<string> temp_file_name)
{
    ofstream file("/home/razvan/Desktop/Proiect_RC/file_cloud/list_of_file.json");
    if (!file.is_open())
    {
        perror("Eroare la deschiderea accounts.json");
    }

    file << "[\n";

    for (int i = 0; i < temp_file_name.size(); i++)
    {
        string file_name = temp_file_name[i];
        ;
        file << "  {\n";
        file << "    \"name\": \"" << file_name << "\"\n";

        if (i < temp_file_name.size() - 1)
        {
            file << "  },\n";
        }
        else
        {
            file << "  }\n";
        }
    }

    file << "]\n";
    file.close();
}

void updateFileJSON(string file_name)
{
    vector<string> temp_file_name;

    openListFile(temp_file_name);
    temp_file_name.push_back(file_name);
    updateJSON(temp_file_name);
}

map<string, vector<string>> openUsersJSON()
{
    map<string, vector<string>> map_to_send;

    ifstream file("/home/razvan/Desktop/Proiect_RC/users_on_file.json");

    if (!file.is_open())
    {
        perror("Eroare la deschiderea users_on_file.json");
    }

    ostringstream buffer;
    buffer << file.rdbuf();

    string json_users_content = buffer.str();

    size_t start = 0, end = 0;

    while ((start = json_users_content.find("{", end)) != string::npos)
    {
        string file_name;
        vector<string> users;

        start = json_users_content.find("\"fileName\":", start);
        if (start == string::npos)
            break;

        start = json_users_content.find("\"", start + 10) + 1;
        end = json_users_content.find("\"", start);
        file_name = json_users_content.substr(start, end - start);

        start = json_users_content.find("\"users\":", end);
        if (start == string::npos)
            break;

        start = json_users_content.find("[", start) + 1;
        end = json_users_content.find("]", start);

        string temp = json_users_content.substr(start, end - start);
        size_t start_2 = 0, end_2 = 0;

        while ((start_2 = temp.find("\"", end_2)) != string::npos)
        {
            start_2++;
            end_2 = temp.find("\"", start_2);
            if (end_2 == string::npos)
                break;
            users.push_back(temp.substr(start_2, end_2 - start_2));

            end_2 = temp.find(",", end_2);
            if (end_2 != string::npos)
                end_2++;
        }

        map_to_send[file_name] = users;
        end = json_users_content.find("}", end) + 1;
    }
    return map_to_send;
}

void modifyUsersJSON(map<string, vector<string>> users_map)
{
    ofstream file("/home/razvan/Desktop/Proiect_RC/users_on_file.json");
    if (!file.is_open())
    {
        perror("Eroare la deschiderea fisierului users_on_file.json");
    }

    file << "[\n";
    bool first = true;
    for (auto i : users_map)
    {
        if (!first)
        {
            file << ",\n";
        }
        first = false;

        file << "  {\n";
        file << "   \"fileName\": \"" << i.first << "\",\n";
        file << "   \"users\": [";

        vector<string> temp = i.second;
        for (size_t j = 0; j < temp.size(); j++)
        {
            file << "\"" << temp[j] << "\"";
            if (j < temp.size() - 1)
                file << ", ";
        }

        file << "]\n";
        file << "  }";
    }

    file << "\n]\n";
    file.close();
}

void *addUserOnFile(void *arg)
{
    thData *td = (thData *)arg;
    string message_to_send = "";

    map<string, vector<string>> temp_map = openUsersJSON();

    bool found = false;
    for (auto &i : temp_map)
    {
        i.second.erase(remove(i.second.begin(), i.second.end(), td->username), i.second.end());
    }

    temp_map[td->file_name].push_back(td->username);

    modifyUsersJSON(temp_map);

    for (auto i : temp_map[td->file_name])
    {
        message_to_send += i;
        message_to_send += ";";
    }

    if (write(td->cl, message_to_send.c_str(), sizeof(message_to_send)) == -1)
    {
        perror("Eroare la write()");
    }
}

void *sendUserOnFile(void *arg)
{
    thData *td = (thData *)arg;
    map<string, vector<string>> temp_map = openUsersJSON();
    string message_to_send = "";

    for (auto i : temp_map[td->file_name])
    {
        message_to_send += i;
        message_to_send += ";";
    }
    if (write(td->cl, message_to_send.c_str(), sizeof(message_to_send)) == -1)
    {
        perror("Eroare la write()");
    }
}

vector<pair<string, string>> openChatLogJSON()
{
    vector<pair<string, string>> temp_list;

    ifstream file("/home/razvan/Desktop/Proiect_RC/chat_log.json");

    if (!file.is_open())
    {
        perror("Eroare la deschiderea users_on_file.json");
    }

    ostringstream buffer;
    buffer << file.rdbuf();

    string json_users_content = buffer.str();

    size_t start = 0, end = 0;

    while ((start = json_users_content.find("{", end)) != string::npos)
    {
        pair<string, string> user_and_message;

        start = json_users_content.find("\"username\":", start);
        if (start == string::npos)
            break;

        start = json_users_content.find("\"", start + 10) + 1;
        end = json_users_content.find("\"", start);
        user_and_message.first = json_users_content.substr(start, end - start);

        start = json_users_content.find("\"message\":", start);
        if (start == string::npos)
            break;

        start = json_users_content.find("\"", start + 9) + 1;
        end = json_users_content.find("\"", start);
        user_and_message.second = json_users_content.substr(start, end - start);

        temp_list.push_back(user_and_message);
        end = json_users_content.find("}", end) + 1;
    }
    return temp_list;
}

void addMessageToJSON(vector<pair<string, string>> temp_vector)
{
    ofstream file("/home/razvan/Desktop/Proiect_RC/chat_log.json");
    if (!file.is_open())
    {
        perror("Eroare la deschiderea fisierului chat_log.json");
    }

    file << "[\n";
    bool first = true;
    for (auto i : temp_vector)
    {
        if (!first)
        {
            file << ",\n";
        }
        first = false;

        file << "  {\n";
        file << "   \"username\": \"" << i.first << "\",\n";
        file << "   \"message\": \"" << i.second << "\"\n";
        file << "  }";
    }

    file << "\n]\n";
    file.close();
}

void *saveInChatLog(void *arg)
{
    thData *td = (thData *)arg;
    vector<pair<string, string>> fullChatLog = openChatLogJSON();

    pair<string, string> temp_pair;
    temp_pair.first = td->username;
    temp_pair.second = td->message;

    fullChatLog.push_back(temp_pair);

    addMessageToJSON(fullChatLog);
    return NULL;
}

void *sendLog(void *arg)
{
    thData *td = (thData *)arg;

    vector<pair<string, string>> fullChatLog = openChatLogJSON();

    string message_to_send = "";
    for (auto i : fullChatLog)
    {
        string temp = i.first + ": " + i.second + "~";
        message_to_send += temp;
    }
    if (write(td->cl, message_to_send.c_str(), message_to_send.size() + 1) == -1)
    {
        perror("Eroare la write()");
    }
    return NULL;
}

void *sendNotification(void *arg)
{
    thData *td = (thData *)arg;
    string temp_message = td->message;
    temp_message = temp_message.substr(1);

    string temp_username = "";
    for (int i = 0; i < temp_message.size(); i++)
    {
        if (temp_message[i] == ':')
        {
            break;
        }
        else
        {
            temp_username += temp_message[i];
        }
    }

    map<string, vector<string>> temp_map = openUsersJSON();
    vector<Account> total_users = listOfUsers();

    string temp_file_name = "";
    for (auto i : temp_map)
    {
        for (auto j : i.second)
        {
            if (j == temp_username)
            {
                temp_file_name = i.first;
                break;
            }
        }
        if (!temp_file_name.empty())
            break;
    }

    for (int i = 0; i < temp_map[temp_file_name].size(); i++)
    {
        for (auto j : total_users)
        {
            if (j.username == temp_map[temp_file_name][i])
            {
                if (write(j.descriptor, temp_message.c_str(), temp_message.size() + 1) == -1)
                {
                    perror("Eroare la write()");
                }
            }
        }
    }
}

void *handleUser(void *arg)
{

    thData *td = (thData *)arg;

    vector<int> *temp_users = td->users_connected;
    fd_set readfds;
    fd_set actfds;
    struct timeval tv;

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(td->cl, &readfds);

    bool user_disconnected = false;
    bool logged = false;
    while (1)
    {
        vector<int> new_users_list;
        {
            lock_guard<mutex> lock(*td->mutx);
            new_users_list = *temp_users;
        }
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(td->cl, &readfds);

        if (select(td->cl + 1, &readfds, nullptr, nullptr, &tv))
        {
            char buffer[1000];
            bzero(buffer, sizeof(buffer));
            if (read(td->cl, &buffer, sizeof(buffer)) == -1)
            {
                perror("Eroare la read()");
            }
            buffer[strlen(buffer)] = '\0';

            if (buffer[0] == '~')
            {
                pthread_t thread_login;
                thData *thrd = new thData();
                thrd->cl = td->cl;
                string tempusr = "";
                string temppass = "";
                int space = 0;
                for (int i = 1; i < strlen(buffer); i++)
                {
                    if (buffer[i] != '~')
                        tempusr += buffer[i];
                    else
                    {
                        space = i;
                        break;
                    }
                }

                for (int i = space + 1; i < strlen(buffer); i++)
                {
                    temppass += buffer[i];
                }
                thrd->username = tempusr;
                thrd->password = temppass;
                if (pthread_create(&thread_login, NULL, verifyLogin, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_login);

                while (!thrd->done)
                {
                    usleep(1000);
                }

                if (!thrd->error)
                {
                    if (write(td->cl, thrd->finish_message.c_str(), thrd->finish_message.size()) == -1)
                    {
                        {
                            lock_guard<mutex> lock(*td->mutx);
                            td->username_list_td->push_back(tempusr);
                        }
                        logged = true;
                        perror("Eroare la write()");
                    }
                }
                else
                {
                    string errormsg = "nu s-a conectat!";
                    if (write(td->cl, thrd->error_message.c_str(), thrd->error_message.size()) == -1)
                    {
                        perror("Eroare la write()");
                    }
                }
            }
            else if (buffer[0] == '^')
            {
                pthread_t thread_register;
                thData *thrd = new thData();
                thrd->cl = td->cl;
                string tempusr = "";
                string temppass = "";
                string temprpass = "";
                int space = 0;
                for (int i = 1; i < strlen(buffer); i++)
                {
                    if (buffer[i] != '^')
                        tempusr += buffer[i];
                    else
                    {
                        space = i;
                        break;
                    }
                }
                for (int i = space + 1; i < strlen(buffer); i++)
                {
                    if (buffer[i] != '^')
                        temppass += buffer[i];
                    else
                    {
                        space = i;
                        break;
                    }
                }
                for (int i = space + 1; i < strlen(buffer); i++)
                {
                    if (buffer[i] != '^')
                        temprpass += buffer[i];
                    else
                    {
                        space = i;
                        break;
                    }
                }
                td->username = tempusr;
                td->password = temppass;
                td->vpassword = temprpass;
                if (pthread_create(&thread_register, NULL, verifyRegister, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_register);

                while (!thrd->done)
                {
                    usleep(1000);
                }

                if (!thrd->error)
                {
                    if (write(td->cl, thrd->finish_message.c_str(), thrd->finish_message.size()) == -1)
                    {
                        perror("Eroare la write()");
                    }
                }
                else
                {
                    if (write(td->cl, thrd->error_message.c_str(), thrd->error_message.size()) == -1)
                    {
                        perror("Eroare la write()");
                    }
                }
            }
            else if (strcmp(buffer, "filelist") == 0)
            {
                pthread_t thread_files;
                thData *thrd = new thData();
                thrd->cl = td->cl;
                if (pthread_create(&thread_files, NULL, fileNameList, thrd) == -1)
                {
                    perror("Eroare la crearea threadului");
                }
                pthread_detach(thread_files);
            }
            else if (buffer[0] == '!')
            {
                pthread_t thread_file;
                thData *thrd = new thData();
                thrd->cl = td->cl;

                string file_name = "";
                for (int i = 1; i < strlen(buffer); i++)
                {
                    file_name += buffer[i];
                }

                thrd->file_name = file_name;
                if (pthread_create(&thread_file, NULL, downloadFile, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_file);
            }
            else if (buffer[0] == '<')
            {
                string file_name = "";
                for (int i = 1; i < strlen(buffer); i++)
                {
                    file_name += buffer[i];
                }

                string path_to_save = "/home/razvan/Desktop/Proiect_RC/file_cloud/" + file_name;

                fd_set readfds;
                struct timeval tv;

                tv.tv_sec = 5;
                tv.tv_usec = 0;

                FD_ZERO(&readfds);
                FD_SET(td->cl, &readfds);

                FILE *file = fopen(path_to_save.c_str(), "wb");
                if (!file)
                {
                    perror("Eroare la deschiderea fisierului!");
                }

                size_t file_size;
                if (select(td->cl + 1, &readfds, nullptr, nullptr, &tv))
                {
                    if (read(td->cl, &file_size, sizeof(file_size)) == -1)
                    {
                        perror("Eroare la read()");
                    }

                    char buffer_file[1024];
                    size_t current_bytes_received = 0;

                    while (current_bytes_received < file_size)
                    {
                        size_t bytes_read = read(td->cl, &buffer_file, sizeof(buffer_file));
                        if (bytes_read == -1)
                        {
                            perror("Eroare la read()");
                        }
                        fwrite(buffer_file, 1, bytes_read, file);
                        current_bytes_received += bytes_read;
                        cout << current_bytes_received << '/' << file_size << '\n';
                    }
                }
                fclose(file);
                updateFileJSON(file_name);
            }
            else if (strcmp(buffer, "numberusers") == 0)
            {
                pthread_t thread;
                thData *thrd = new thData();
                thrd->cl = td->cl;
                if (pthread_create(&thread, NULL, numberOfUsers, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread);
            }
            else if (buffer[0] == '(')
            {
                string file_name = "";
                string user_name = "";
                pthread_t thread_users_on_files;
                thData *thrd = new thData;
                thrd->cl = td->cl;

                int space = 0;
                for (int i = 1; i < strlen(buffer); i++)
                {
                    if (buffer[i] == ';')
                    {
                        space = i;
                        break;
                    }
                    else
                    {
                        file_name += buffer[i];
                    }
                }
                for (int i = space + 1; i < strlen(buffer); i++)
                {
                    user_name += buffer[i];
                }
                thrd->file_name = file_name;
                thrd->username = user_name;
                if (pthread_create(&thread_users_on_files, NULL, addUserOnFile, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_users_on_files);
            }
            else if (buffer[0] == ')')
            {
                pthread_t thread_open_json_users;
                thData *thrd = new thData;
                thrd->cl = td->cl;
                string temp_file_name = "";
                for (int i = 1; i < strlen(buffer); i++)
                {
                    temp_file_name += buffer[i];
                }
                thrd->file_name = temp_file_name;
                if (pthread_create(&thread_open_json_users, NULL, sendUserOnFile, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_open_json_users);
            }
            else if (buffer[0] == '?')
            {
                string temp_file_name = "";
                for (int i = 1; i < strlen(buffer); i++)
                {
                    temp_file_name += buffer[i];
                }
                fd_set readfds;
                struct timeval tv;
                tv.tv_sec = 5;
                tv.tv_usec = 0;

                FD_ZERO(&readfds);
                FD_SET(td->cl, &readfds);

                size_t file_size;
                ssize_t bytes_received;
                string file_to_compare_path = "/home/razvan/Desktop/Proiect_RC/file_cloud/" + temp_file_name;
                FILE *file = fopen(file_to_compare_path.c_str(), "rb");

                if (!file)
                {
                    perror("Eroare la deschiderea fisierului!");
                    return nullptr;
                }
                if (select(td->cl + 1, &readfds, nullptr, nullptr, &tv))
                {
                    if (read(td->cl, &file_size, sizeof(file_size)) == -1)
                    {
                        perror("Eroare la read()");
                    }
                    cout << "file size: " << file_size << '\n';

                    char *user_file_buffer = new char[file_size];
                    char buffer_file[1024];
                    size_t current_bytes_received = 0;

                    while (current_bytes_received < file_size)
                    {
                        size_t bytes_read = read(td->cl, &buffer_file, sizeof(buffer_file));
                        if (bytes_read == -1)
                        {
                            perror("Eroare la read()");
                        }
                        memcpy(user_file_buffer + current_bytes_received, buffer_file, bytes_read);
                        current_bytes_received += bytes_read;
                        cout << current_bytes_received << '/' << file_size << '\n';
                    }

                    fseek(file, 0, SEEK_END);
                    size_t server_file_size = ftell(file);
                    rewind(file);
                    cout << "Server file size: " << server_file_size << '\n';
                    char *server_file_buffer = new char[server_file_size];
                    if (fread(server_file_buffer, 1, server_file_size, file) != server_file_size)
                    {
                        perror("Eroare la citirea din fisier!");
                    }

                    bool result = (memcmp(server_file_buffer, user_file_buffer, max(server_file_size, file_size)) == 0);

                    fclose(file);

                    if (!result)
                    {
                        FILE *file = fopen(file_to_compare_path.c_str(), "wb+");
                        size_t total_bytes = current_bytes_received;
                        size_t current_bytes = 0;

                        while (total_bytes)
                        {
                            size_t cut_size = (total_bytes > 1024) ? 1024 : total_bytes;
                            size_t bytes_write = write(fileno(file), user_file_buffer + current_bytes, cut_size);
                            if (bytes_write == -1)
                            {
                                perror("Eroare la write()");
                            }
                            current_bytes += bytes_write;
                            total_bytes -= bytes_write;
                        }
                        fclose(file);

                        map<string, vector<string>> temp_map = openUsersJSON();
                        vector<Account> total_users = listOfUsers();
                        string temp_name = "";
                        for (auto i : total_users)
                        {
                            if (i.descriptor == td->cl)
                            {
                                temp_name = i.username;
                            }
                        }

                        string message_to_send = temp_name + ": modified the file! The file was also updated in the cloud!";
                        if (write(td->cl, message_to_send.c_str(), message_to_send.size()) == -1)
                        {
                            perror("Eroare la write()");
                        }
                        delete[] server_file_buffer;
                        delete[] user_file_buffer;
                    }
                    else
                    {
                        string message_to_send = "nothing!";
                        if (write(td->cl, message_to_send.c_str(), message_to_send.size() + 1) == -1)
                        {
                            perror("Eroare la write()");
                        }
                        delete[] server_file_buffer;
                        delete[] user_file_buffer;
                    }
                }
            }
            else if (buffer[0] == '/')
            {
                pthread_t thread_send_notification;
                thData *thrd = new thData;
                thrd->cl = td->cl;
                thrd->message = buffer;

                if (pthread_create(&thread_send_notification, NULL, sendNotification, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_send_notification);
            }
            else if (strcmp(buffer, "chatlog") == 0)
            {
                pthread_t thread_send_log;
                thData *thrd = new thData;
                thrd->cl = td->cl;
                if (pthread_create(&thread_send_log, NULL, sendLog, thrd) == -1)
                {
                    perror("Eroare la creare thread");
                }
                pthread_detach(thread_send_log);
            }
            else if (strcmp(buffer, "quit") == 0)
            {
                user_disconnected = true;
                logged = false;
            }
            else if (buffer[0] == '+')
            {
                string temp_username = string(buffer);
                temp_username = temp_username.substr(1);
                map<string, vector<string>> temp_map = openUsersJSON();

                for (auto &i : temp_map)
                {
                    i.second.erase(remove(i.second.begin(), i.second.end(), temp_username), i.second.end());
                }

                modifyUsersJSON(temp_map);
            }
            else
            {
                if (strcmp(buffer, ""))
                {
                    pthread_t thread_chatlog;
                    thData *thrd = new thData;
                    thrd->cl = td->cl;
                    vector<Account> listUsers = listOfUsers();
                    for (auto i : listUsers)
                    {
                        if (i.descriptor == td->cl)
                        {
                            thrd->username = i.username;
                            break;
                        }
                    }
                    thrd->message = string(buffer);
                    if (pthread_create(&thread_chatlog, NULL, saveInChatLog, thrd) == -1)
                    {
                        perror("Eroare la creare thread");
                    }
                    pthread_detach(thread_chatlog);
                }
            }
        }
        if (user_disconnected)
        {
            break;
        }
    }

    if (user_disconnected)
    {
        string username = "";
        vector<Account> temp_list = listOfUsers();
        for (auto &i : temp_list)
        {
            if (i.descriptor == td->cl)
            {
                username = i.username;
                i.descriptor = -1;
                i.connected = false;
                break;
            }
        }
        modify_json(temp_list);

        map<string, vector<string>> temp_map = openUsersJSON();

        for (auto &i : temp_map)
        {
            i.second.erase(remove(i.second.begin(), i.second.end(), username), i.second.end());
        }

        modifyUsersJSON(temp_map);
    }

    return NULL;
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    fd_set readfds;
    fd_set actfds;
    struct timeval tv;
    int sd, client;
    int optval = 1;
    int fd;

    int nfds;
    socklen_t len;
    vector<int> users_connected;
    mutex mutx;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket()");
        return errno;
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Eroare la bind()");
        return errno;
    }

    if (listen(sd, SOMAXCONN) == -1)
    {
        perror("Eroare la listen()");
        return errno;
    }

    FD_ZERO(&actfds);
    FD_SET(sd, &actfds);

    nfds = sd;

    while (1)
    {
        bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));
        readfds = actfds;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if (select(nfds + 1, &readfds, NULL, NULL, &tv))
        {
            len = sizeof(from);
            bzero(&from, sizeof(from));
            client = accept(sd, (struct sockaddr *)&from, &len);
            if (client < 0)
            {
                perror("Eroare la accept()");
            }
            FD_SET(client, &actfds);
            {
                lock_guard<mutex> lock(mutx);
                users_connected.push_back(client);
            }
            if (nfds < client)
                nfds = client;
            pthread_t thread_user;
            thData *td = new thData();
            td->users_connected = &users_connected;
            td->mutx = &mutx;
            td->cl = client;

            if (pthread_create(&thread_user, NULL, handleUser, td) == -1)
            {
                perror("Eroare la creare thread!");
            }
            pthread_detach(thread_user);
        }
    }
}
