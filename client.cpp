#include "imports.hh"
#include "RSA.hh"
using namespace std;
void AnswerPoll(int sfd)
{
    char buffer[1024];
    read(sfd, buffer, 1024);
    string PollQn(buffer);
    if (PollQn[0] == '#')
    {
        cout << "no questions to answer!!" << endl;
        return;
    }

    size_t pos = PollQn.find(':');
    string question = PollQn.substr(0, pos);
    string options = PollQn.substr(pos + 1);
    istringstream iss(options);
    string option1, option2, option3, option4;
    getline(iss, option1, ':');
    getline(iss, option2, ':');
    getline(iss, option3, ':');
    getline(iss, option4, ':');

    cout << "Please answer the following poll" << endl;
    cout << question << endl;
    cout << "The options are " << endl;
    cout << "1:" << option1 << endl;
    cout << "2:" << option2 << endl;
    cout << "3:" << option3 << endl;
    cout << "4:" << option4 << endl;
    cout << "enter your option 1 or 2 or 3 or 4" << endl;
    string option;
    cin >> option;

    send(sfd, option.c_str(), size(option), 0);
}

void PostPoll(int sfd)
{
    string question, option1, option2, option3, option4;

    cout << "Enter the poll question: ";
    cin.ignore();
    getline(cin, question);
    cout << "Enter the options one by one" << endl;
    cout << "Option 1: ";
    getline(cin, option1);
    cout << "Option 2: ";
    getline(cin, option2);
    cout << "Option 3: ";
    getline(cin, option3);
    cout << "Option 4: ";
    getline(cin, option4);

    string PollQn = question + ":" + option1 + ":" + option2 + ":" + option3 + ":" + option4 + ":";
    send(sfd, PollQn.c_str(), PollQn.length(), 0);
    cout << "Poll posted successfully!" << endl;
}

int main()
{
    string command;
    string email, password;
    string message;
    char buffer[1024] = {0};
    int port;
    string ip;

    port = 9000;
    ip = "127.0.0.1";
    int sfd = Create_TCPSocket_client(port, ip);
    cout << "Enter command (signup/signin/exit): ";
    cin >> command;
    if (command == "signup" || command == "signin")
    {
        cout << "Enter email: ";
        cin >> email;
        cout << "Enter password: ";
        cin >> password;
        string hashed_password = hashSHA256(password);
        cout << "Hashed Password : " << hashed_password << endl;
        message = command + ":" + email + ":" + hashed_password;
        int valread = send(sfd, message.c_str(), message.length(), 0);
        valread = recv(sfd, buffer, 1024, 0);
        buffer[valread] = '\0';
        string serverResponse(buffer);

        if (command == "signup")
        {
            cout << "Server response: " << serverResponse << endl;
            if (serverResponse.substr(0, 5) != "Check")
                exit(0);
            cout << "Enter otp : ";
            string otp;
            cin >> otp;
            send(sfd, otp.c_str(), otp.length(), 0);
            valread = recv(sfd, buffer, 1024, 0);
            buffer[valread] = '\0';
        }
        cout << "Server response: " << buffer << endl;
        string response(buffer);
        if (response == "Invalid email or password" || response == "Invalid otp")
            exit(0);


        char buffer[500000];
        read(sfd, buffer, 500000);
        string RSAkeys(buffer);
        string e_key,n_key;
        istringstream iss(RSAkeys);
        getline(iss, e_key, ':');
        getline(iss, n_key, ':');
        mpz_t e,n;
        mpz_init(e);
        mpz_init(n);
        string_to_mpz(e_key,e);
        string_to_mpz(n_key,n);
        cout<<"recieved RSA Keys from server"<<endl;
        cout<<"e_RSA :"<<endl<<e<<endl;
        cout<<"n_RSA :"<<endl<<n<<endl;
        


        while (true)
        {
            cout << "Enter command (PostPoll/AnswerPoll/logout): ";
            cin >> command;
            send(sfd, command.c_str(), command.length(), 0);

            if (command == "PostPoll")
            {
                char buffer[1024];
                read(sfd, buffer, 1024);
                string permission(buffer);
                cout << "Server Response : " << permission << endl;
                if (permission != "granted")
                    continue;
                PostPoll(sfd);
            }
            else if (command == "AnswerPoll")
            {
                while (true)
                {
                    AnswerPoll(sfd);
                    cout << "enter exit to stop answering and enter continue to continue answering" << endl;
                    string x;
                    cin >> x;
                    send(sfd, x.c_str(), x.length(), 0);
                    if (x == "exit")
                        break;
                }
            }
            else if (command == "logout")
            {

                close(sfd);
                break;
            }
            else
            {
                cout << "Invalid command" << endl;
            }
        }
    }
    else
    {
        cout << "Invalid command" << endl;
    }
    // Close the client socket
    return 0;
}