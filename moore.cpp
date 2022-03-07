#include <stdlib.h>
#include <stdlib.h>
#include <list>
#include <vector>

typedef std::list<int> Int_list;
typedef std::vector<std::vector<Int_list>> Nh;

void* create_neighbourhood(int n){
    Nh nh(n);
    void* ptr = malloc(sizeof(nh));
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            Int_list l = {};
            nh[i][j] = l;
        }
    }
    return ptr;
}
