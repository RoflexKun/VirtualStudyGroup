#include <bits/stdc++.h>
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
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTimer>
#include <QTextEdit>
#include <QListWidget>
#include <QHeaderView>
#include <fstream>
#include <sstream>
#include <QDialog>

using namespace std;

string file_name;
string user_name;
string file_name_shared;

void openFileList(QListWidget &fileList, int sd)
{
    string message_to_send = "filelist";
    if (write(sd, message_to_send.c_str(), message_to_send.size()) == -1)
    {
        perror("Eroare la write()");
    }

    fd_set readfds;
    struct timeval tv;

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(sd, &readfds);

    char filelist_string[10000];
    bzero(filelist_string, 10000);
    if (select(sd + 1, &readfds, nullptr, nullptr, &tv))
    {
        if (read(sd, &filelist_string, sizeof(filelist_string)) == -1)
        {
            perror("Eroare la read()");
        }
        filelist_string[strlen(filelist_string)] = '\0';

    }

    vector<string> file_name;
    string temp_name = "";
    for (int i = 0; i < strlen(filelist_string); i++)
    {
        if (filelist_string[i] == ';')
        {
            file_name.push_back(temp_name);
            temp_name = "";
        }
        else
        {
            temp_name += filelist_string[i];
        }
    }
    for (auto i : file_name)
    {
        QString temp = QString::fromStdString(i);
        fileList.addItem(temp);
    }
}

void clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr)
    {

        if (item->widget())
        {
            QWidget *widget = item->widget();
            widget->setParent(nullptr);
        }
        else if (item->layout())
        {
            QLayout *subLayout = item->layout();
            clearLayout(subLayout);
        }
        delete item;
    }
}

void connectWindow(QApplication &app, int &port, int &sd, struct sockaddr_in &server)
{
    QWidget *connectServerWindow = new QWidget();
    connectServerWindow->setWindowTitle("Connect to Server");
    connectServerWindow->resize(400, 200);

    QHBoxLayout *layout_address = new QHBoxLayout();
    QLabel *label_address = new QLabel("Adresa server: ");
    label_address->setFixedWidth(100);
    layout_address->addWidget(label_address);
    QLineEdit *lineEdit_address = new QLineEdit();
    lineEdit_address->setFixedSize(300, 20);
    layout_address->addWidget(lineEdit_address);

    QHBoxLayout *layout_port = new QHBoxLayout();
    QLabel *label_port = new QLabel("Port: ");
    label_port->setFixedWidth(100);
    layout_port->addWidget(label_port);
    QLineEdit *lineEdit_port = new QLineEdit();
    lineEdit_port->setFixedSize(300, 20);
    layout_port->addWidget(lineEdit_port);

    QVBoxLayout *layout_buttonConnect = new QVBoxLayout();
    QHBoxLayout *buttonOrdered = new QHBoxLayout();
    QPushButton *connectButton = new QPushButton("Connect");
    connectButton->setFixedWidth(100);
    buttonOrdered->addStretch();
    buttonOrdered->addWidget(connectButton);
    buttonOrdered->addStretch();

    QHBoxLayout *errorOrdered = new QHBoxLayout();
    QLabel *errorConnect = new QLabel("");
    errorConnect->setStyleSheet("color: red;");
    errorConnect->setAlignment(Qt::AlignCenter);
    errorOrdered->addStretch();
    errorOrdered->addWidget(errorConnect);
    errorOrdered->addStretch();

    layout_buttonConnect->addLayout(buttonOrdered);
    layout_buttonConnect->addLayout(errorOrdered);

    QVBoxLayout *final_layout = new QVBoxLayout();
    final_layout->addSpacing(20);
    final_layout->addLayout(layout_address);
    final_layout->addSpacing(30);
    final_layout->addLayout(layout_port);
    final_layout->addSpacing(20);
    final_layout->addLayout(layout_buttonConnect);

    connectServerWindow->setLayout(final_layout);
    QEventLoop waitLoop;

    QObject::connect(connectButton, &QPushButton::clicked, [&waitLoop, connectServerWindow, lineEdit_address, lineEdit_port, &server, &sd, &port, errorConnect]()
                     {
       
        QString server_address_temp = lineEdit_address->text();
        QString port_temp = lineEdit_port->text();

        if(server_address_temp.isEmpty() || port_temp.isEmpty())
        {
            errorConnect->setText("Completati ambele campuri!");
        }

        string server_address = server_address_temp.toStdString();
        string portString = port_temp.toStdString();

        port = 0;
        for(auto i : portString)
        {
            port = port*10 + (i - '0');
        }

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            errorConnect->setText("Eroare la socket()!");
            return errno;
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(server_address.c_str());

        if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        {
            errorConnect->setText("Eroare la connect()!");
            return errno;
        }
        connectServerWindow->close();
        waitLoop.quit(); });

    connectServerWindow->show();
    waitLoop.exec();
}

void loginWindow(QApplication &app, int sd)
{
    QEventLoop waitLoop;
    QWidget *loginwindow = new QWidget();
    loginwindow->setWindowTitle("Login");
    loginwindow->resize(500, 200);
    int verify_page = 0;

    QHBoxLayout *username_layout = new QHBoxLayout();
    QLabel *username = new QLabel("Username:");
    QLineEdit *insert_username = new QLineEdit();
    insert_username->setFixedHeight(25);
    username_layout->addWidget(username);
    username_layout->addWidget(insert_username);

    QHBoxLayout *password_layout = new QHBoxLayout();
    QLabel *password = new QLabel("Password:");
    QLineEdit *insert_password = new QLineEdit();
    insert_password->setFixedHeight(25);
    insert_password->setEchoMode(QLineEdit::Password);
    password_layout->addWidget(password);
    password_layout->addSpacing(2);
    password_layout->addWidget(insert_password);

    QHBoxLayout *buttons_layout = new QHBoxLayout();
    QPushButton *login_button = new QPushButton("Login");
    QPushButton *register_button = new QPushButton("Register");
    buttons_layout->addWidget(login_button);
    buttons_layout->addWidget(register_button);

    QHBoxLayout *error_ordered = new QHBoxLayout();
    QLabel *error_message = new QLabel("");
    error_message->setFixedHeight(20);
    error_message->setAlignment(Qt::AlignCenter);
    error_ordered->addStretch();
    error_ordered->addWidget(error_message);
    error_ordered->addStretch();

    QVBoxLayout *final_layout = new QVBoxLayout();

    QHBoxLayout *layout_verify_password = new QHBoxLayout();
    QLabel *verify_pass = new QLabel("Confirm Pass:");
    QLineEdit *insert_verify_pass = new QLineEdit();
    insert_verify_pass->setEchoMode(QLineEdit::Password);
    layout_verify_password->addWidget(verify_pass);
    layout_verify_password->addWidget(insert_verify_pass);
    verify_pass->hide();
    insert_verify_pass->hide();

    final_layout->addSpacing(30);
    final_layout->addLayout(username_layout);
    final_layout->addSpacing(10);
    final_layout->addLayout(password_layout);
    final_layout->addSpacing(10);
    final_layout->addLayout(layout_verify_password);
    final_layout->addSpacing(30);
    final_layout->addLayout(buttons_layout);
    final_layout->addSpacing(10);
    final_layout->addLayout(error_ordered);

    loginwindow->setLayout(final_layout);

    QObject::connect(login_button, &QPushButton::clicked, [&]()
                     {
                         if (verify_page)
                         {
                             verify_page = 0;
                             QLayout *old_layout = loginwindow->layout();
                             clearLayout(old_layout);
                             delete old_layout;

                             insert_password->clear();
                             insert_username->clear();
                             QHBoxLayout *user_layout = new QHBoxLayout();
                             QHBoxLayout *pass_layout = new QHBoxLayout();
                             QHBoxLayout *bttns_layout = new QHBoxLayout();
                             QHBoxLayout *error_ord = new QHBoxLayout();
                             QHBoxLayout *vpass_layout = new QHBoxLayout();
                             QVBoxLayout *new_layout = new QVBoxLayout();

                             user_layout->addWidget(username);
                             user_layout->addWidget(insert_username);

                             pass_layout->addWidget(password);
                             pass_layout->addSpacing(2);
                             pass_layout->addWidget(insert_password);

                             bttns_layout->addWidget(login_button);
                             bttns_layout->addWidget(register_button);

                             error_ord->addStretch();
                             error_ord->addWidget(error_message);
                             error_ord->addStretch();

                             vpass_layout->addWidget(verify_pass);
                             vpass_layout->addWidget(insert_verify_pass);
                             verify_pass->hide();
                             insert_verify_pass->hide();

                             new_layout->addSpacing(30);
                             new_layout->addLayout(user_layout);
                             new_layout->addSpacing(10);
                             new_layout->addLayout(pass_layout);
                             new_layout->addSpacing(10);
                             new_layout->addLayout(vpass_layout);
                             new_layout->addSpacing(30);
                             new_layout->addLayout(bttns_layout);
                             new_layout->addSpacing(10);
                             new_layout->addLayout(error_ord);

                             loginwindow->setLayout(new_layout);
                         }
                         else
                         {

                             string message_to_send = "~";

                             QString text_username = insert_username->text();
                             QString text_password = insert_password->text();

                             if (text_username.isEmpty() && !text_password.isEmpty())
                             {
                                 error_message->setText("You need to insert an username!");
                                 error_message->setStyleSheet("color: red; ");
                             }
                             else if (text_password.isEmpty() && !text_username.isEmpty())
                             {
                                 error_message->setText("Forgot your password? =)");
                                 error_message->setStyleSheet("color: red; ");
                             }
                             else if (text_password.isEmpty() && text_username.isEmpty())
                             {
                                 error_message->setText("No input = no connect");
                                 error_message->setStyleSheet("color: red; ");
                             }
                             else
                             {
                                 string temp_username = text_username.toStdString();
                                 string temp_password = text_password.toStdString();
                                 message_to_send += temp_username;
                                 message_to_send += "~";
                                 message_to_send += temp_password;
                                 message_to_send.push_back('\0');
                                 if (write(sd, message_to_send.c_str(), message_to_send.size()) == -1)
                                 {
                                     error_message->setText("Eroare la write()");
                                     error_message->setStyleSheet("color: purple; ");
                                 }

                                 fd_set readfds;
                                 struct timeval tv;

                                 tv.tv_sec = 1;
                                 tv.tv_usec = 0;

                                 FD_ZERO(&readfds);
                                 FD_SET(sd, &readfds);

                                 if (select(sd + 1, &readfds, nullptr, nullptr, &tv))
                                 {

                                         char message_received[100];
                                         bzero(message_received, 100);

                                         if (read(sd, message_received, sizeof(message_received)) == -1)
                                         {
                                             error_message->setText("Eroare la read()");
                                             error_message->setStyleSheet("color: purple; ");
                                         }
                                         message_received[strlen(message_received)] = '\0';

                                         if (strcmp(message_received, "connected") == 0)
                                         {
                                            user_name = temp_username;
                                            waitLoop.quit();
                                            loginwindow->close();
                                         }
                                         else
                                         {
                                            QString temp_text = QString::fromStdString(string(message_received));
                                             error_message->setText(temp_text);
                                             error_message->setStyleSheet("color: red; ");
                                         }
                                 }
                             }
                         } });

    QObject::connect(register_button, &QPushButton::clicked, [&]()
                     {
                         if (!verify_page)
                         {
                             error_message->setText("");
                             verify_page = 1;
                             QLayout *old_layout = loginwindow->layout();
                             clearLayout(old_layout);
                             delete old_layout;

                             insert_password->clear();
                             insert_username->clear();

                             verify_pass->show();
                             insert_verify_pass->show();
                             QHBoxLayout *user_layout = new QHBoxLayout();
                             QHBoxLayout *pass_layout = new QHBoxLayout();
                             QHBoxLayout *verify_layout = new QHBoxLayout();
                             QHBoxLayout *bttns_layout = new QHBoxLayout();
                             QHBoxLayout *error_ord = new QHBoxLayout();
                             QVBoxLayout *new_layout = new QVBoxLayout();

                             user_layout->addWidget(username);
                             user_layout->addWidget(insert_username);

                             pass_layout->addWidget(password);
                             pass_layout->addSpacing(2);
                             pass_layout->addWidget(insert_password);

                             verify_layout->addWidget(verify_pass);
                             verify_layout->addWidget(insert_verify_pass);

                             bttns_layout->addWidget(login_button);
                             bttns_layout->addWidget(register_button);

                             error_ord->addStretch();
                             error_ord->addWidget(error_message);
                             error_ord->addStretch();

                             new_layout->addSpacing(30);
                             new_layout->addLayout(user_layout);
                             new_layout->addSpacing(10);
                             new_layout->addLayout(pass_layout);
                             new_layout->addSpacing(10);
                             new_layout->addLayout(verify_layout);
                             new_layout->addSpacing(30);
                             new_layout->addLayout(bttns_layout);
                             new_layout->addSpacing(10);
                             new_layout->addLayout(error_ord);

                             loginwindow->setLayout(new_layout);
                         }
                         else
                         {
                             string message_to_send = "^";
                             QString username_text = insert_username->text();
                             QString password_text = insert_password->text();
                             QString repassword_text = insert_verify_pass->text();

                             if (username_text.isEmpty() || password_text.isEmpty() || repassword_text.isEmpty())
                             {
                                 error_message->setText("Missing information!");
                                 error_message->setStyleSheet("color: red; ");
                             }
                             else
                             {
                                 string temp_user = username_text.toStdString();
                                 string temp_pass = password_text.toStdString();
                                 string temp_repass = repassword_text.toStdString();

                                 message_to_send += temp_user;
                                 message_to_send += "^";
                                 message_to_send += temp_pass;
                                 message_to_send += "^";
                                 message_to_send += temp_repass;
                                 message_to_send.push_back('\0');

                                 if (write(sd, message_to_send.c_str(), message_to_send.size()) == -1)
                                 {
                                     error_message->setText("Eroare la write()");
                                     error_message->setStyleSheet("color: purple; ");
                                 }

                                 fd_set readfds;
                                 struct timeval tv;

                                 tv.tv_sec = 1;
                                 tv.tv_usec = 0;

                                 FD_ZERO(&readfds);
                                 FD_SET(sd, &readfds);

                                 if (select(sd + 1, &readfds, nullptr, nullptr, &tv))
                                 {
                                         char message_received[100];
                                         bzero(message_received, 100);

                                         if (read(sd, message_received, sizeof(message_received)) == -1)
                                         {
                                             error_message->setText("Eroare la read()");
                                             error_message->setStyleSheet("color: purple; ");
                                         }
                                         message_received[strlen(message_received)] = '\0';
                                         if (strcmp(message_received, "Account created succesfully!") == 0)
                                         {
                                             error_message->setText(message_received);
                                             error_message->setStyleSheet("color: green; ");
                                             usleep(1000);
                                             insert_password->clear();
                                             insert_username->clear();
                                             insert_verify_pass->clear();
                                         }
                                         else
                                         {
                                             error_message->setText(message_received);
                                             error_message->setStyleSheet("color: red; ");
                                         }
                                     
                                 }
                             }
                         } });
    loginwindow->show();
    waitLoop.exec();
}

int main(int argc, char *argv[])
{
    int port, sd;
    struct sockaddr_in server;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    QApplication app(argc, argv);
    connectWindow(app, port, sd, server);
    loginWindow(app, sd);

    QWidget mainWindow;
    mainWindow.setWindowTitle("StudyBuddy");
    mainWindow.resize(1600, 900);

    static QTimer *timer_user_table = nullptr;
    static QTimer *timer_notifications = nullptr;
    static QTimer *new_online_users_timer = nullptr;
    static QTimer *new_verify_messages = nullptr;
    static QTimer *timer_files = nullptr;

    QVBoxLayout *layout_main = new QVBoxLayout();
    QHBoxLayout *layout_buttons = new QHBoxLayout();
    QPushButton *chatRoom = new QPushButton("Chat Room");
    QPushButton *fileSharing = new QPushButton("Files");
    QPushButton *liveCooperation = new QPushButton("Live working");
    layout_buttons->addWidget(chatRoom);
    layout_buttons->addWidget(fileSharing);
    layout_buttons->addWidget(liveCooperation);

    layout_main->addLayout(layout_buttons);
    layout_main->addStretch();
    mainWindow.setLayout(layout_main);
    mainWindow.show();

    QObject::connect(fileSharing, &QPushButton::clicked, [&]()
                     {  
                        string command = "+" + user_name;
                        if(write(sd, command.c_str(), command.size() + 1) == -1)
                        {
                            perror("Eroare la write()");
                        }
                        if(new_verify_messages)
                            new_verify_messages->stop();
                        if(new_online_users_timer)
                            new_online_users_timer->stop();
                        if(timer_user_table)
                            timer_user_table->stop();
                        if(timer_notifications)
                            timer_notifications->stop();
                        if(timer_files)
                            timer_files->stop();
                        static QListWidget *fileList = nullptr;
                         QLayout *oldLayout = mainWindow.layout();
                         clearLayout(oldLayout);
                         delete oldLayout;

                         QHBoxLayout *files_layout = new QHBoxLayout();
                         QVBoxLayout *new_layout = new QVBoxLayout();

                         fileList = new QListWidget;
                         openFileList(*fileList, sd);
                         
                         int cntfiles = fileList->count();
                         fileList->setGridSize(QSize(fileList->sizeHintForColumn(0), 25));
                         fileList->setSpacing(0);
                         if (cntfiles <= 35)
                         {
                             fileList->setFixedHeight(cntfiles * 25);
                             fileList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                             fileList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                         }
                         else
                         {
                             fileList->setFixedHeight(35 * 25);
                             fileList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                         }

                         layout_buttons = new QHBoxLayout();
                         layout_buttons->addWidget(chatRoom);
                         layout_buttons->addWidget(fileSharing);
                         layout_buttons->addWidget(liveCooperation);

                         QLabel *quickMessage = new QLabel("Downloading...");
                         quickMessage->hide();

                         QHBoxLayout *buttons_files_layout = new QHBoxLayout();
                         QPushButton *upload_button = new QPushButton("Upload File");
                         QPushButton *download_button = new QPushButton("Download file");

                         buttons_files_layout->addWidget(upload_button);
                         buttons_files_layout->addWidget(download_button);

                         QVBoxLayout *file_box_layout = new QVBoxLayout();
                         file_box_layout->addWidget(fileList);
                         file_box_layout->addStretch();
                         file_box_layout->addWidget(quickMessage);
                         file_box_layout->addLayout(buttons_files_layout);

                         files_layout->addLayout(file_box_layout);

                         new_layout->addLayout(layout_buttons);
                         new_layout->addLayout(files_layout);
                         mainWindow.setLayout(new_layout);

                         timer_files = new QTimer(&mainWindow);
                         timer_files->start(5000);

                         QObject::connect(timer_files, &QTimer::timeout, [&](){

                            string message_to_send = "filelist";
                            if(write(sd, message_to_send.c_str(), sizeof(message_to_send)) == -1)
                            {
                                perror("Eroare la write()");
                            }
                            fd_set readfds;
                            struct timeval tv;

                            tv.tv_sec = 5;
                            tv.tv_usec = 0;

                            FD_ZERO(&readfds);
                            FD_SET(sd, &readfds);


                            char filenames[10000];
                            bzero(filenames, 10000);
                            if(select(sd+1, &readfds, nullptr, nullptr, &tv))
                            {
                                if(read(sd, &filenames, sizeof(filenames)) == -1)
                                {
                                    perror("Eroare la read()");
                                }
                            }

                            vector<string> file_names_separated;
                            string name = "";
                            for(int i = 0 ; i <strlen(filenames) ; i++)
                            {
                                if(filenames[i] == ';')
                                {
                                    file_names_separated.push_back(name);
                                    name = "";
                                }
                                else
                                {
                                    name+=filenames[i];
                                }
                            }

                            for(auto i : file_names_separated)
                            {
                                QString temp = QString::fromStdString(i);
                                bool to_add = true;
                                for(int j = 0 ; j < fileList->count();j++)
                                {
                                    QListWidgetItem *item = fileList->item(j);
                                    QString item_name = item->text();
                                    if(temp == item_name)
                                        to_add = false;
                                }
                                if(to_add)
                                    fileList->addItem(temp);
                            }

                            int cntfiles = fileList->count();
                            fileList->setGridSize(QSize(fileList->sizeHintForColumn(0), 25));
                            fileList->setSpacing(0);
                            if (cntfiles <= 35)
                            {
                                fileList->setFixedHeight(cntfiles * 25);
                                fileList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                                fileList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                            }
                            else
                            {
                                fileList->setFixedHeight(35 * 25);
                                fileList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                            }

                            timer_files->stop();
                            timer_files->start(5000);

                         });

                         QObject::connect(fileList, &QListWidget::itemClicked, [&](QListWidgetItem *item)
                                          {
                                            
                            file_name.clear();
                            QString temp_name = item->text();
                            file_name = temp_name.toStdString();
                            });  
                         QObject::connect(download_button, &QPushButton::clicked, [&]()
                                          {
                                            timer_files->stop();
                                              string message_to_send = "!";
                                              if (file_name.empty())
                                              {
                                                  return;
                                              }
                                              else
                                              {
                                                  message_to_send += file_name;
                                              }
                                              if (write(sd, message_to_send.c_str(), sizeof(message_to_send)) == -1)
                                              {
                                                  perror("Eroare la write()");
                                              }

                                              fd_set readfds;
                                              struct timeval tv;

                                              tv.tv_sec = 5;
                                              tv.tv_usec = 0;

                                              FD_ZERO(&readfds);
                                              FD_SET(sd, &readfds);

                                              string new_file_path = "/home/razvan/Downloads/" + file_name;
                                              FILE *file = fopen(new_file_path.c_str(), "wb");

                                              if(!file)
                                                {
                                                    perror("Eroare la deschierea fisierului!");
                                                }

                                                size_t file_size;
                                                
                                                if (select(sd + 1, &readfds, nullptr, nullptr, &tv))
                                                { 
                                                   if(read(sd, &file_size, sizeof(file_size)) == -1)
                                                   {
                                                        perror("Eroare la read()");
                                                        fclose(file);
                                                   }

                                                   cout << file_size << '\n';

                                                   char buffer[1024];
                                                   size_t current_bytes_received = 0;

                                                   while(current_bytes_received < file_size)
                                                   {
                                                        size_t bytes_read = read(sd, &buffer, sizeof(buffer));
                                                        if(bytes_read == -1)
                                                        {
                                                            perror("Eroare la read()");
                                                        }
                                                        fwrite(buffer, 1, bytes_read, file);
                                                        current_bytes_received += bytes_read;
                                                        cout << current_bytes_received  << '/' << file_size << '\n';
                                                   }

                                                }   
                                                fclose(file);  
                                                timer_files->start(5000);                                  
                                          }); 
                        
                        QObject::connect(upload_button, &QPushButton::clicked, [&]()
                        {
                            
                            QDialog *send_path = new QDialog();
                            send_path->setWindowTitle("Insert path to file");
                            send_path->resize(500, 200);

                            QLabel *temp_text = new QLabel("Please insert the path to the file you want to send:");
                            temp_text->setAlignment(Qt::AlignCenter);
                            temp_text->setFixedHeight(45);
                            temp_text->setStyleSheet("font-size: 17pt; ");
                            QLineEdit *insert_path = new QLineEdit();
                            QPushButton *submit = new QPushButton("Submit path");

                            QHBoxLayout *line_layout = new QHBoxLayout();
                            line_layout->addWidget(insert_path);
                            line_layout->addWidget(submit);

                            QVBoxLayout *window_layout = new QVBoxLayout();
                            window_layout->addStretch();
                            window_layout->addWidget(temp_text);
                            window_layout->addLayout(line_layout);
                            window_layout->addStretch();


                            send_path->setLayout(window_layout);
                            

                            QObject::connect(submit, &QPushButton::clicked, [&](){

                                    string file_path = insert_path->text().toStdString();
                                    string file_name_temp = "";
                                    string temp_file_path = file_path.substr(0, file_path.size() - 1);
                                    for(auto i : file_path)
                                    {
                                        if(i == '/')
                                        {
                                            file_name_temp = "";
                                        }else 
                                        {
                                            file_name_temp += i;
                                        }
                                    }
                                    string to_send = "<" + file_name_temp;
                                    if(write(sd, to_send.c_str(), to_send.size()) == -1)
                                    {
                                        perror("Eroare la write()");
                                    }

                                    FILE* file = fopen(file_path.c_str(), "rb");
                                    if(!file)
                                    {
                                        perror("Eroare la deschierea fisierului!");
                                    }

                                    fseek(file, 0, SEEK_END);
                                    size_t file_size = ftell(file);
                                    rewind(file);

                                    if(write(sd, &file_size, sizeof(file_size)) == -1)
                                    {
                                        perror("Eroare la write()");
    
                                    }

                                    char buffer[1024];
                                    size_t current_bytes_sent = 0;
                                    while(current_bytes_sent < file_size)
                                        {
                                            unsigned long tmp = 1024;
                                            size_t remaining_bytes = min(tmp, file_size - current_bytes_sent);
                                            fread(buffer, 1, remaining_bytes, file);
                                            if(write(sd, buffer, remaining_bytes) == -1)
                                                {
                                                    perror("Eroare la write()");
                                                }
                                            current_bytes_sent += remaining_bytes;
                                            cout << current_bytes_sent  << '/' << file_size << '\n';
                                        }
                                    fclose(file);
                                    send_path->accept();
                            });
                            send_path->exec();
                            
                        }); });

    QObject::connect(chatRoom, &QPushButton::clicked, [&]()
                     {
                        string command = "+" + user_name;
                        if(write(sd, command.c_str(), command.size() + 1) == -1)
                        {
                            perror("Eroare la write()");
                        }
                        if(new_verify_messages)
                            new_verify_messages->stop();
                        if(new_online_users_timer)
                            new_online_users_timer->stop();
                        if(timer_user_table)
                            timer_user_table->stop();
                        if(timer_notifications)
                            timer_notifications->stop();
                        if(timer_files)
                            timer_files->stop();
                         QLayout *oldLayout = mainWindow.layout();
                         clearLayout(oldLayout);
                         delete oldLayout;
                         
                         QVBoxLayout *new_layout = new QVBoxLayout();

                        layout_buttons = new QHBoxLayout();
                        layout_buttons->addWidget(chatRoom);
                        layout_buttons->addWidget(fileSharing);
                        layout_buttons->addWidget(liveCooperation);

                        QVBoxLayout *chat_layout = new QVBoxLayout();
                        static QTextEdit *messageBox = nullptr;
                        messageBox = new QTextEdit();
                        messageBox->setReadOnly(true);
                        messageBox->setFixedHeight(850);

                            QHBoxLayout *layout_textMessage = new QHBoxLayout();
                            static QLineEdit *messageInput = nullptr;
                            messageInput = new QLineEdit();
                            messageInput->setFixedWidth(1200);
                            messageInput->setFixedHeight(50);
                            messageInput->setStyleSheet("font-size: 14pt;");

                            QPushButton *messageSend = new QPushButton("Send");
                            messageSend->setFixedWidth(150);
                            messageSend->setFixedHeight(50);
                            layout_textMessage->addWidget(messageInput);
                            layout_textMessage->addWidget(messageSend);
                            chat_layout->addSpacing(20);
                            chat_layout->addWidget(messageBox);
                            chat_layout->addStretch();
                            chat_layout->addLayout(layout_textMessage);

                            QVBoxLayout *layout_activeUsers = new QVBoxLayout();
                            static QTableWidget *active_usersTable = nullptr;
                            static QLabel *active_usersTitle = nullptr;
                            active_usersTitle = new QLabel("Active Users: 0");
                            active_usersTitle->setStyleSheet("color: green; ");
                            layout_activeUsers->addWidget(active_usersTitle);
                            active_usersTable = new QTableWidget(0, 1);
                            active_usersTable->verticalHeader()->hide();
                            active_usersTable->horizontalHeader()->hide();
                            active_usersTable->setSelectionMode(QAbstractItemView::NoSelection);
                            active_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
                            active_usersTable->setFocusPolicy(Qt::NoFocus);
                            active_usersTable->setDragEnabled(false);
                            active_usersTable->setAcceptDrops(false);
                            active_usersTable->setColumnWidth(0, 200);
                            active_usersTable->setFixedWidth(200);
                            layout_activeUsers->addWidget(active_usersTable);


                         QHBoxLayout *random_layout = new QHBoxLayout();
                         random_layout->addLayout(layout_activeUsers);
                         random_layout->addLayout(chat_layout);

                         new_layout->addLayout(layout_buttons);
                         new_layout->addLayout(random_layout);
                         mainWindow.setLayout(new_layout); 

                         new_online_users_timer = new QTimer(&mainWindow);
                         new_verify_messages = new QTimer(&mainWindow);

                         new_online_users_timer->start(3000);
                         new_verify_messages->start(1000);
                         
                          QObject::connect(new_online_users_timer, &QTimer::timeout, [&]()
                                {    
                         char users_command[100] = "numberusers";
                         char usernames[100000];
                         if (write(sd, users_command, strlen(users_command)) == -1)
                         {
                             printf("Eroare la write()!");
                         }
                         bzero(usernames, sizeof(usernames));

                         fd_set readfds;
                         struct timeval tv;

                         tv.tv_sec = 1;
                         tv.tv_usec = 0;

                         FD_ZERO(&readfds);
                         FD_SET(sd, &readfds);

                         if(select(sd+1, &readfds, nullptr, nullptr, &tv))
                         {
                            if (read(sd, usernames, sizeof(usernames)) == -1)
                         {
                             printf("Eroare la read()!");
                         }
                         }
                         usernames[strlen(usernames)] = '\0';
                         vector<string> all_usernames;
                         string temp_name = "";
                         for(int i = 0; i < strlen(usernames); i++ )
                         {
                            if(usernames[i] == ';')
                            {
                                all_usernames.push_back(temp_name);
                                temp_name = "";
                            }
                            else 
                            {
                                temp_name += usernames[i];
                            }
                         }
                         printf("%d\n", all_usernames.size());
                         active_usersTable->setRowCount(all_usernames.size());
                         string temp = "Active users: ";
                         temp += to_string(all_usernames.size());
                         active_usersTitle->setText(QString::fromStdString(temp));
                         for (int i = 0; i < all_usernames.size(); i++)
                            {
                                QString username = QString::fromStdString(all_usernames[i]);
                                active_usersTable->setItem(i, 0, new QTableWidgetItem(username));
                            } 
                        new_online_users_timer->stop();
                        new_online_users_timer->start(3000);

                    });


                        int time_elapsed = 0;
                        int time_interval = 1500;
                         QObject::connect(new_verify_messages, &QTimer::timeout, [&]()
                     {
                        if(new_online_users_timer->isActive())
                        {
                            time_elapsed = time_interval - new_online_users_timer->remainingTime();
                            new_online_users_timer->stop();
                        }

                        string message_to_send = "chatlog";
                        if(write(sd, message_to_send.c_str(), message_to_send.size() + 1) == -1)
                        {
                            perror("Eroare la write()");
                        }
                        
                        fd_set readfds;
                        struct timeval tv;
                        tv.tv_sec = 1;
                        tv.tv_usec = 0; 

                        FD_ZERO(&readfds);
                        FD_SET(sd, &readfds);

                        if(select(sd+1, &readfds, nullptr, nullptr, &tv))
                            {
                                char chatlog[100000];
                                bzero(chatlog, sizeof(chatlog));
                                if (read(sd, chatlog, sizeof(chatlog)) == -1)
                                    {
                                        perror("Eroare la read()");
                                    }
                                chatlog[strlen(chatlog)] = '\0';

                                messageBox->clear();
                                string temp_message_string = "";
                                for(int i = 0; i < strlen(chatlog); i++)
                                    {
                                        if(chatlog[i] == '~')
                                            {
                                                QString temp_message = QString::fromStdString(temp_message_string);
                                                messageBox->append(temp_message);
                                                temp_message_string = "";
                                            }
                                            else 
                                            {
                                                temp_message_string += chatlog[i];
                                            }
                                    }
                            } 
                        int time_remaining = time_interval - time_elapsed;
                        if(time_remaining > 0)
                            new_online_users_timer->start(time_remaining);
                        else 
                            new_online_users_timer->start(time_interval); 

                            new_verify_messages->stop();
                            new_verify_messages->start(1000);
                            });
                        

                        QObject::connect(messageSend, &QPushButton::clicked, [&]()
                     {
                        new_online_users_timer->stop();
                        new_verify_messages->stop();
                         QString temp_message = messageInput->text();
                         string message_to_send = temp_message.toStdString();
                        
                         if (write(sd, message_to_send.c_str(), message_to_send.size()) == -1)
                         {
                             printf("Eroare la write()");
                             return errno;
                         }
                         messageInput->clear(); 
                         new_online_users_timer->start(3000);
                         new_verify_messages->start(1000);
                         }); });

    QObject::connect(liveCooperation, &QPushButton::clicked, [&]()
                     {
                        if(new_verify_messages)
                            new_verify_messages->stop();
                        if(new_online_users_timer)
                            new_online_users_timer->stop();
                        if(timer_user_table)
                            timer_user_table->stop();
                        if(timer_notifications)
                            timer_notifications->stop();
                        if(timer_files)
                            timer_files->stop();
                        
                        QLayout *oldLayout = mainWindow.layout();
                        clearLayout(oldLayout);
                        delete oldLayout;
                        file_name_shared.clear();

                        static QListWidget *fileList_shared = nullptr;
                        static QLabel *users_on_file_title = nullptr;
                        static QTableWidget *users_on_file = nullptr;

                        
                        fileList_shared = new QListWidget();

                        static map<string, vector<string>> users_map;

                        openFileList(*fileList_shared, sd);

                        QHBoxLayout* new_buttons_layout = new QHBoxLayout();
                        new_buttons_layout->addWidget(chatRoom);
                        new_buttons_layout->addWidget(fileSharing);
                        new_buttons_layout->addWidget(liveCooperation);

                        QVBoxLayout *two_tables_layout = new QVBoxLayout();
                        QHBoxLayout *all_tables_layout = new QHBoxLayout();
                        QVBoxLayout *new_final_layout = new QVBoxLayout();

                        users_on_file = new QTableWidget(0, 1);
                        
                        users_on_file_title= new QLabel("Users viewing file: ");
                        users_on_file->verticalHeader()->hide();
                        users_on_file->horizontalHeader()->hide();
                        users_on_file->setSelectionMode(QAbstractItemView::NoSelection);
                        users_on_file->setEditTriggers(QAbstractItemView::NoEditTriggers);
                        users_on_file->setFocusPolicy(Qt::NoFocus);
                        users_on_file->setDragEnabled(false);
                        users_on_file->setAcceptDrops(false);
                        static QTextEdit *notifications = nullptr;
                        notifications = new QTextEdit();
                        notifications->setReadOnly(true);
                        QPushButton *sync_button = new QPushButton("sync");
                        QPushButton *pull_button = new QPushButton("pull");
                        QVBoxLayout *just_for_sync_button_layout = new QVBoxLayout();
                        QHBoxLayout *more_buttons_yey_layout = new QHBoxLayout();
                        more_buttons_yey_layout->addWidget(sync_button);
                        more_buttons_yey_layout->addWidget(pull_button);
                        just_for_sync_button_layout->addWidget(notifications);
                        just_for_sync_button_layout->addLayout(more_buttons_yey_layout);

                        two_tables_layout->addWidget(fileList_shared);
                        two_tables_layout->addWidget(users_on_file_title);
                        two_tables_layout->addWidget(users_on_file);

                        all_tables_layout->addLayout(just_for_sync_button_layout);
                        all_tables_layout->addLayout(two_tables_layout);

                        new_final_layout->addLayout(new_buttons_layout);
                        new_final_layout->addLayout(all_tables_layout);
                        mainWindow.setLayout(new_final_layout); 

                        timer_user_table = new QTimer(&mainWindow);
                        timer_notifications = new QTimer(&mainWindow);
                        timer_user_table->start(3000);
                        timer_notifications->start(1000);

                        QObject::connect(sync_button, &QPushButton::clicked, [&](){
                            timer_notifications->stop();
                            timer_user_table->stop();
                            if(!file_name_shared.empty())
                            {
                                string message_to_send = "?" + file_name_shared;
                            if(write(sd, message_to_send.c_str(), message_to_send.size()) == -1)
                            {
                                perror("Eroare la write()");
                            }

                            string folder = "liveCooperation_" + user_name + "/";
                            string file_name_path = "/home/razvan/Desktop/" + folder + file_name_shared;
                            FILE* file = fopen(file_name_path.c_str(),"rb");

                            if(!file)
                            {
                                perror("Eroare la deschiderea fisierului");
                            }

                            fseek(file, 0, SEEK_END);
                            size_t file_size = ftell(file);
                            rewind(file);

                            if(write(sd, &file_size, sizeof(file_size)) == -1)
                            {
                                perror("Eroare la write()");
                            }

                            char buffer[1024];
                            size_t current_bytes_sent = 0;
                            while(current_bytes_sent < file_size)
                                {
                                    unsigned long tmp = 1024;
                                    size_t remaining_bytes = min(tmp, file_size - current_bytes_sent);
                                    fread(buffer, 1, remaining_bytes, file);

                                    if(write(sd, buffer, remaining_bytes) == -1)
                                        {
                                            perror("Eroare la write()");
                                        }
                                    current_bytes_sent += remaining_bytes;
                                    cout << current_bytes_sent  << '/' << file_size << '\n';
                                }

                            fd_set readfds;
                            struct timeval tv;

                            tv.tv_sec = 1;
                            tv.tv_usec = 0;

                            FD_ZERO(&readfds);
                            FD_SET(sd, &readfds);

                            char msg_buffer[1000];
                            bzero(msg_buffer, sizeof(buffer));
                            if(select(sd + 1, &readfds, nullptr, nullptr, &tv))
                            {
                                if(read(sd, &msg_buffer, sizeof(msg_buffer)) == -1)
                                    {
                                        perror("Eroare la read()");
                                    }
                                msg_buffer[strlen(msg_buffer)] = '\0';
                            }

                            if(strcmp(msg_buffer, "nothing!"))
                            {
                                string message_to_send_to_all = "/" + string(msg_buffer);
                                if(write(sd, message_to_send_to_all.c_str(), message_to_send_to_all.size() + 1) == -1)
                                    {
                                        perror("Eroare la write()");
                                    }
                            }
                            timer_user_table->start(3000);
                            timer_notifications->start(1000); 
                            }
                            timer_user_table->start(3000);
                            timer_notifications->start(1000); 
                            
                        });

                        QObject::connect(pull_button, &QPushButton::clicked, [&](){
                            timer_notifications->stop();
                            timer_user_table->stop();
                            string message_to_send = "!" + file_name_shared;
                            if(write(sd, message_to_send.c_str(), message_to_send.size() + 1) == -1)
                            {
                                perror("Eroare la write()");
                            }

                            fd_set readfds;
                            struct timeval tv;
                            tv.tv_sec = 5;
                            tv.tv_usec = 0;

                            FD_ZERO(&readfds);
                            FD_SET(sd, &readfds);

                            string folder = "liveCooperation_" + user_name + "/";
                            string file_name_path = "/home/razvan/Desktop/" + folder + file_name_shared; 
                            FILE* file = fopen(file_name_path.c_str(), "wb");
                            size_t file_size;
                                                
                            if (select(sd + 1, &readfds, nullptr, nullptr, &tv))
                                { 
                                    if(read(sd, &file_size, sizeof(file_size)) == -1)
                                        {
                                            perror("Eroare la read()");
                                            fclose(file);
                                        }

                                        cout << file_size << '\n';
                                        char buffer[1024];
                                        size_t current_bytes_received = 0;

                                        while(current_bytes_received < file_size)
                                            {
                                                size_t bytes_read = read(sd, &buffer, sizeof(buffer));
                                                if(bytes_read == -1)
                                                    {
                                                        perror("Eroare la read()");
                                                    }
                                                fwrite(buffer, 1, bytes_read, file);
                                                current_bytes_received += bytes_read;
                                                cout << current_bytes_received  << '/' << file_size << '\n';
                                            }
                                        
                                        fclose(file);
                                                   
                                }   
                                    timer_user_table->start(3000);
                                    timer_notifications->start(1000);
                        });


                        QObject::connect(fileList_shared, &QListWidget::itemClicked, [&](QListWidgetItem *item){
                            notifications->clear();
                            QString temp = item->text();
                            string temp_file_name = temp.toStdString();
                            file_name_shared = temp_file_name;
                            string new_label = "Users viewing file: " + temp_file_name;
                            users_on_file_title->setText(QString::fromStdString(new_label));
                            
                            string message_to_send = "(" + temp_file_name + ";" + user_name;
                            if(write(sd, message_to_send.c_str(), message_to_send.size()) == -1)
                            {
                                perror("Eroare la write()");
                            }

                            fd_set readfds;
                            struct timeval tv;

                            tv.tv_sec = 1;
                            tv.tv_usec = 0;

                            FD_ZERO(&readfds);
                            FD_SET(sd, &readfds);

                            if(select(sd +1, &readfds, nullptr, nullptr, &tv))
                            {
                                char buffer[1000];
                                bzero(buffer, sizeof(buffer));

                                if(read(sd, &buffer, sizeof(buffer)) == -1)
                                {
                                    perror("Eroare la read()");
                                }
                                string temp_username = "";
                                users_map[temp_file_name].clear();
                                for(int i = 0; i<strlen(buffer);i++)
                                {
                                    if(buffer[i] == ';')
                                    {
                                        users_map[temp_file_name].push_back(temp_username);
                                        temp_username = "";
                                    }
                                    else 
                                    {
                                        temp_username += buffer[i];
                                    }
                                }


                            }
                           
                        });

                        int time_elapsed = 0;
                        int time_interval = 3000;
                        QObject::connect(timer_notifications, &QTimer::timeout, [&](){
                            if(timer_user_table->isActive())
                                {
                                    time_elapsed = time_interval - timer_user_table->remainingTime();
                                    timer_user_table->stop();
                                }
                            fd_set readfds;
                            struct timeval tv;

                            tv.tv_sec = 0;
                            tv.tv_usec = 100;

                            FD_ZERO(&readfds);
                            FD_SET(sd, &readfds);

                            if(select(sd + 1, &readfds, nullptr, nullptr, &tv))
                            {
                                char buffer[1000];
                                bzero(buffer, sizeof(buffer));
                                if(read(sd, &buffer, sizeof(buffer)) == -1)
                                {
                                    perror("Eroare la read()");
                                }
                                buffer[strlen(buffer)] = '\0';
                                if(strcmp(buffer, ""))
                                {
                                    QString temp = QString::fromStdString(string(buffer));
                                    notifications->append(temp);
                                }
                                
                            }

                            int time_remaining = time_interval - time_elapsed;
                            if(time_remaining > 0)
                                timer_user_table->start(time_remaining);
                            else 
                                timer_user_table->start(time_interval); 
                            timer_notifications->stop();
                            timer_notifications->start(1000);    
                        });

                        QObject::connect(timer_user_table, &QTimer::timeout, [&](){
                            cout << "Intra in timer_user_table" << '\n';
                            if(file_name_shared.empty())
                                {
                                    timer_user_table->stop();
                                    timer_user_table->start(3000);
                                    return;
                                }
                            string message_to_send = ")" + file_name_shared;
                            if(write(sd, message_to_send.c_str(), sizeof(message_to_send)) == -1)
                            {
                                perror("Eroare la write()");
                            }

                            fd_set readfds;
                            struct timeval tv;
                            tv.tv_sec = 5;
                            tv.tv_usec = 0;

                            FD_ZERO(&readfds);
                            FD_SET(sd, &readfds);

                            if(select(sd+1, &readfds, nullptr, nullptr, &tv))
                            {
                                char buffer[1000];
                                bzero(buffer, sizeof(buffer));

                                if(read(sd, &buffer, sizeof(buffer)) == -1)
                                {
                                    perror("Eroare la read()");
                                }
                                string temp_user_name = "";
                                users_map[file_name_shared].clear();
                                for(int i = 0; i < strlen(buffer);  i++)
                                {
                                    if(buffer[i] == ';')
                                    {
                                        users_map[file_name_shared].push_back(temp_user_name);
                                        temp_user_name = "";
                                    }
                                    else
                                    {
                                        temp_user_name += buffer[i];
                                    }
                                }
                            }
                            users_on_file->setRowCount(users_map[file_name_shared].size());
                            users_on_file->horizontalHeader()->setStretchLastSection(true);
                            for(int i = 0; i < users_map[file_name_shared].size(); i++)
                                {
                                    QString username = QString::fromStdString(users_map[file_name_shared][i]);
                                    users_on_file->setItem(i, 0, new QTableWidgetItem(username));
                                }
                                timer_user_table->stop();
                                timer_user_table->start(3000);
                        }); });

    QObject::connect(&app, &QApplication::aboutToQuit, [&]()
                     {
        string message_to_send = "quit";
        if(write(sd, message_to_send.c_str(), message_to_send.size() + 1) == -1)
        {
            perror("Eroare la write");
        } });

    return app.exec();
}