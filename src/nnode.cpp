#pragma once 
#include <iostream>
#include <vector>
#include <map>
#include <set>

struct node{
    int data;
    std::vector<node*>childs;
    node(int _data): data(_data){}
};
using ptr = node*;

struct Tree{
    ptr root = nullptr;
    std::map<int, ptr>mp;
    std::set<ptr>st;
    std::set<int>ever;
    Tree(int key) {
        root = new node(key);
        mp[key] = root;
        st.insert(root);
        ever.insert(key);
    } 
    void create(int id, int id_parent) {
        if(ever.find(id) != ever.end()) {
            std::cout << "Node with this id is already exist" << std::endl;
            return;
        }
        ever.insert(id);
        ptr parent = mp[id_parent];
        ptr child = new node(id);
        parent->childs.push_back(child);
        mp[id] = child;
        st.insert(child);
    }
    void destroy(int id){
        destroy(mp[id]);
    }
    void destroy(ptr u){
        if(u == nullptr) return;
        for(auto x:u->childs) 
            destroy(x);
        mp.erase(u->data);
        st.erase(u);
        delete u;
    }
    void print() {
        print_tree(root, 0);
        std::cout << std::endl;
    }
    void print_tree(ptr u, int k) {
        if(u == nullptr) 
            return;
        for(int i = 0; i < k; ++i) {
            std::cout<<' ';
        }
        std::cout << u->data << '\n';
        for(auto x:u->childs) 
            if(st.find(x) != st.end())
                print_tree(x, k + 1);
    }
    ptr get_ptr(int id) {
        return mp[id];
    }

};
