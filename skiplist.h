#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <list>
#define STORE_FILE "store/dumpFile"

std::mutex mtx;     
std::string delimiter = ":";

template<typename K, typename V> 
class Node {

public:
    
    Node() {} 

    Node(K k, V v, int); 

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);
    
    // Pointer array: hold pointers to next node of different level
    // forward[i]: next node pointed by the node at i-th level 
    Node<K, V> **forward;
    // Previous node in doubly linked list
    // Node<K, V> *prev;
    // Next node in doubly linked list
    // Node<K, V> *next;
    
    // std::chrono::steady_clock::time_point expiration_time;
    int node_level;

private:
    K key;
    V value;
};

template<typename K, typename V> 
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level; 
        
    // Set expiration time
    // this->expiration_time = std::chrono::steady_clock::now() + expiration

    // level + 1, because array index is from 0 - level
    this->forward = new Node<K, V>* [level + 1];
    
	// Fill forward array with 0 
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));

    // this->prev = nullptr;
    // this->next = nullptr;
};

template<typename K, typename V> 
Node<K, V>::~Node() {
    delete []forward;
};

template<typename K, typename V> 
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V> 
V Node<K, V>::get_value() const {
    return value;
};

template<typename K, typename V> 
void Node<K, V>::set_value(V value) {
    this->value=value;
};






template <typename K, typename V> 
class SkipList {

public: 
    SkipList(int);
    ~SkipList();
    // Generate level for a given Node
    int get_random_level();
    Node<K, V>* create_node(K, V, int);

    bool search_element(K);
    int insert_element(K, V);
    void delete_element(K);
    // Show all nodes at every level 
    void display_list();
    // Expict expire elements
    // void expire_elements(); 

    // Save data into persistent disk
    void dump_file();
    // Load file into SkipList
    void load_file();
    // Delete nodes recursively
    void clear(Node<K,V>*);
    int size();

private:
    // Utility functions for load_file
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:    
    // Maximum level of the skip list 
    int _max_level;

    // Current level of skip list 
    int _skip_list_level;

    // Pointer to header node 
    Node<K, V> *_header;
        
    // LRU cache
    // std::unordered_map<K, Node<K, V>*> node_map
    // std::list<Node<K, V>*> lru_list;
    // std::chrono::milliseconds default_expiration;


    // File operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // SkipList current elements count
    int _element_count;
};

template<typename K, typename V>                                                                                        
SkipList<K, V>::SkipList(int max_level) {                                                                                
    this->_max_level = max_level;                                                                                       
    this->_skip_list_level = 0;                                                                                         
    this->_element_count = 0;                                                                                           
    // create header node and initialize key and value to NULL
    K k;                                                                                                                
    V v;                                                                                                                
    this->_header = new Node<K, V>(k, v, _max_level);                                                                   
}; 

template <typename K, typename V>                                                                                       
void SkipList<K, V>::clear(Node<K, V> * cur)                                                                            
{                                                                                                                       
    if(cur->forward[0] != nullptr){                                                                                       
        clear(cur->forward[0]);                                                                                         
    }                                                                                                                   
    delete(cur);                                                                                                        
}

template<typename K, typename V>                                                                                        
SkipList<K, V>::~SkipList() {                                                                                           
    if (_file_writer.is_open()) {                                                                                       
        _file_writer.close();                                                                                           
    }                                                                                                                   
    if (_file_reader.is_open()) {                                                                                       
        _file_reader.close();                                                                                           
    }                                                                                                                   
                                                                                                    
    if(_header->forward[0] != nullptr){                                                                                   
        clear(_header->forward[0]);                                                                                     
    }                                                                                                                   
    delete(_header);                                                                                                    
} 

// create new node 
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}


// Search for element in skip list                                                                                      
/*                                                                                                                      
                            +------------+                                                                               
                            |  select 60 |                                                                               
                            +------------+                                                                               
 level 4     +-->1+                                                      100                                             
                  |                                                                                                      
                  |                                                                                                      
 level 3         1+-------->10+------------------>50+           70       100                                             
                                                    |                                                                    
                                                    |                                                                    
 level 2         1          10         30         50|           70       100                                             
                                                    |                                                                    
                                                    |                                                                    
 level 1         1    4     10         30         50|           70       100                                             
                                                    |                                                                    
                                                    |                                                                    
 level 0         1    4   9 10         30   40    50+-->60      70       100                                             

*/ 

template<typename K, typename V>                                                                                                                 
bool SkipList<K, V>::search_element(K key) {                                                                                                                                                                                                    
     std::cout << "Search_element-----------------" << std::endl;                                                        
     // expire_elements();

     // if (node_map.find(key) == node_map.end()) {
     //    std::count << "Key: " << key << " not found." << std::endl;
     //    return false;
     // }

     // Node<K, V>* node = node_map[key];
     // if (node -> expiration_time < std::chrono::steady_clock::now()) {
     //     delete_element(node->get_key());
     //     lru_list.remove(node);
     //     std::cout << "Key: " << key << " expired" << std::endl;
     //     return false;
     // }

     Node<K, V> *current = _header;                                                                                      
                                                                                                                         
     // Start from the highest level of skip list                                                                            
     for (int i = _skip_list_level; i >= 0; i--) {
         // Traverse through current layer until found a node whose next node's key is greater than or equals to
         // current key
         while (current->forward[i] && current->forward[i]->get_key() < key) {                                           
             current = current->forward[i];                                                                              
         }
         // current node is the last node whose key is smaller than or equals to the current node's
         // key -> sink to the lower level 
     }                                                                                                                   
                                                                                                                         
     // Reached at level 0: check if the next node is what we search for                                                
     current = current->forward[0];                                                                                      
                                                                                                                         
     if (current and current->get_key() == key) {                                                                        
         std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;                          
         return true;                                                                                                    
     }                                                                                                                   
     // Move the node to the front of the LRU list
     // // lru_list.remove(current);
     // lru_list.push_front(current);


     std::cout << "Not Found Key:" << key << std::endl;                                                                  
     return false;                                                                                                       
}



// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

template<typename K, typename V>                                                                                        
int SkipList<K, V>::get_random_level(){                                                                                 
    int k = 1;    
    // Flip a coin: whether to add level
    while (rand() % 2) {                                                                                                
         k++;                                                                                                            
     }                                                                                                                   
     k = (k < _max_level) ? k : _max_level;                                                                              
     return k;                                                                                                           
}; 


template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    // Use mutex to guareetee thread safety
    mtx.lock();
    Node<K, V> *current = this->_header;
    
    // Preserve the previous nodes(larget node who's smaller than the current node) at every level
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));  

    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i]; 
        }
        // Perverse current node for updating its pointer later
        update[i] = current;
    }

    // Reached level 0 and forward pointer to the next node, which is the position to insert new node.
    current = current->forward[0];

    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        // unlock before return
        mtx.unlock();
        return 1;
    }

    // If current is NULL that means we have reached to end of the level 
    // If current's key is not equal to key that means we have to insert node between update[0] and current node 
    if (current == NULL || current->get_key() != key ) {
        
        // Generate a random level for node
        int random_level = get_random_level();

        // If random level is greater than skip list's current level, initialize update value with pointer to header
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level+1; i < random_level+1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // Create a new node with random level generated 
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // Insert node at every level
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count ++;
        
        // Add to LRU List
        // node_map[key] = inserted_node;
        // lru_list.push_front(insert_element);
    }
    mtx.unlock();
    return 0;
}

template<typename K, typename V> 
void SkipList<K, V>::display_list() {

    std::cout << "\n*****Skip List*****"<<"\n"; 
    for (int i = 0; i <= _skip_list_level; i++) {
        // Points to the first node of every level
        Node<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key) {
    mtx.lock();
    Node<K, V> *current = this->_header; 
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        // previous node every level
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {
       
        for (int i = 0; i <= _skip_list_level; i++) {

            if (update[i]->forward[i] != current) 
                break;
            // delete current node at level i
            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --; 
        }

        std::cout << "Successfully deleted key "<< key << std::endl;
        delete current;
        _element_count --;

        // node_map.erase(key)
    }
    mtx.unlock();
    return;
}

// template<typename K, typename V>
// void SkipList<K, V>::expire_elements() {
//     auto now = std::chrono::steady_clock::now();
//     while (!lru_list.empty()) {
//         Node<K, V>* node = lru_list.back();
//         if (node->expiration_time > now) {
//             break;
//         }
//         delete_element(node->get_key());
//         lru_list.pop_back();
//     }
// }


// Dump data in memory to file
template<typename K, typename V>
void SkipList<K, V>::dump_file() {
    std::cout << "Dump file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << ";\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return ;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
    if (str.empty()) {
         return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}


// Load data from disk
template<typename K, typename V>
void SkipList<K, V>::load_file() {
    _file_reader.open(STORE_FILE);
    std::cout << "Load file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();

    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        // Define key as int type
        insert_element(stoi(*key), *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    delete key;
    delete value;
    _file_reader.close();
}

// Get current SkipList size
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}

