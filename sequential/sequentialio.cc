#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

inline void printUsage()
{
    cout << "./sequentialio [OPTIONS] -d <DESTINATION>" << endl
         << "sequentialio writes and reads to <DESTINATION>" << endl
         << "\t-d: Specifies the destination where sequential writes will be done." << endl
         << "\t    You must have write permission in the parent directory of <DESTINATION>" << endl

         << "OPTIONS:" << std::endl
         << "\t-c: Set the amount of measurement units (-u) to be written (default: 1)" << endl
         << "\t-h: Display this help." << endl
         << "\t-i: Number of iterations to be done (per write/read test)" << endl
         << "\t-r: Remove file afterwards (default: not)" << endl
         << "\t-u: Set the measurement unit. Options: " << endl
         << "\t    b or B (Byes), k (kB), K (KiB), m (MB), M (MiB), g (GB), G (GiB), t (TB), T (TiB) Default: GiB" << endl
         << "\t    BEWARE: The system must have a sufficient amount of memory!" << endl
         << "\t    .iB values are exponentials of 1024, i.e. MiB means 1024*1024 Bytes will be written." << endl;
}

long long calculateMeasurementBytes(char u)
{
    switch(u) {
    case 'b':
    case 'B':
        return 1;
    case 'k':
        return 1000;
    case 'K':
        return 1024;
    case 'm':
        return 1000*1000;
    case 'M':
        return 1024*1024;
    case 'g':
        return 1000*1000*1000;
    case 'G':
        return 1024*1024*1024;
    case 't':
        return (long long)1000*1000*1000*1000;
    case 'T':
        return (long long)1024*1024*1024*1024;
    default:
        return 1024*1024*1024;
    }
}

string getMeasurementUnit(char u)
{
    switch(u) {
    case 'b':
    case 'B':
        return "B";
    case 'k':
        return "kB";
    case 'K':
        return "KiB";
    case 'm':
        return "MB";
    case 'M':
        return "MiB";
    case 'g':
        return "GB";
    case 'G':
        return "GiB";
    case 't':
        return "TB";
    case 'T':
        return "TiB";
    default:
        return "GiB";
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage();
        return 0;
    }

    char u = 'G';
    int count = 1;
    int iterations = 4;
    char input;
    bool removeFile = false;
    string destination("");
    while((input = getopt (argc, argv, "c:d:hu:rs:")) != -1) {
        switch (input) {
        case 'c':
            count = atoi(optarg);
            break;
        case 'd':
            destination = optarg;
            break;
        case 'i':
            iterations = atoi(optarg);
            break;
        case 'u':
            u=optarg[0];
            break;
        case 'r':
            removeFile = true;
            break;
        case 'h':
        default:
            printUsage();
        }
    }

    if(destination.empty()) {
        printUsage();
        return 0;
    }

    long long totalAmountOfBytes = calculateMeasurementBytes(u) * count;

    cout << "Write/read of " << count << getMeasurementUnit(u) << " or " << totalAmountOfBytes << " Bytes in total, "
         << iterations << " times to destination " << destination << endl;
    cout << "Allocating ...";

    char* outputString = new char[totalAmountOfBytes];
    cout << " done" << endl << "Filling array ... " << flush;
    for(long long i = 0; i<totalAmountOfBytes; i++)
        outputString[i] = i % 26 + 'a';
    cout << "done" << endl;

    unsigned long long writeRuntimes[iterations];
    unsigned long long readRuntimes[iterations];

    fstream testFile;
    for(int iteration = 0; iteration < iterations; iteration++) {
        cout << "Run " << iteration+1 << " write" << endl;
        testFile.open(destination, fstream::out | fstream::binary);
        if(testFile) {
            //write
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            testFile << outputString;
            std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
            writeRuntimes[iteration] = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            testFile.close();
        } else {
            cout << "Could not open " << destination << " for write, aborting." << endl;
            exit(1);
        }

        cout << "Run " << iteration+1 << " read" << endl;
        testFile.open(destination, fstream::in | fstream::binary);
        if(testFile) {
            //read
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            testFile >> outputString;
            std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
            readRuntimes[iteration] = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            testFile.close();
        } else {
            cout << "Could not open " << destination << " for read, aborting." << endl;
            exit(2);
        }
    }

    unsigned long long readResult[2] = {std::numeric_limits<unsigned long long>::max(), std::numeric_limits<unsigned long long>::min()};
    unsigned long long writeResult[2] = {std::numeric_limits<unsigned long long>::max(), std::numeric_limits<unsigned long long>::min()};

    for(int iteration = 0; iteration < iterations; iteration++) {
        if(readRuntimes[iteration] < readResult[0])
            readResult[0] = readRuntimes[iteration];
        if(readRuntimes[iteration] > readResult[1])
            readResult[1] = readRuntimes[iteration];

        if(writeRuntimes[iteration] < writeResult[0])
            writeResult[0] = writeRuntimes[iteration];
        if(writeRuntimes[iteration] > writeResult[1])
            writeResult[1] = writeRuntimes[iteration];
        cout << "write " << iteration << " " << writeRuntimes[iteration] << endl;
        cout << "read " << iteration << " " << readRuntimes[iteration] << endl;

    }

    auto totalMB  = totalAmountOfBytes / calculateMeasurementBytes('m');

    cout << "READ:  min " << totalMB * 1000 / readResult[1] << "MB/s max " << totalMB * 1000 / readResult[0] << " MB/s" << endl;
    cout << "WRITE: min " << totalMB * 1000 / writeResult[1] << "MB/s max " << totalMB * 1000 / writeResult[0] << " MB/s" << endl;

    if(removeFile && !remove(destination.c_str()))
        cout << "Could not remove " << destination << endl;

    delete(outputString);
}

