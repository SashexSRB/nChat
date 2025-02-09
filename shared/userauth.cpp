#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <algorithm> // For std::remove_if
#include <gpgme.h>   // GPGME library for PGP encryption
#include <cstring>
#include <sys/mman.h>  // For Linux (mprotect)

/*#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <wincrypt.h>
#else
    #include <sys/mman.h>
    #include <sys/random.h>
    #include <unistd.h>
#endif

void lockAndProtectPassword(std::string& password) {
    size_t len = password.size();

    // Check the password length
    if (len == 0) {
        throw std::invalid_argument("Password cannot be empty.");
    }

    // Allocate buffer to hold password
    char* passwordBuffer = new char[len];

    // Clear memory (zero out the buffer)
    std::memset(passwordBuffer, 0, len);

    // Copy password into the cleared memory
    std::memcpy(passwordBuffer, password.c_str(), len);

// Mark memory as read-only (protected)
#ifdef _WIN32
    DWORD oldProtect;
    if (!VirtualProtect(passwordBuffer, len, PAGE_READONLY, &oldProtect)) {
        delete[] passwordBuffer;
        throw std::runtime_error("Failed to protect memory.");
    }
#elif __linux__
    if (mprotect(passwordBuffer, len, PROT_READ) == -1) {
        delete[] passwordBuffer;
        throw std::runtime_error("Failed to protect memory.");
    }
#endif

    // Now pass the protected memory reference to the hashing function
    hashPassword(passwordBuffer, len);  // Your existing hashing function (e.g., SHA256)

    // After use, clear memory and delete buffer
    std::memset(passwordBuffer, 0, len);  // Clear memory before deleting
    delete[] passwordBuffer;  // Now delete the buffer
}*/

std::string trim(const std::string& str) {
    std::string s = str;

    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());

    return s;
}

std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(hash[i]);
    }

    return ss.str();
}

bool registerUser(const std::string& username, const std::string& password) {
    std::ofstream file("users.txt", std::ios::app);

    if (!file) {
        std::cerr << "Error opening users.txt for writing." << std::endl;

        return false;
    }

    std::string trimmedPassword = trim(password);
    std::string hashedPassword = hashPassword(trimmedPassword);
    
    file << username << " " << hashedPassword << std::endl;

    return true;
}

bool validateUser(const std::string& username, const std::string& password) {
    std::ifstream file("users.txt");

    if (!file) {
        std::cerr << "Error opening users.txt for reading." << std::endl;

        return false;
    }

    std::string line, storedUsername, storedHash;
    std::string trimmedPassword = trim(password);
    std::string hashedPassword = hashPassword(trimmedPassword);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        iss >> storedUsername >> storedHash;

        if (storedUsername == username && storedHash == hashedPassword) {
            return true; // Successful validation
        }
    }

    return false; // Failed validation
}
