#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <iterator>
#include <vector>

using namespace std;

int main() {
  std::string str = std::string();
  std::map<std::string, int> mapNFC;
  std::map<std::string, int> mapNPG;
  std::map<std::string, int> mapNFA;
  std::map<std::string, std::string> mapS;
  while(cin >> str) {
    std::string strname = std::string();
    int i = 0;
    for (int ie = str.size(); i < ie; i++) {
      if (str[i] == ',')
        break;
      strname = strname + str[i];
    }
    i++;
    int tmpVal = 0;
    for (int ie = str.size(); i < ie; i++) {
      if ((str[i] < '0') || (str[i] > '9'))
        break;
      tmpVal = tmpVal * 10;
      tmpVal = tmpVal + (str[i] - '0');
    }
    i++;
    int tmpVal2 = 0;
    for (int ie = str.size(); i < ie; i++) {
      if ((str[i] < '0') || (str[i] > '9'))
        break;
      tmpVal2 = tmpVal2 * 10;
      tmpVal2 = tmpVal2 + (str[i] - '0');
    }
    i++;
    int tmpVal3 = 0;
    for (int ie = str.size(); i < ie; i++) {
      if ((str[i] < '0') || (str[i] > '9'))
        break;
      tmpVal3 = tmpVal3 * 10;
      tmpVal3 = tmpVal3 + (str[i] - '0');
    }
    if (mapNFC.count(strname) == 0) {
      mapNFC[strname] = tmpVal;
      mapNPG[strname] = tmpVal2;
      mapNFA[strname] = tmpVal3;
    }
    else {
      mapNFC[strname] = tmpVal +  mapNFC[strname];
      mapNPG[strname] = tmpVal2 + mapNPG[strname];
      mapNFA[strname] = tmpVal3 + mapNFA[strname];
      
    }
  }
  cout << "Benchmark, Number of Function Calls, Number of Pragmas Inserted,";
  cout << " Number of Functions Annotated,\n";
  for (std::map<std::string, int>::iterator I = mapNFC.begin(),
       IE = mapNFC.end(); I != IE; I++) {
    cout << I->first << "," << I->second << "," << mapNPG[I->first] << ",";
    cout << mapNFA[I->first] << "," << endl;
  }
  return 0;
}
