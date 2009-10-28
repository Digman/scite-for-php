#include <iostream>

using namespace std;

class Diogin {
    public:
        Diogin();
        ~Diogin();
    protected:
        int age;
        char *name;
};
int main(int argc, const char **argv) {
    printf("%d\n", argv[argc - 1]);
    return 0;
}
