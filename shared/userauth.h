#ifndef USERAUTH_H
#define USERAUTH_H

#include <string>

#define MAGIC_LOGIN 0x00099586
#define MAGIC_REGISTER 0x43582534

    // Function to hash the password
    std::string hashPassword(const std::string& password);
    std::string trim(const std::string& str);

    // Function to register a new user and store their hashed password
    bool registerUser(const std::string& username, const std::string& password);

    // Function to validate user credentials during login
    bool validateUser(const std::string& username, const std::string& password);
#endif
