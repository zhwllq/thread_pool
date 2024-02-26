#include "thread_pool.h"                                                                                                                                     
                                                                                                                                                             
using namespace nio;                                                                                                                                         
                                                                                                                                                             
void testFunction(int i) {                                                                                                                                   
    std::cout << i << std::endl;                                                                                                                             
}                                                                                                                                                            
                                                                                                                                                             
int main() {                                                                                                                                                 
    std::cout << "thread poll start..." << std::endl;                                                                                                        
    ThreadPool tpool;                                                                                                                                        
    tpool.init(5);                                                                                                                                           
    tpool.exec(testFunction, 10);                                                                                                                            
    tpool.start();                                                                                                                                           
    tpool.waitForAllDone(1000);                                                                                                                              
    return 0;                                                                                                                                                
}            

