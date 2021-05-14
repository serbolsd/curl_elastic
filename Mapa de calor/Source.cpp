#include <cstdlib>
#include <ctime>
#include <sstream>
#include "dataBase.h"
#include "heatMap.h"

void sentRandomData() {
  DataBase data;

  std::string a;
  std::string b;
  srand((unsigned)time(0));
  for (int i = 0; i < 100; i++)
  {
    std::string query = "{\"position\":{\"x\":";
    int randomNum =  std::rand() % 1280 + 1;
    std::stringstream ss;
    ss << randomNum;
    query += ss.str();

    query += ",\"y\":";
    randomNum = std::rand() % 1280 + 1;
    std::stringstream ss2;
    ss2 << randomNum;
    query += ss2.str();
    ss2.clear();

    query += ",\"z\":";
    randomNum = std::rand() % 100 + 1;
    std::stringstream ss3;
    ss3 << randomNum;
    query += ss3.str();
    ss3.clear();
    
    query += "}}";
    data.Post("http://localhost:9200/mytutorialindex/message?pretty", query,a,b);
  }
}

int main()
{
  //sentRandomData();
  DataBase data;
  std::vector<sf::Vector3i> pos;
  //"{\"from\" : 0, \"size\" : 1000, \"query\": {\"query_string\": {\"query\": \"*\"}}}"
  //"{\"query\": {\"query_string\": {\"query\": \"*\"}}}"
  pos = data.downloadData("http://localhost:9200/mytutorialindex/_search?pretty",
                          "{\"from\" : 0, \"size\" : 100, \"query\": {\"query_string\": {\"query\": \"*\"}}}");
  HeatMap heat(1280, 720,0.05, 1);
  heat.createHeatMap(pos);
  heat.render();
  return 0;
}