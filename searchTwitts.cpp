/*
* g++ <sourcefile> -o <outputfile> -lcurl -ljsoncpp
*/
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <cstdlib>
#include <cstring>

#include <sstream>
#include <vector>

using namespace std;

static size_t WriteCallback(char *contents, size_t size, size_t nmemb, char *buffer_in) {
	((string*)buffer_in)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

string getAuthor(Json::Value users, Json::Value id) {
	for (int n = 0; n < users.size(); n++) {
		if (users[n]["id"] == id) {
			return "@" + users[n]["username"].asString();
		}
	}
	return "undefined";
}

char* getAuthorization() {
	char *TWITTER_ACCESSTOKEN = getenv("TWITTER_ACCESSTOKEN");
	if ( TWITTER_ACCESSTOKEN == NULL ) {
		cerr << "Environment variable TWITTER_ACCESSTOKEN is required" << endl;
		return "undefined";
	} else {
		string str_obj(string("Authorization: Bearer ") + TWITTER_ACCESSTOKEN);
		char* char_arr;
		char_arr = &str_obj[0];
		return char_arr;
	}
}

string getStringData(string search) {
	string urlString = "https://api.twitter.com/2/tweets/search/recent?tweet.fields=author_id,created_at,lang&expansions=author_id&user.fields=name,profile_image_url,username,verified,withheld&query=" + search;

	char url[urlString.length()] = "";
	for (int i = 0; i < urlString.length(); i++) {
		url[i] = urlString[i];
	}

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, getAuthorization());

	CURL *curl = curl_easy_init();
	if (!curl) {
		cerr << "curl initialization failure" << endl;
		return "1";
	}

	CURLcode res;
	string readBuffer;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK) {
		cout << url << endl;
		cerr << "curl_easy_perform() failed: " << res << " - " << curl_easy_strerror(res) << endl;
	}

	return readBuffer;
}

string split(string s, string delimiter) {
	size_t pos = 0;
	string token;
	while ((pos = s.find(delimiter)) != string::npos) {
		token = s.substr(0, pos);

		cout << token << endl;
		s.erase(0, pos + delimiter.length());
	}
	// cout << s << endl;
	return s;
}

vector<string> splitToArray(string s1) {
    istringstream iss(s1);
    vector<string> result;
    for(string s;iss>>s;)
        result.push_back(s);

	return result;

    // int n=result.size();
    // for(int i=0;i<n;i++)
    //     cout<<result[i]<<endl;
}

int main(int total, char* args[]) {

	if (!args[1]) {
		cerr << "A query param is required" << endl;
		return 1;
	}

	string search = args[1];
	string response = getStringData(search);

	Json::Reader reader;
	Json::Value obj, data, users;
	reader.parse(response, obj);

	data = obj["data"];
	users = obj["includes"]["users"];

	for (int i = 0; i < data.size(); i++) {
		cout << "------------------------------------" << endl;
		cout << getAuthor(users, data[i]["author_id"]) << " - " << data[i]["created_at"].asString() << endl;
		cout << data[i]["text"].asString() << endl;
	}

	// cout << endl;
	// cout << endl;
	// cout << "###################################################" << endl;

    // vector<string> result = splitToArray(data[1]["text"].asString());
    // int n=result.size();
    // for(int i=0;i<n;i++)
    //     cout<<result[i]<<endl;

	// for (int i = 0; i < data.size(); i++) {
	// cout << split(data[0]["text"].asString(), " ") << endl;
	// }

	return 0;
}
