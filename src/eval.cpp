#include <iostream>
#include <boost/regex.hpp>
#include <list>
#include <string>
#include <map>
#include "h1.h"
#define regex boost::regex
#define smatch boost::smatch
using namespace std;

regex emptyline("(\\s*\\$)");
regex comment("(^#.*)");
regex ifopen("(if )");
regex elopen("(else:)");
regex defopen("(def )");
regex fcall("(.*\\w\\(\\))");
regex fret("(return .*\\w)");
regex assig("(.*\\w =)");
regex assig2("(.*\\w=)");
regex var_n("(\\w.*\\b)"); 


list <string> prgLines;
map<string, int> vars_map;
list <int> iflevels;
list <string> mutvars;


void ifelcounter(string line, int &ifs, int& elses);
bool ifcheck(string line);
bool elcheck(string line);
int bodychecker(list<string>::iterator it, int level);
void bodypasser(list<string>::iterator &it, int level);
bool defcheck(string line);
void recursiveCheck(list<string>::iterator &it, string value, string function);
void ifrecursive(list<string>::iterator &it, string functionName);


void passPrg(list <string> temp) {
	prgLines = temp;
	int ifs = 0;
	int elses = 0;
	list<string>::iterator it;
	map<string, list<string>::iterator> functionNames;
	bool rCompletes = false;
	for (it = prgLines.begin(); it!= prgLines.end(); it++) {//remove comments, they serve no purpose in evaluation 
		string &line = *it;
		smatch m;
		if (regex_search(line, m, comment)) {
			line = "";
		}
	}
	//count ifs and elses, when there are more elses than ifs, the input code is incorrect
	for (it = prgLines.begin(); it != prgLines.end(); it++) {
		ifelcounter(*it, ifs, elses);
	}
	if (elses > ifs) {
		cout << "error: else without if" << endl;
		return;
	}

	int iteration = 0;
	//count if else nesting
	for (it = prgLines.begin(); it != prgLines.end(); it++) {
		int iflevel = 0;
		int elselevel = 0;
		string tmp = *it;
		bool ifb = false;
		bool elb = false;
		if (defcheck(tmp)) {
			bodypasser(it, 1);
		}
		if (ifcheck(tmp)) { //if found
			ifb = true;
			iflevel = bodychecker(next(it), iflevel + 1);
			bodypasser(it, 1);
			it++;
			tmp = *it;
		}
		if (elcheck(tmp)) { //else found
			elb = true;
			elselevel = bodychecker(next(it), elselevel + 1);
			bodypasser(it, 1);
		}
		if (elb) {
			if (!ifb) {
				cout << "error: else without if" << endl;
				return;
			}
		}
		if (iflevel > 0 || elselevel > 0) { //push deepest statement between if/else pair
			int t;
			(iflevel > elselevel) ? (t = iflevel) : (t = elselevel);
			iflevels.push_back(t);
		}
		iteration++;
	}


	//count all potential mutations
	smatch m;
	for (it = prgLines.begin(); it != prgLines.end(); it++) {
		string tmp = *it;
		if (defcheck(tmp)) {
			bodypasser(it, 1);
		}
		if (!regex_search(tmp, m, ifopen) && regex_search(tmp, m, assig)) {
			bool localvar = false;
			size_t sp = tmp.find_first_not_of(" ");
			if (sp != 0) //if there is any indentation, the variable must be local
				localvar = true;
			tmp = tmp.substr(sp);
			sp = tmp.find_first_of("=");
			tmp = tmp.substr(0, sp);
			tmp.erase(remove(tmp.begin(), tmp.end(), ' '), tmp.end());
			if (vars_map.find(tmp) != vars_map.end()) { //if the variable is found in the map, there is a mutation
				vars_map[tmp] += 1;
				if (vars_map[tmp] == 1) //add mutated var to list if it does not already exist in the list
					mutvars.push_back(tmp);
			}
			else { //the variable is not found in the map and if it is not a local variable, it is added to the map
				if(!localvar)		
					vars_map[tmp] = 0;
			}
		}
		if (!regex_search(tmp, m, ifopen) && regex_search(tmp, m, assig2)) {
                        bool localvar = false;
                        size_t sp = tmp.find_first_not_of(" ");
                        if (sp != 0) //if there is any indentation, the variable must be local
                                localvar = true;
                        tmp = tmp.substr(sp);
                        sp = tmp.find_first_of("=");
                        tmp = tmp.substr(0, sp);
                        tmp.erase(remove(tmp.begin(), tmp.end(), ' '), tmp.end());
                        if (vars_map.find(tmp) != vars_map.end()) { //if the variable is found in the map, there is a mutation
                                vars_map[tmp] += 1;
                                if (vars_map[tmp] == 1) //add mutated var to list if it does not already exist in the list
                                        mutvars.push_back(tmp);
                        }
                        else { //the variable is not found in the map and if it is not a local variable, it is added to the map
                                if(!localvar)
                                        vars_map[tmp] = 0;       
                        }        
                }

	}
	
	//check recursion
	int lineNum = 0;
	for (it = prgLines.begin(); it != prgLines.end(); it++) {
		lineNum++;
		string line = *it;
		string name="";
		int x = 4;
		bool hasvariable = false;
		string functionVariable="";

		if(line[0]=='d'&&line[1]=='e'&&line[2]=='f'){
			while(line[x]!='('){
				name+=line[x];
				x++;
			}
			functionNames.insert(pair<string, list<string>::iterator>(name, it));
			//cout << "Function line: " << *functionNames.at(name) << endl;
			//cout << "Function name: " << name << endl;
		}
		size_t find = line.find('(');
		if(find != string::npos){
			name = "";
			int y = 0;
			int z;
			while(line[y]!='('){
				name+=line[y];
				y++;
				if(line[y]=='('&&line[y+1]!=')'){
					hasvariable=true;
					z=y+1;
					while(line[z]!=')'){
						functionVariable+=line[z];
						z++;
					}
				}
				else
				if(line[y]=='('&&line[y+1]==')'){
					hasvariable=false;
				}
			}
			//cout << "Could be a function name: " << name << endl;
		}
		if(functionNames.find(name)!=functionNames.end()){
			//cout << "Function: " << name << " does exit" << endl;
			if(hasvariable){
				//TODO first check if variable isnt another function first
				recursiveCheck(functionNames.at(name), functionVariable, name);
			}
			if(!hasvariable){
				ifrecursive(functionNames.at(name), name);
			}
			
		}
		//cout << lineNum << ": " << *it << endl;
		
	}
	
	//--------------------------------------------------------------------------------------------------------------
	//print mutations
	if (!mutvars.empty()) {
		cout << "Mutated vars: ";
		string opt = "";
		for (it = mutvars.begin(); it != mutvars.end(); it++) {
			opt += *it + ", ";
		}
		cout << opt.substr(0, opt.length() - 2) << endl;
	}

	//print if/else levels
	list<int>::iterator lv;
	if (!iflevels.empty()) {
		cout << "Nested if/else level: ";
		string opt = "";
		for (lv = iflevels.begin(); lv != iflevels.end(); lv++) {
			opt += to_string(*lv - 1) + " level, ";
		}
		cout << opt.substr(0, opt.length() - 2) << endl;
	}
}

bool ifcheck(string line) { //check line for if statement
	smatch m;
	if (regex_search(line, m, ifopen))
		return true;
	return false;
}

bool elcheck(string line) { //check line for else statement
	smatch m;
	if (regex_search(line, m, elopen))
		return true;
	return false;
}

bool defcheck(string line) { //check line for function
	smatch m;
	if (regex_search(line, m, defopen))
		return true;
	return false;
}

void ifelcounter(string line, int &ifs, int &elses) { //count ifs and elses
	if (ifcheck(line))
		ifs++;
	else if (elcheck(line))
		elses++;
	else
		return;
}

void bodypasser(list<string>::iterator &it, int level) {//skip over the body of an if/else/def
	int spacecount = level * 3;
	string cmp = string(spacecount, ' ');
	string line = *it;
	int count = 0;

	do {
		it++;
		count++;
		line = *it;
		if (it == prgLines.end())
			break;
	} while (line.length() > spacecount && line.substr(0, spacecount) == cmp);
	it = prev(it);
}

int bodychecker(list<string>::iterator it, int level) { //recursively checks if/else body to find nested if/else statements, returns furthest depth of a nested statement
        int spacecount = level * 3;
        string cmp = string(spacecount, ' ');
	string line = *it;

	int maxlv = level;

	while (line.length() > spacecount && line.substr(0, spacecount) == cmp) {
		int curr = level;
		int curr2 = level;
		string sub = line.substr(spacecount);
		bool ifb = false;
		bool elb = false;
		if (ifcheck(sub)) {
			ifb = true;
			curr++;
			curr = bodychecker(next(it), curr);
			bodypasser(it, level);
			it++;
			sub = *it;
		}		
		if (elcheck(sub)) {
			elb = true;
			curr2++;
			curr2 = bodychecker(next(it), curr);
			bodypasser(it, curr);
		}
		if (elb) {
			if (!ifb) {
				cout << "error: else without if" << endl;
				exit(-1);
			}
		}
		int t = level;
                (curr > curr2) ? (t = curr) : (t = curr2);
		if (t > maxlv)
			maxlv = t;

		it++;
		if (it == prgLines.end())
			break;
		line = *it;
	}

	return maxlv;
}

void ifrecursive(list<string>::iterator &it, string functionName) {
	it++;
	string line = *it;
	stringstream ss(line);
	string word;
	
	while(line[0]==' '){
		stringstream ss(line);
		while (ss >> word){
			if(word == "return"){
				//cout << " return line: " << line << endl;
				ss >> word;
				if(word==functionName+"()"){
					cout << "Recursive Function " << functionName << "() Ends: No" << endl;
					return;
				}
			}
		}
		it++;
		line=*it;
	}
}

void recursiveCheck(list<string>::iterator &it, string value, string function) {
	string lineOne = *it;
	string variableName="";
	string variableName2="";
	string comparison;
	string lineCheck;
	string comparisontype="";
	bool checkvalue1 = false;
	bool checkvalue2 = false;
	int value1;
	int value2;
	string temp;

	bool start = false;
	bool twovariables = false;
	for(unsigned int i = 0; i<lineOne.length(); i++) {
		if (lineOne[i]=='(') {
			start = true;
			i++;
		}
		if (lineOne[i]==')') { 
			start = false;
			break;
		}
		if(start){
			if(lineOne[i]==','){
				twovariables=true;
				i++;
			}
			if(!twovariables){
				variableName+=lineOne[i];
			}
			if(twovariables){
				variableName2+=lineOne[i];
			}
			
		}
	}
	if(!twovariables){
		value1=stoi(value);
		//cout << "Recursive Variable Name: " << variableName << endl;
		//cout << "Recursive Variable Value: " << value1 << endl;
	}

	if(twovariables){
		bool var1 = true;
		string temp="";
		string temp2="";
		for(int x = 0; x < value.length(); x++){
			if (value[x]==','){
				var1=false;
				x++;
			}
			if(var1){
				temp+=value[x];
			}
			else{
				temp2+=value[x];
			}
		}
		value1=stoi(temp);
		value2=stoi(temp2);
		//cout << "Recursive Variable Name: " << variableName << endl;
		//cout << "Recursive Variable2 Name: " << variableName2 << endl;
		//cout << "Recursive Variables Value: " << value1 << " and " << value2 << endl;
	}
	
	it++;
	lineCheck=*it;
	while (lineCheck[0]==' '){
		stringstream s(lineCheck);
		string temp;
		int comparisonValue;

		s >> temp;
		if(temp == "if" || temp == "elif"){
			string temporary="";
			s >> temp;
			for(int a = 0; a < temp.length(); a++){
				temporary+=temp[a];
				if(temporary==variableName){
					checkvalue1=true;
					//cout << temporary << endl;
					temporary="";
				}
				if(temporary==variableName2){
					checkvalue2=true;
					//cout << temporary << endl;
					temporary="";
				}
				if(temp[a]=='='||temp[a]=='>'||temp[a]=='<'){
					comparisontype+=temp[a];
					temporary="";
				}
			}
			comparisonValue=stoi(temporary);
			//cout << "compared type: " << comparisontype << endl;
			//cout << "compared value: " << comparisonValue << endl;
			if(checkvalue1){
				if(comparisontype=="=="&&comparisonValue==value1){
					cout << "Recursive Function " << function << "(" << value << ") Ends: Yes" << endl; 
					return;
				}
			}
			if(checkvalue2){
				if(comparisontype=="=="&&comparisonValue==value2){
					cout << "Recursive Function " << function << "(" << value << ") Ends: Yes" << endl;
					return;
				}
			}
			
		}
		if(temp=="return"){
			while(s >> temp){
				string checkforfunction="";
				for(int b = 0; b < temp.length(); b++){
					checkforfunction+=temp[b];
					if(checkforfunction==function){					//only happens if its a recursive function, 
						cout << "Recursive Function " << function << "(" << value << ") Ends: ";        //else it wont output because it is not a recursive function
						if(twovariables){
							//cout <<"Two Variables!" << endl;
							if(checkvalue1==true){
								//cout << "Check Value 1" << endl;
								string var="";
								string number="";
								string operatorsymbol="";
								int num;
								b+=2;
								while(temp[b]!=','){
									var+=temp[b];
									if(var==variableName){
										b++;
										if(temp[b]==','){
											cout<<"No"<< endl; //recursive function does not end because variable was not modified
											return;
										}
										else{
											operatorsymbol+=temp[b];
											//cout << "Variable in return function: "<< var << endl;
											//cout << "Operator symbol for variable: "<< operatorsymbol << endl;
											while(temp[b+1]!=','){
												b++;
												number+=temp[b];
											}
											num=stoi(number);
											//cout << "number that modifies variable = " << num << endl;
											if(comparisontype=="=="){
												//cout << "inside == comparison"<< endl;
												if(value1%num==0){
													if(value1==comparisonValue){
														cout << "Yes" << endl; // recursion ends because already meets comparison
														return;
													}
													if(operatorsymbol=="-"){
														if(value1>comparisonValue && num>0){
															//cout << value1 << endl;
															//cout << num << endl;
															cout << "Yes"<< endl;
															return;
														}
														else
														if(value1<comparisonValue && num>0){
															cout << "No" << endl;
															return;
														}
													}
													if(operatorsymbol=="+"){
														if(value1<comparisonValue && num>0){
															cout << "Yes"<< endl;
															return;
														}
														else
														if(value1>comparisonValue && num>0){
															cout << "No" << endl;
															return;
														}
													}
												}
												else{
													cout << "No, ERROR in code" << endl;
												}
											}
											if(comparisontype=="<"||comparisontype=="<="){
												if(value1<=comparisonValue){
													cout << "Yes" << endl; // recursion ends because already meets comparison
													return;
												}
												else{
													if(operatorsymbol=="-" && num>0){
														cout << "Yes" << endl;
													}
													if(operatorsymbol=="+"&& num>0){
														cout << "No" << endl;
													}
												}

											}
											if(comparisontype==">"||comparisontype==">="){
												//cout << "HERE" << endl;
												if(value1>=comparisonValue){
													cout << "Yes" << endl; // recursion ends because already meets comparison
													return;
												}
												else{
													if(operatorsymbol=="+" && num>0){
														cout << "Yes" << endl;
													}
													if(operatorsymbol=="-"&& num>0){
														cout << "No" << endl;
													}
												}
											}
										}
									}
									b++;
								}
							}
							if(checkvalue2){
								//cout << "Check Value 2" << endl;
								string var="";
								string number="";
								string operatorsymbol="";
								bool begin = false;
								int num;
							
								b+=2;
								while(!begin){
									if(temp[b]==','){
										begin=true;
										//cout << "second variable begin"<< endl;
										b++;
										break;
									}
									b++;
								}
								while(temp[b]!=')'){
									var+=temp[b];
									if(var==variableName2){
										//cout << "Variable in return function: "<< var << endl;										
										b++;
										if(temp[b]==')'){
											cout<<"No"<< endl; //recursive function does not end because variable was not modified
											return;
										}
										else{
											operatorsymbol+=temp[b];
											//cout << "Variable in return function: "<< var << endl;
											//cout << "Operator symbol for variable: "<< operatorsymbol << endl;
											while(temp[b+1]!=')'){
												b++;
												number+=temp[b];
											}
											num=stoi(number);
											//cout << "number that modifies variable = " << num << endl;
											if(comparisontype=="=="){
												//cout << "inside == comparison"<< endl;
												if(value2%num==0){
													if(value2==comparisonValue){
														cout << "Yes" << endl; // recursion ends because already meets comparison
														return;
													}
													if(operatorsymbol=="-"){
														if(value2>comparisonValue && num>0){
															//cout << value2 << endl;
															//cout << num << endl;
															cout << "Yes"<< endl;
															return;
														}
														else
														if(value2<comparisonValue && num>0){
															cout << "No" << endl;
															return;
														}
													}
													if(operatorsymbol=="+"){
														if(value2<comparisonValue && num>0){
															cout << "Yes"<< endl;
															return;
														}
														else
														if(value2>comparisonValue && num>0){
															cout << "No" << endl;
															return;
														}
													}
												}
												else{
													cout << "No, ERROR in code" << endl;
												}
											}
											if(comparisontype=="<"||comparisontype=="<="){
												if(value2<=comparisonValue){
													cout << "Yes" << endl; // recursion ends because already meets comparison
													return;
												}
												else{
													if(operatorsymbol=="-" && num>0){
														cout << "Yes" << endl;
													}
													if(operatorsymbol=="+"&& num>0){
														cout << "No" << endl;
													}
												}

											}
											if(comparisontype==">"||comparisontype==">="){
												if(value2>=comparisonValue){
													cout << "Yes" << endl; // recursion ends because already meets comparison
													return;
												}
												else{
													if(operatorsymbol=="+" && num>0){
														cout << "Yes" << endl;
													}
													if(operatorsymbol=="-"&& num>0){
														cout << "No" << endl;
													}
												}
											}
										}
									}
									b++;
								}
							}
						}
						if(!twovariables){
							string var="";
							string number="";
							string operatorsymbol="";
							int num;
							b+=2;
							while(temp[b]!=')'){
								var+=temp[b];
								if(var==variableName){
									b++;
									if(temp[b]==')'){
										//////////////////////////////////////////////
										cout<<"No...."<< endl; //recursive function does not end because variable was not modified
										return;
									}
									else{
										operatorsymbol+=temp[b];
										//cout << "Variable in return function: "<< var << endl;
										//cout << "Operator symbol for variable: "<< operatorsymbol << endl;
										while(temp[b+1]!=')'){
											b++;
											number+=temp[b];
										}
										num=stoi(number);
										//cout << "number that modifies variable = " << num << endl;
										if(comparisontype=="=="){
											//cout << "inside == comparison"<< endl;
											if(value1==comparisonValue){
												cout << "Yes" << endl; // recursion ends because already meets comparison
												return;
												}
											if(value1%num==0){
												if(operatorsymbol=="-"){
													if(value1>comparisonValue && num>0){
														//cout << value1 << endl;
														//cout << num << endl;
														cout << "Yes"<< endl;
														return;
													}
													else
													if(value1<comparisonValue && num>0){
														cout << "No" << endl;
														return;
													}
												}
												if(operatorsymbol=="+"){
													if(value1<comparisonValue && num>0){
														cout << "Yes"<< endl;
														return;
													}
													else
													if(value1>comparisonValue && num>0){
														cout << "No" << endl;
														return;
													}
												}
											}
											else{
												cout << "No, ERROR in code" << endl;
											}
										}
										if(comparisontype=="<"||comparisontype=="<="){
												if(value1<=comparisonValue){
													cout << "Yes" << endl; // recursion ends because already meets comparison
													return;
												}
												else{
													if(operatorsymbol=="-" && num>0){
														cout << "Yes" << endl;
													}
													if(operatorsymbol=="-"&& num<0){
														cout << "No" << endl;
													}
												}

											}
										if(comparisontype==">"||comparisontype==">="){
											if(value1>=comparisonValue){
												cout << "Yes" << endl; // recursion ends because already meets comparison
												return;
											}
											else{
													if(operatorsymbol=="+" && num>0){
														cout << "Yes" << endl;
													}
													if(operatorsymbol=="+"&& num<0){
														cout << "No" << endl;
													}
												}
										}
									}
								}
								b++;
							}
						}
					}
				}
			}
		}
		//cout << lineCheck << endl;
		it++;
		lineCheck=*it;
	}
	return;
}




