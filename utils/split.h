#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace Utils
{
  std::vector<std::string>& split(const std::string& s, const char delim, std::vector<std::string>& elems);

  /**
    * @brief split string function
    * @param s string to split
    * @param delim delimter character
    * @return splitted string in a std::vector<std::string>
    */
  std::vector<std::string> split(const std::string& s, const char delim);

  /**
    * @brief split string function
    * @param s string to split - altering the input string
    * @param delim delimter character
    * @return splitted string in a std::vector<const char*>
    */
  std::vector<const char*> splitDestructive(std::string& s, const char delim);

  /**
    * @brief Splits a string on every occasion of one of the input-chars, in the given order
    *      Undefined behavior if the chars occur multiple times.
    */
  std::vector<std::string> split(const std::string& s, const std::string& delim);

  std::string replaceString(std::string subject, const std::string& search, const std::string& replace);
}  // namespace Utils
