#include "imports.hh"
#include "RSA.hh"
using namespace std;

struct User
{
    string email;
    string password;
    int fd;
    string e_key;
    string d_key;
    string n_key;
};
map<string, User> users;

struct Poll
{
    string Qn_optns;
    map<string, int> poll_status;
};
vector<Poll> polls;

bool signup(string email, string password, int nsfd)
{
    srand(time(0));
    string otp = "";
    for (int i = 0; i < 8; ++i)
    {
        otp.push_back('0' + (rand() % 10));
    }

    string response = mailSender(email, otp);

    if (send(nsfd, response.c_str(), response.size(), 0) < 0)
        pthread_exit(NULL);

    if (response.substr(0, 5) != "Check")
        response = "error";

    char buffer[1024] = {0};
    recv(nsfd, buffer, 1024, 0);

    string recvdOTP(buffer);
    cout << recvdOTP << endl;
    if (recvdOTP == otp)
    {
        users[email] = {email, password, -1};
        return 1;
    }
    return 0;
}

bool signin(string email, string password)
{
    auto it = users.find(email);
    if (it != users.end() && it->second.password == password)
    {
        cout << "password recieved : " << password << endl;
        cout << "Actual password : " << it->second.password << endl;
        return true;
    }
    return false;
}

bool CheckPollStatus(struct Poll x)
{
    for (auto i : x.poll_status)
    {
        cout << i.first << " " << i.second << endl;
        if (i.second == -1)
            return false;
    }
    return true;
}

void PostPoll(int sfd)
{
    char buffer[1024];
    read(sfd, buffer, 1024);
    string PollQn(buffer);
    cout << "Received poll question: " << PollQn << endl;
    map<string, int> UsersTillNow;
    for (auto i : users)
    {
        if (i.second.fd != sfd)
        {
            UsersTillNow[i.first] = -1;
        }
    }
    cout << UsersTillNow.size() << endl;
    polls.push_back({PollQn, UsersTillNow});
}

string GetWinningOption(map<string, int> votes)
{
    string winningOption;
    vector<int> count(4, 0);
    for (auto it : votes)
    {
        if (it.second == 1)
            count[0]++;
        if (it.second == 2)
            count[1]++;
        if (it.second == 3)
            count[2]++;
        if (it.second == 4)
            count[3]++;
    }
    int index = max_element(count.begin(), count.end()) - count.begin();
    return to_string(index);
}

void SendPoll(int nsfd, string email)
{

    while (true)
    {
        bool flag = 0;
        for (auto &each_poll : polls)
        {
            if (each_poll.poll_status[email] == -1)
            {
                flag = 1;
                send(nsfd, each_poll.Qn_optns.c_str(), each_poll.Qn_optns.length(), 0);
                char answer_buffer[1024] = {0};
                read(nsfd, answer_buffer, 1024);
                string answer(answer_buffer);
                each_poll.poll_status[email] = stoi(answer);

                if (CheckPollStatus(each_poll))
                {
                    cout << "the poll " << each_poll.Qn_optns << " is answered by all the clients" << endl;
                    string winner = GetWinningOption(each_poll.poll_status);
                    cout << "The option that recieved most number of votes is :" << stoi(winner) + 1 << endl;
                }

                break;
            }
        }
        cout << flag << endl;
        if (flag == 0)
        {
            string res = "#";
            send(nsfd, res.c_str(), res.length(), 0);
        }
        char buffer[1024] = {0};
        read(nsfd, buffer, 1024);
        string command(buffer);
        if (command != "continue")
            break;
    }
}

void *clientHandler(void *args)
{
    int nsfd = *((int *)args);
    char buffer[1024] = {0};
    const char *response;

    // Read client request
    read(nsfd, buffer, 1024);
    string request(buffer);
    size_t pos = request.find(':');
    string command = request.substr(0, pos);
    string data = request.substr(pos + 1);
    pos = data.find(':');
    string email = data.substr(0, pos);
    string password = data.substr(pos + 1);
    bool res;

    if (command == "signup")
    {
        res = signup(email, password, nsfd);
        response = res ? "signup successful" : "Invalid otp";
        if (res)
            cout << "User signed-up " << email << " pswd : " << password << endl;
    }
    else if (command == "signin")
    {
        cout << "signin request from " << email << endl;
        res = signin(email, password);
        response = res ? "signin successful" : "Invalid email or password";
        if (response == "signin successful")
        {
            cout << "User signed-in " << email << " pswd : " << password << endl;
            
        }
    }
    else
    {
        response = "Invalid command";
    }
    cout<<"sending response"<<response<<endl;
    send(nsfd, response, strlen(response), 0);
    sleep(3);
    if (res)
    {
        mpz_t e,d,n;
        mpz_inits(e,d,n,NULL);
        generateKeys(e,d,n);
        string e_key=mpz_to_string(e);
        string d_key=mpz_to_string(n);
        string n_key=mpz_to_string(n);
        users[email].e_key=e_key;
        users[email].d_key=d_key;
        users[email].n_key=n_key;
        cout<<"e_RSA :"<<endl<<e_key<<endl;
        cout<<"n_RSA :"<<endl<<n_key<<endl;
        cout<<"d_RSA :"<<endl<<d_key<<endl;
        users[email].fd = nsfd;
        string key_exchange=users[email].e_key+":"+users[email].n_key+":";
        int x=send(nsfd,key_exchange.c_str(),key_exchange.length(),0);
    }
    else
        pthread_exit(NULL);

    while (1)
    {
        char command_buffer[1024] = {0};
        read(nsfd, command_buffer, 1024);
        string command(command_buffer);
        if (command == "logout")
        {
            users[email].fd = -1;
            close(nsfd);
            break;
        }
        else if (command == "PostPoll")
        {
            string response = "granted";
            send(nsfd, response.c_str(), response.size(), 0);
            PostPoll(nsfd);
        }
        else if (command == "AnswerPoll")
        {
            SendPoll(nsfd, email);
        }
    }

    return NULL;
}

int main()
{
    int sfd = Create_TCPSocket_server();
    while (true)
    {
        sockaddr_in address;
        int addrlen = sizeof(address);
        int nsfd = accept(sfd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        pthread_t th;
        pthread_create(&th, NULL, clientHandler, (void *)&nsfd);
    }
    close(sfd);
    return 0;
}