#include <iostream>
#include <string>

// Unless otherwise specified, all code is written by Google Gemini 3.1 Pro
// Any comment made by Kristoffer Roy Comahig, the student, is marked with STU: as a prefix
// 'STU:' comments have been written in during debugging and validation phases, also as additional proof of comprehension of AI-generated code

// Helper function to validate a single token and parse its values if valid.
// Returns true if the token is a strictly valid IPv4 address (with optional port).
bool isValidToken(const std::string& t, unsigned long& addr, int& port) { //STU: addr and port are passed in as references in order to alter the variables in extractIPv4()
    if (t.empty()) return false; //STU: Checks if passed string is empty, thus being an invalid token
    
    int parts[5]; //STU: An array meant to store and keep track of the individual decimal values of the four octets alongside the optional port number
    int numParts = 0; //STU: a integer counter meant to keep track of how many parts have been parsed through and stored
    
    unsigned long currentVal = 0; //STU: tracks the current decimal value of the extracted IP address
    int digitCount = 0; //STU: keeps track of the amount of digits parsed through in the current portion of token
    bool isPortPhase = false; //STU: a boolean value meant to track if the current portion of token is a port, in which case it needs to be parsed through different

    for (size_t i = 0; i < t.length(); ++i) { //STU: iterates through the whole passed token
        char c = t[i]; 
        
        if (c == '.') { //STU: this if-statement checks if the program is at the beginning or end of an octet
            if (isPortPhase) return false; // Dot not allowed in port
            if (digitCount == 0) return false; // Consecutive dots or leading dot
            
            // Leading zero check: if more than 1 digit, first digit cannot be '0'
            if (digitCount > 1 && t[i - digitCount] == '0') return false; 
            if (currentVal > 255) return false; 
            
            parts[numParts++] = static_cast<int>(currentVal); //STU: Stores the newly parsed through parts decimal value into parts[]
            if (numParts > 4) return false; // Too many octets
            
            currentVal = 0; //STU: regardless of being the end or beginning of an octet, it resets both current val and digit count back to0
            digitCount = 0;
        } 
        else if (c == ':') { //STU: this if-statement checks for the beginning of a Port number
            if (isPortPhase) return false; // Multiple colons
            if (digitCount == 0) return false; // Colon immediately after a dot or start
            
            if (digitCount > 1 && t[i - digitCount] == '0') return false; //STU: checks for leading 0s in the previous/last octet, making this number and overall token invalid
            if (currentVal > 255) return false; //STU: similarly as above, checks if the last octet had a decimal value greater than 255, making it invalid
            
            parts[numParts++] = static_cast<int>(currentVal); //STU: grabs the current value of the previous octet and stores it in parts[]
            if (numParts != 4) return false; // Colon must be exactly after the 4th octet
            
            isPortPhase = true; //STU: sets up variables necessary to mark the program has passed into checking for a port number and resets the token portion storage
            currentVal = 0;
            digitCount = 0;
        } 
        else if (c >= '0' && c <= '9') { //STU: this if-statement is used when the program is mid-way through parsing a number whether that be for an octet or port
            digitCount++; 
            currentVal = currentVal * 10 + (c - '0'); //STU: takes the ASCII value of the digit and subtracts by the ASCII value of '0' to get the actual decimal value of said digit, raises it to the proper decimal place and adds to said value to currentVal
            
            if (!isPortPhase) {
                // Octet checks
                if (digitCount > 3 || currentVal > 255) return false;
            } else {
                // Port checks
                if (digitCount > 5 || currentVal > 65535) return false;
            }
        } 
        else {
            // Should never be reached based on tokenization rules, but included for safety
            return false;
        }
    }

    // Process the final segment of the token
    if (digitCount == 0) return false; // Token ends with a '.' or ':'
    if (digitCount > 1 && t[t.length() - digitCount] == '0') return false; //STU: checks if the final segment has a leading 0, making it invalid
    
    if (!isPortPhase) {
        if (currentVal > 255) return false; //STU: if there is no optional port number, checks if the last octets value is greater than 255, making it invalid
        parts[numParts++] = static_cast<int>(currentVal); //STU: stores the final octets decimal value in parts[]
        if (numParts != 4) return false; //STU: If there aren't four octets, this address is invalid
        port = -1; //STU: indicates there is no port
    } else {
        if (currentVal > 65535) return false; //STU: checks if the port numbers decimal value is valid
        parts[numParts++] = static_cast<int>(currentVal); //STU: stores the decimal value into parts[]
        if (numParts != 5) return false; //STU: if there aren't four octets and one port number, it is an invalid address
        port = parts[4]; //STU: has port reference parts[4] which is the ports decimal value
    }

    // Assemble the 32-bit address 
    //STU: addr is made to reference a decimal / binary value in which individual values from parts are stored in a singular value
    //STU: this is done by shifting each token part's decimal value to a different section of the singular value and using OR to do so without having them affect each other
    addr = ((unsigned long)parts[0] << 24) | 
           ((unsigned long)parts[1] << 16) | 
           ((unsigned long)parts[2] << 8)  | 
           ((unsigned long)parts[3]);
           
    return true; //STU: indicates it is a valid address token
}

// Returns True if a valid address was found, False otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    outAddress = 0; //STU: outAddress and outPort initialize variables that will store the return values for an extracted address and port
    outPort = -1;

    std::string currentToken = "";
    
    // We iterate up to str.length() inclusively to force processing of the last token
    for (size_t i = 0; i <= str.length(); ++i) {
        char c = (i < str.length()) ? str[i] : ' '; //STU: c is set to be the value of str at index i UNLESS i is greater than or equal to the length of the string, meaning the program must start processing the last token
        
        // Allowed characters for a candidate token
        if ((c >= '0' && c <= '9') || c == '.' || c == ':') { //STU: if the character is valid and it is not the end of the current token sequence, it is added to the token being built
            currentToken += c;
        } else {
            // End of current token sequence
            if (!currentToken.empty()) { 
                unsigned long tempAddr = 0; //STU: tempAddr and tempPort are set to be variables used to reference the addr and port returned by isValidToke
                int tempPort = -1;
                
                if (isValidToken(currentToken, tempAddr, tempPort)) { //STU: if the token iterated through is a valid address and port, sets outAddress and outPort to the ref variables and returns True
                    outAddress = tempAddr;
                    outPort = tempPort;
                    return true; //STU: returns True here to prevent multiple IP addr extractions from one line
                }
                // Reset token for the next candidate search
                currentToken = "";
            }
        }
    }
    
    return false;
}

int main() {
    std::string inputLine; //STU: initalizes a string to store the user's input
    
    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, inputLine)) {
            break; // Break gracefully on unexpected EOF
        }
        
        if (inputLine == "END") {
            std::cout << "Program terminated.\n";
            break;
        }

        unsigned long address = 0; //STU: address and port serve as variables to store the references passed by extractIPv4 and isValidToken during their running
        int port = -1;
        
        if (extractIPv4(inputLine, address, port)) {
            // Unpack 32-bit int back into individual octets for display purposes
            unsigned long a = (address >> 24) & 0xFF;
            unsigned long b = (address >> 16) & 0xFF;
            unsigned long c = (address >> 8) & 0xFF;
            unsigned long d = address & 0xFF;
            
            //STU: below is a string output that prints each individual decimal value for the octet in the address format as well as the decimal value overall along with a possible port number
            std::cout << "Extracted IPv4 address: " 
                      << a << "." << b << "." << c << "." << d 
                      << " (decimal value: " << address << ", port: ";
                      
            if (port == -1) { //STU: displays 'none' if there was no port number detected and the port number itself if there was
                std::cout << "none)\n";
            } else {
                std::cout << port << ")\n";
            }
        } else {
            std::cout << "Invalid input: no valid IPv4 address found\n";
        }
    }
    
    return 0;
}