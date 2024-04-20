#include <iostream>
#include <curl/curl.h>
#include <string>
#include <regex>
#include <vector>

using namespace std;


struct MemoryStruct {
    char* memory;
    size_t size;
};


static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
       
        cout << "Not enough memory (realloc returned NULL)\n";
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

string GetTemperature(const string& htmlContent) {
    regex pattern(R"regex(<div class="min">(-?[0-9]+)</div><div class="max">(-?[0-9]+)</div>)regex");
    smatch match;

    if (regex_search(htmlContent, match, pattern)) {
        string minTemp = match[1];
        string maxTemp = match[2];
        return "Min: " + minTemp + "°C, Max: " + maxTemp + "°C";
    }

    return "N/A";
}


string GetWebPage(const string& url) {
    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = (char*)malloc(1);  
    chunk.size = 0;                  

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }

    string page(chunk.memory);
    free(chunk.memory);

    return page;
}


void GetWeather(const string& cityURL, const string& cityName) {
    string htmlContent = GetWebPage(cityURL);
    string temperature = GetTemperature(htmlContent);
    cout << "Current temperature in " << cityName << ": " << temperature << endl;
}

int main() {
    vector<pair<string, string>> cities = {
        {"https://ua.sinoptik.ua/погода-київ", "Kyiv"},
        {"https://ua.sinoptik.ua/погода-харків", "Kharkiv"},
        {"https://ua.sinoptik.ua/погода-одеса", "Odessa"},
        {"https://ua.sinoptik.ua/погода-львів", "Lviv"},
        {"https://ua.sinoptik.ua/погода-дніпро", "Dnipro"},
        {"https://ua.sinoptik.ua/погода-запоріжжя", "Zaporizhzhia"},
        {"https://ua.sinoptik.ua/погода-вінниця", "Vinnytsia"},
        {"https://ua.sinoptik.ua/погода-миколаїв", "Mykolaiv"},
        {"https://ua.sinoptik.ua/погода-суми", "Sumy"},
        {"https://ua.sinoptik.ua/погода-чернігів", "Chernihiv"}
    };

    for (const auto& city : cities) {
        GetWeather(city.first, city.second);
    }

    return 0;
}
