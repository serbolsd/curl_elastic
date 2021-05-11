#include <SFML/Window.hpp>
#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "rapidjson/stringbuffer.h"

struct MemoryStruct {
  char* memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct* mem = (struct MemoryStruct*)userp;

  char* ptr = (char*)std::realloc((void*)mem->memory, mem->size + realsize + 1);
  //ptr.resize(mem->size + realsize + 1);
  //std::memcpy(ptr, mem->memory, mem->size + realsize + 1);
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

int write() {
  curl_global_init(CURL_GLOBAL_ALL);
  CURL* curl = curl_easy_init();
  if (!curl)
  {
    std::cout << "No se inicio curl" << std::endl;
  }
  //curl http://localhost:9200/
  CURLcode res = CURLE_OK;

  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  if ((res = curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/mytutorialindex/message?pretty")) != CURLE_OK)
  {
    fprintf(stderr, curl_easy_strerror(res));
    return 1;
  }
  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charset: utf-8");

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"text\" : \"bye world!\" , \"timestamp\" : \"2021-10-07T10:29:50+06:00\" }");
  //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");

  if ((res = curl_easy_perform(curl)) != CURLE_OK)
  {
    fprintf(stderr, curl_easy_strerror(res));
    //return 1;
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

int read() {
  CURL* curl = curl_easy_init();

  struct MemoryStruct chunk;

  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  if (!curl)
  {
    std::cout << "No se inicio curl" << std::endl;
  }
  //curl http://localhost:9200/
  CURLcode res = CURLE_OK;

  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  if ((res = curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/mytutorialindex/_search?pretty")) != CURLE_OK)
  {
    fprintf(stderr, curl_easy_strerror(res));
    return 1;
  }
  if ((res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1)) != CURLE_OK)
  {
    fprintf(stderr, curl_easy_strerror(res));
    return 1;
  }

  /* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

  //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  //curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

  if ((res = curl_easy_perform(curl)) != CURLE_OK)
  {
    fprintf(stderr, curl_easy_strerror(res));
    //return 1;
  }

  rapidjson::Document doc;
  doc.Parse(chunk.memory, chunk.size);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  if (doc.HasMember("mytutorialindex")) {
    std::cout << "Se guardo" << std::endl;
  }

  std::ofstream outFile;
  outFile.open("elastic2.json");
  outFile << buffer.GetString();
  outFile.close();
  free(chunk.memory);
  curl_easy_cleanup(curl);
  // run the program as long as the window is open
}

int main()
{
  //write();
  read();

  return 0;
}