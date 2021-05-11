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
// #include <algorithm>

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

// string split(string s, string delimiter) {
// 	size_t pos = 0;
// 	string token;
// 	while ((pos = s.find(delimiter)) != string::npos) {
// 		token = s.substr(0, pos);

// 		cout << token << endl;
// 		s.erase(0, pos + delimiter.length());
// 	}
// 	// cout << s << endl;
// 	return s;
// }


string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

string sanitize(std::string str) {
	vector<string> iChars = {"ç","Ç","á","é","í","ó","ú","ý","Á","É","Í","Ó","Ú","Ý","à","è","ì","ò","ù","À","È","Ì","Ò","Ù","ã","õ","ñ","ä","ë","ï","ö","ü","ÿ","Ä","Ë","Ï","Ö","Ü","Ã","Õ","Ñ","â","ê","î","ô","û","Â","Ê","Î","Ô","Û"};
	vector<string> oChars = {"c","C","a","e","i","o","u","y","A","E","I","O","U","Y","a","e","i","o","u","A","E","I","O","U","a","o","n","a","e","i","o","u","y","A","E","I","O","U","A","O","N","a","e","i","o","u","A","E","I","O","U"};

	for (int i = 0; i < iChars.size(); i++){
		string& from = iChars[i];
		string& to = oChars[i];
		str = ReplaceAll(str, from, to);
	}
    return str;
}

vector<string> splitToArray(string s1) {
    istringstream iss(s1);
    vector<string> result;
    for(string s;iss>>s;) {
		s = sanitize(s);
        result.push_back(s);
	}

	cout << result.size() << " WORDS" << endl;
	return result;
}

vector<string> prepareToLCDOutput(string text) {
	vector<string> words = splitToArray(text);
	int maxCount = 16;
	vector<string> lines;
	int lineLength = 0;
    
	string lineString;
	vector<string> linesContainer;

	for(int i=0;i<words.size();i++) {

		if (lineLength + words[i].length() + 1 <= maxCount){
			lineString += (lineString.length() > 0 ? " " : "") + words[i];
		} else {
			linesContainer.push_back(lineString);
			lineString = words[i];
		}

		if (i + 1 == words.size()) {
			linesContainer.push_back(lineString);
			cout << linesContainer.size() << " LINES" << endl;
		}
		lineLength = lineString.length();
	}

	return linesContainer;

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
		
		cout << endl;

		vector<string> result = prepareToLCDOutput(data[i]["text"].asString());

		int n=result.size();
		for(int i=0;i<n;i++)
			cout<<"> "<<result[i]<<endl;
		
		cout << endl;
	}

	return 0;
}
