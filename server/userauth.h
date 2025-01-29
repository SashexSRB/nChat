#ifndef USERAUTH_H
    #define USERAUTH_H

    #include <string>

    // Function to hash the password
    std::string hashPassword(const std::string& password);

    // Function to register a new user and store their hashed password
    bool registerUser(const std::string& username, const std::string& password);

    // Function to validate user credentials during login
    bool validateUser(const std::string& username, const std::string& password);
#endif
