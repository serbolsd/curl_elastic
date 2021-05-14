#pragma once
#include <array>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <SFML/System/Vector3.hpp>
using std::string;
class DataBase
{
public:
		DataBase() {};
		~DataBase() {};

		std::vector<sf::Vector3i>
		downloadData(string _url, string _query);

		int
		Post(string url, string query, string& headerStr, string& responseStr);

		int
		Get(string url, string query, string& headerStr, string& responseStr);
};
