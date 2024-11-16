#include "syscall.h"

int main(void) {
    char test[] = "abcdefghijklmnopqrstuvwxyz";
    int success = Create("file1.test");
    int success2;
    OpenFileId fid;
    OpenFileId fid2;
    
    int i;
    if (success != 1)
        MSG("Failed on creating file");
    


    fid = Open("file1.test");
    if (fid < 0)
        MSG("Failed on opening file");
    else {
        MSG("Successful on opening file!!");
        PrintInt(fid);
    }
    fid2 = Open("file1.test");
    if (fid2 < 0)
        MSG("Failed on opening file");
    else {
        MSG("Successful on opening file!!");
        PrintInt(fid2);
    }
    
    for (i = 0; i < 26; ++i) {
        int count = Write(test + i, 1, fid);
        if (count != 1)
            MSG("Failed on writing file");
    }

    for (i = 0; i < 26; ++i) {
        int count = Write(test + i, 1, fid2);
        if (count != 1)
            MSG("Failed on writing file");
    }
    success = Close(fid);
    if (success != 1)
        MSG("Failed on closing file");
    MSG("Success on creating file1.test");
    success2 = Create("file1.test");
    if (success2 != 1)
        MSG("Failed on creating file");
    Halt();
}
