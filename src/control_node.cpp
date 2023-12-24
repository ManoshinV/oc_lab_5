// улучшение старого
#include <unistd.h>
#include <iostream>
#include <ctime>
#include "my_zmq.h"
#include "nnode.cpp"

using node_id_type = int;

void tokenize(std::vector<int> &args, const std::string &input) {
    std::stringstream ss(input);
    std::string s;
    while (std::getline(ss, s, ' ')) {
        args.push_back(std::stoi(s));
    }
}

int main() {
    Tree tr(0);
    std::string s;
    node_id_type id;
    std::pair<void *, void *> child;
    long long child_id = -1;
    bool first = 1;
    while (std::cin >> s) {
        if (s == "create") {
            int parent;
            std::cin>>id>>parent;
            ptr node = tr.get_ptr(id);
            if (node != nullptr) {
                std::cout << "Node with this id is already exist" << std::endl;
                continue;
            }
            if (first) { // если это первый вычислительный узел.
                first = 0;
                my_zmq::init_pair_socket(child.first, child.second);
                child_id = id;
                if (zmq_bind(child.second, ("tcp://*:" + std::to_string(PORT_BASE + id)).c_str()) != 0) {
                    perror("ZMQ_Bind");
                    exit(EXIT_FAILURE);
                }
                int pid = fork();
                if (pid == 0) { //son
                    execl(NODE_EXECUTABLE_NAME, NODE_EXECUTABLE_NAME, std::to_string(id).c_str(), nullptr);
                    perror("Execl");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) { // parent
                    tr.root->data = 0;
                } else { // error
                    perror("Fork");
                    exit(EXIT_FAILURE);
                }
            }  // если это не первый вычислительный узел
            if(!first) {
                auto *msg = new msg_t({create, 0, id});
                msg_t reply = *msg;
                my_zmq::send_receive_wait(msg, reply, child.second);
                tr.create(id, parent);  
            }    
            tr.print();
        } else if (s == "pingall") {
            int counter = 0;
            std::vector<int>nd;
            for(auto x: tr.ever) {
                if(tr.mp.find(x) == tr.mp.end()) {
                    nd.push_back(x);
                    ++counter;
                }
            }
            std::cout<<"OK: ";
            if(counter){
                for(int x : nd) {
                    std::cout << x<<";";
                }
            }
            std::cout<<std::endl;
        } else if (s == "exec") {
            std::cin>>id;
            if(tr.mp.find(id) != tr.mp.end()) {
                auto *terminate_msg = new msg_t({exec_add, -1, 0});
                int n = 2;
                std::string s;
                for (int i = 0; i < n; ++i){
                    std::cin >> s;
                    if(i == 0) s+="#";
                    for(char c : s) {
                        auto *msg_to_child = new msg_t({exec_add, c, 0});
                        my_zmq::send_msg_wait(msg_to_child, child.second);
                    }
                    
                }
                my_zmq::send_msg_wait(terminate_msg, child.second);
            } else {
                std::cout<<"NO node!"<< std::endl;
            }
        } else if (s == "remove") {
            std::cin>>id;
            std::pair<ptr, bool> res;
            if(tr.mp.find(id) == tr.mp.end()) {
                res = {nullptr, 0};
            } else {
                res = {tr.get_ptr(id), 1};
            }
            if (!res.second) {
                std::cout << "Error: Node with id " << id << " doesn't exists" << std::endl;
                continue;
            }
            tr.destroy(id);
            tr.print();
            if(tr.mp.find(id) != tr.mp.end()) {
                auto *msg = new msg_t({destroy, -1, id});
                if (res.first != nullptr) {
                    auto *msg = new msg_t({destroy, res.first->data, id});
                }
                msg_t reply = *msg;
                my_zmq::send_receive_wait(msg, reply, child.second);
                std::cout << "Node " << id << " was deleted" << std::endl;
            }
        } else {
            std::cout << "unknown command"<<std::endl;
            continue;
        }
    }
    std::cout << "Out tree:" << std::endl;
    tr.print();
    auto *msg = new msg_t({destroy, -1, tr.root->data});
    msg_t reply = *msg;
    my_zmq::send_receive_wait(msg, reply, child.second);
    zmq_close(child.second);
    zmq_ctx_destroy(child.first);
    return 0;
}
