#include <iostream>
#include <map>
#include <string>

using namespace std;
using std::map;
usind std::string;


// 这个实现时间复杂度是ON^3,这个复杂度是不可接受的
class Solution1 {
public:
    bool checkPalindrome(string s) {
        int j = s.length() - 1;
        int i = 0;
        while(i < j) {
            if(s[j] != s[i]) {
                return false;
            }
            i++;
            j--;
        }
        return true;
    }

    string longestPalindrome(string s) {
        map<string, int> tmp;
        int dp_max = 0;
        for(int i = 0; i < s.length(); i++) {
            for(auto it = map<string, int>::iterator(); it != tmp.end(); it++) {
                string key = it->first + s[i];
                auto exist = tmp.find(key);
                if(exist != tmp.end()) {
                    if(dp_max < tmp[key])
                        dp_max = tmp[key];
                } else {
                    if(checkPalindrome(key)) {
                        tmp[key] = key.length();
                    } else {
                        tmp[key] = 0;
                    }
                }
            }
            string cs(1, s[i]);
            tmp[cs] = 1;
            if(dp_max == 0) dp_max = 1;
        }
        
        for(auto it = map<string, int>::iterator(); it != tmp.end(); it++) {
            if(it->second == dp_max) {
                return it->first;
            }
        }
        return "";
    }
};
class Solution {
public:
    bool checkPalindrome(string s) {
        int j = s.length() - 1;
        int i = 0;
        while(i < j) {
            if(s[j] != s[i]) {
                return false;
            }
            i++;
            j--;
        }
        return true;
    }

    string longestPalindrome(string s) {
        if(s.length() == 0) return "";

        map<string, int> tmp;
        int dp_max = 0;
        for(int i = 0; i < s.length(); i++) {
            cout << s[i];
            for(auto it = tmp.begin(); it != tmp.end(); it++) {
                string key = it->first + s[i];
                auto exist = tmp.find(key);
                if(exist != tmp.end()) {
                    if(dp_max < tmp[key])
                        dp_max = tmp[key];
                } else {
                    if(checkPalindrome(key)) {
                        tmp[key] = key.length();
                    } else {
                        tmp[key] = 0;
                    }
                }
            }
            string cs(1, s[i]);
            tmp[cs] = 1;
            if(dp_max == 0) dp_max = 1;
        }
        cout << tmp.size();
        
        for(auto it = tmp.begin(); it != tmp.end(); it++) {
            if(it->second == dp_max) {
                return it->first;
            }
        }
        return "";
    }
};

class Solution {
public:
    string longestPalindrome(string s) {
        int n = s.size();
        if(n < 2) return s;
        //动态规划的表格，记录数据
        vector<vector<bool>> dp(n, vector<bool>(n));

        //只有一个字符的子串是回文串
        for(int i = 0; i < n; i++) {
            dp[i][i] = true;
        }

        int max = 1; //由于单个字符是回文串，因此初始值是1
        int start = 0; //记录子串的起始位置

        //遍历所有字符串的长度
        for(int l = 2; l <= n; l++) {
            // l = j - i + 1，遍历i即可遍历j
            for(int i = 0; i < n; i++) {
                int j = l + i - 1;
                if(j >= n) break;
                //一定不是回文串
                if (s[i] != s[j]) {
                    dp[i][j] = false;
                } else {
                    //s[i] == s[j], i + 1 = j ,设置true
                    if (j - i < 3) {
                        dp[i][j] = true;
                    } else { //否则取递推的公式
                        dp[i][j] = dp[i + 1][j - 1];
                    }
                }
                //直接计算最大值
                if(dp[i][j] && max < (j - i + 1)) {
                    max = j - i + 1;
                    start = i;
                }
            }
        }
    
        //cout << start << ";" << max ;
        return s.substr(start, max);
    }
};