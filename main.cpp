#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {

    SkipList<int, std::string> skipList(6);
	skipList.insert_element(1, "Learn"); 
	skipList.insert_element(3, "Test"); 
	skipList.insert_element(7, "Skip list"); 
	skipList.insert_element(8, "I built using C++"); 
	skipList.insert_element(9, "Check"); 
	skipList.insert_element(19, "with"); 
	skipList.insert_element(19, "Several tests"); 

    std::cout << "Size:" << skipList.size() << std::endl;

    skipList.dump_file();

    // skipList.load_file();

    skipList.search_element(9);
    skipList.search_element(18);


    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "Size:" << skipList.size() << std::endl;

    skipList.display_list();
}
