#include "dataBase.h"

#include <curl/curl.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>

struct MemoryStruct {
  char* memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp){
  size_t realsize = size * nmemb;
  struct MemoryStruct* mem = (struct MemoryStruct*)userp;

  char* ptr = (char*)std::realloc((void*)mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

std::vector<sf::Vector3i> 
DataBase::downloadData(string _url, string _query) {
  std::vector<sf::Vector3i> data;
  string header;
  string response = "";
  string url = _url;
  string query = _query;
  Get(url, query, header, response);

  if (response == "") {
    return data;
  }
  
  rapidjson::Document jsonFile;
  jsonFile.Parse(response.c_str());

  if (jsonFile.IsNull()) {
    return data;
  }

  if (jsonFile.HasMember("hits")) {
    if (jsonFile.HasMember("hits")) {
      if (jsonFile["hits"]["hits"].IsArray()) {
        auto arrayOfResults = jsonFile["hits"]["hits"].GetArray();
        for (rapidjson::Value::ConstValueIterator itr = arrayOfResults.Begin(); 
             itr != arrayOfResults.End(); 
             ++itr) {
          auto& jsonObject = *itr;
          if (jsonObject.HasMember("_source")) {
            if (jsonObject["_source"].IsObject()) {
              auto sourceObject = jsonObject["_source"].GetObj();

              if (sourceObject.HasMember("position")) {
                sf::Vector3i v;
                auto positionArray = sourceObject["position"].GetObj();
                v.x = static_cast<int>(positionArray["x"].GetInt());
                v.y = static_cast<int>(positionArray["y"].GetInt());
                v.z = static_cast<int>(positionArray["z"].GetInt());
                data.push_back(v);
              }

            }
          }
        }
      }

    }
  }

  std::ofstream json("result.json");
  json << response;
  json.close();
  
  return data;
}

int
DataBase::Post(string url, string query, string& headerStr, string& responseStr) {
  MemoryStruct responseChunk;
  MemoryStruct headerChunk;

  responseChunk.memory = (char*)malloc(1);
  responseChunk.size = 0;
  headerChunk.memory = (char*)malloc(1);
  headerChunk.size = 0;

  CURLcode res = CURLE_OK;
  CURL* curl;
  struct curl_slist* headers;

  headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charset: utf-8");

  curl = curl_easy_init();
  if ((res = curl_easy_setopt(curl, 
                              CURLOPT_URL, 
                              url.c_str())) != CURLE_OK) {
    fprintf(stderr, curl_easy_strerror(res));
    return res;
  }
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 204800L);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)query.size());
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.55.1");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseChunk);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerChunk);

  res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl = nullptr;
  curl_slist_free_all(headers);
  headers = nullptr;

  responseStr = responseChunk.memory;
  headerStr = headerChunk.memory;
  delete curl;
  free(responseChunk.memory);
  free(headerChunk.memory);

  return res;
}

int 
DataBase::Get(string url, string query, string& headerStr, string& responseStr) {
  MemoryStruct responseChunk;
  MemoryStruct headerChunk;

  responseChunk.memory = (char*)malloc(1); 
  responseChunk.size = 0;
  headerChunk.memory = (char*)malloc(1);
  headerChunk.size = 0;

  CURLcode res = CURLE_OK;
  CURL* curl;
  struct curl_slist* headers;

  headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  if ((res = curl_easy_setopt(curl,
                              CURLOPT_URL,
                              url.c_str())) != CURLE_OK) {
    fprintf(stderr, curl_easy_strerror(res));
    return res;
  }
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 204800L);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)query.size());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&responseChunk);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&headerChunk);

  res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl = nullptr;
  curl_slist_free_all(headers);
  headers = nullptr;

  responseStr = responseChunk.memory;
  headerStr = headerChunk.memory;
  delete curl;

  free(responseChunk.memory);
  free(headerChunk.memory);
  return res;
}
