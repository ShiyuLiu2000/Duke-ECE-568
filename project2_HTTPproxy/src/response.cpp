#include "response.hpp"
//#include <boost/algorithm/string/trim.hpp>
// Definition of the << operator
std::ostream& operator<<(std::ostream& os, const Response& response) {
    os << "Response Content:" << std::endl;
    os << "Status Line: " << response.statusLine << std::endl;
    os << "Content-Type: " << response.header.contentType << std::endl;
    os << "Content-Length: " << response.header.contentLength << std::endl;
    os << "Server: " << response.header.server << std::endl;
    os << "ETag: " << response.header.etag << std::endl;
    os << "Expires: " << response.header.expirationStr << std::endl;

    return os;
}
Response& Response::operator=(const Response& other) {
        if (this != &other) {
            // Perform the assignment
            response = other.response;
            statusLine = other.statusLine;
            header = other.header;
        }
        return *this;
    }
bool Response::isEmpty() const {
    return response.empty() && statusLine.empty() && header.isEmpty();
}
void Response::parseStatusLine(const char * line, size_t length) {
    std::string statLine(line, length);
    size_t endOfFirstLine = statLine.find("\r\n");
    statusLine = statLine.substr(0, endOfFirstLine);
}

void Response::parseGenericHeaderField(const std::string& hdr, const std::string& fieldName, std::string & fieldValue) {
    std::size_t fieldPos;
        if ((fieldPos = hdr.find(fieldName)) != std::string::npos) {
            // Calculate the start and end positions of the field value
            std::size_t startOfFieldPos = fieldPos + fieldName.length();
            std::size_t endOfFieldPos = hdr.find_first_of("\r\n", startOfFieldPos);
      
            // Check if the end of line character is found
            if (endOfFieldPos == std::string::npos) {
                std::cerr << "No end of line character found after " << fieldName << std::endl;
            }

            // Extract the field value using substr
            fieldValue = hdr.substr(startOfFieldPos, endOfFieldPos - startOfFieldPos);
        }
        else {
            //std::cerr << "Failed to parse " + fieldName << std::endl;
        }
        
    }


std::string Response::extractStatusCode(const std::string& httpResponse) {
    // Find the end of the first line (status line)
    size_t firstLineEnd = httpResponse.find("\r\n");
    if (firstLineEnd == std::string::npos) {
        // Unable to find the end of the first line
        std::cerr << "Invalid HTTP response format: Unable to find end of first line." << std::endl;
        return "";  // Return an empty string to indicate an error
    }

    // Extract the first line (status line)
    std::string firstLine = httpResponse.substr(0, firstLineEnd);

    // Parse the status code from the first line
    int statusCode;
    if (sscanf(firstLine.c_str(), "HTTP/%*s %d", &statusCode) != 1) {
        // Unable to parse the status code
        std::cerr << "Invalid HTTP response format: Unable to parse status code." << std::endl;
        return "";  // Return an empty string to indicate an error
    }

    // Convert the status code to a string and return
    return std::to_string(statusCode);
}

void Response::parseHeader(const char * head, size_t length) {
    std::string hdr (head, length);
    parseGenericHeaderField(hdr, "Content-Type: ", header.contentType);
    parseGenericHeaderField(hdr, "Content-Length: ", header.contentLength);
    parseGenericHeaderField(hdr, "Last-Modified: ", header.lastModified);
    parseGenericHeaderField(hdr, "Server: ", header.server);
    parseGenericHeaderField(hdr, "Location: ", header.location);
    extractStatusCode(hdr); 
    std::size_t datePos;
    if ((datePos = hdr.find("Date: ")) != std::string::npos) {
        size_t gmtPos = hdr.find(" GMT", datePos);
        std::string responseTimeStr = hdr.substr(datePos + 6, gmtPos - datePos - 6);
        header.responseTime.parseAndInitTime(responseTimeStr);
  }

  std::size_t maxAgePos;
  std::string maxAge = "max-age=";
  if ((maxAgePos = hdr.find(maxAge)) != std::string::npos) {
    std::string maxAgeStr = hdr.substr(maxAgePos + 8);
    
    try {
        header.maxAge = std::stoi(maxAgeStr.c_str());
    } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << " couldn't find or parse "  << maxAge << std::endl;
            //exit(EXIT_FAILURE);
            return;
        } catch (const std::out_of_range& e) {
            std::cerr << "Out of range: " << e.what() << std::endl;
            //exit(EXIT_FAILURE);
            return;
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            //exit(EXIT_FAILURE);
            return;
        }
    
  }
 
  std::size_t expirePos;
  const std::string expireSearchString = "Expires: ";
  if ((expirePos = hdr.find(expireSearchString, expireSearchString.length())) != std::string::npos) {
    //std::cout << hdr << std::endl;
    std::size_t gmtPos = hdr.find("GMT", expirePos);
    header.expirationStr = hdr.substr(expirePos + 9, gmtPos - expirePos - 9);
    header.expirationTime.parseAndInitTime(header.expirationStr);
  }
  std::size_t nocatchPos;
  if ((nocatchPos = hdr.find("no-cache")) != std::string::npos) {
    header.no_cache = true;
  }
 
  std::size_t etagPos;
  if ((etagPos = hdr.find("ETag: ")) != std::string::npos) {
    std::size_t etag_end = hdr.find("\r\n", etagPos + 6);
    header.etag = hdr.substr(etagPos + 6, etag_end - etagPos - 6);
  }
}

void Response::setResponse(const std::string & resp) {
    response = resp;
}
int Response::getResponseSize() const {
    return response.length();
}

const char * Response::getResponse() const {
    return response.c_str();
}

std::string Response::getResponseStr() const {
    return response;
}
std::string Response::getStatusLine () const {
    return statusLine;
}

Header Response::getHeader() const {
    return header;
}

