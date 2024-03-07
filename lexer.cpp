#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <stack>

using namespace std;

struct IDENTIFIER {
	string name;
	string type;
	bool is_define;
	int num;
};

class Resolver {
	/*1*/map<string, int> TW{{"or", 1}, {"and", 2}, {"not", 3}, {"if", 4}, {"else", 5}, {"for", 6}, {"to", 7}, {"while", 8}, {"read", 9}, {"write", 10}, {"integer", 11}, {"real", 12}, {"boolean", 13}, {"as", 14}, {"do", 15}, {"then", 16}, {"true", 17}, {"false", 18}};
	/*2*/map<string, int> TL{ {"{", 1}, {"}", 2}, {"(", 3}, {")", 4}, {"[", 5}, {"]", 6}, {";", 7}, {":", 8}, {",", 9}, {"+", 10}, {"-", 11}, {"*", 12}, {"/", 13}, {"<>", 14}, {"=", 15}, {"<", 16}, {"<=", 17}, {">", 18}, {">=", 19}, {"endl", 20} };
	/*3*/map<string, int> TN;
	/*4*/map<string, int> TI;

	vector<IDENTIFIER> semanticTI;

	enum state {
		H,
		ID,
		NUM2,
		NUM8,
		NUM10,
		NUM16,
		C1,
		C2,
		C3,
		M1,
		M2,
		P1,
		P2,
		B,
		O,
		D,
		HX,
		E11,
		E12,
		E13,
		E22,
		ZN,
		E21,
		OG,
		V,
		ERR,
	};
	string S;
	char CH;
	ifstream fd;
	ofstream outputFile;
	int z;
	ifstream fin;
	string LEX;
	bool first_record = true;


	bool gl() {
		if (fin.eof()) {
			LEX = "@";
			//cout << LEX << endl;
			return true;
		}
		fin >> LEX;
		int n, k;
		n = stoi(LEX.substr(1, 1));
		k = stoi(LEX.substr(3, LEX.size() - 4));
		switch (n) {
		case 1:
			LEX = findKeysByValue(TW, k);
			break;
		case 2:
			LEX = findKeysByValue(TL, k);
			break;
		case 3:
			LEX = findKeysByValue(TN, k);
			break;
		case 4:
			LEX = findKeysByValue(TI, k);
			break;
		}
		//cout << LEX << endl;
		return true;
	}
	bool is_eq(string S) {
		return S == LEX;
	}
	bool id_check() {
		if (TW.find(LEX) != TW.end())
			return false;
		if (!isalpha(LEX[0]))
			return false;
		for (char c : LEX) {
			if (!isalpha(c) && !isdigit(c))
				return false;
		}
		return true;
	}
	bool num_check() {
		regex regular("^[0-9]*[.,]?[0-9]+$");
		if (regex_match(LEX, regular)) {
			return true;
		}
		return false;
	}
	void err_proc(int numb_error) {
		switch (numb_error) {
		case 1:
			cout << "Ошибка отсутствует символ { в PROG" << endl;
			break;
		case 2:
			cout << "Ошибка отсутствует ID после ',' в DEF" << endl;
			break;
		case 3:
			cout << "Ошибка отсутствует : в DEF" << endl;
			break;
		case 4:
			cout << "Ошибка некорректный тип данных" << endl;
			break;
		case 5:
			cout << "Ошибка отсутствует ; в PROG" << endl;
			break;
		case 6:
			cout << "Ошибка отсутствует } в PROG" << endl;
			break;
		case 7:
			cout << "Ошибка после } не должно быть символов" << endl;
			break;
		case 8:
			cout << "Ошибка OPER не определена" << endl;
			break;
		case 9:
			cout << "Ошибка после read должна быть (" << endl;
			break;
		case 10:
			cout << "Ошибка в аргументах read должен быть ID" << endl;
			break;
		case 12:
			cout << "Ошибка отсутствует ) после read(" << endl;
			break;
		case 13:
			cout << "Ошибка OPER в COMP должны разделяться через : или endl" << endl;
			break;
		case 14:
			cout << "Ошибка отсутствует ] в COMP" << endl;
			break;
		case 15:
			cout << "Ошибка отсутствует as в ASSGN" << endl;
			break;
		case 16:
			cout << "Ошибка отсутствует then в COND" << endl;
			break;
		case 17:
			cout << "Ошибка отсутствует do в FL" << endl;
			break;
		case 18:
			cout << "Ошибка отсутствует to в FL" << endl;
			break;
		case 19:
			cout << "Ошибка отсутствует do в CL" << endl;
			break;
		case 20:
			cout << "Ошибка отсутствует ) после write(" << endl;
			break;
		case 21:
			cout << "Ошибка после write должна быть (" << endl;
			break;
		case 22:
			cout << "Ошибка отсутствует ) в MULT" << endl;
			break;
		case 23:
			cout << "Ошибка в MULT" << endl;
		case 24:
			cout << "Ошибка в PROG должно быть минимум один DEF или OPER" << endl;
			break;
		case 25:
			cout << "Ошибка переменная уже была определена" << endl;
			cout << "SEMANTIC INCORRECT";
			exit(1);
			break;
		case 26:
			cout << "Ошибка использование неопределенной переменной" << endl;
			cout << "SEMANTIC INCORRECT";
			exit(1);
			break;
		}
		//cout << numb_error << endl;
		cout << "SYNTAX INCORRECT";
		exit(1);
	}
	void PROG() {
		gl();
		if (is_eq("{")) {
			gl();
			if (is_eq("}")) {
				err_proc(24);
			}
			while (!is_eq("}") && !is_eq("@")) {	
				if (id_check()) {
					gl();
					if (is_eq("as")) {
						OPER();
					}
					else {
						DEF();
					}		
				}
				else {
					OPER(); // Проверить норм ли все с ;
				}
				if (!is_eq(";")) { // В конце OPER и DEF должен считывать лексему
					err_proc(5);
				}
				gl();
			}
			if (is_eq("}")) {
				gl();
				if (is_eq("@")) {
					cout << "SYNTAX CORRECT" << endl;
				}
				else {
					err_proc(7);
				}	
			}
			else {
				err_proc(6);
			}		
		}
		else
			err_proc(1);
	}
	void DEF() {
		while (is_eq(",")) {
			gl();
			if (!id_check()) {
				err_proc(2);
			}
			gl();
		}
		if (is_eq(":")) {
			gl();
			if (is_eq("integer") || is_eq("real") || is_eq("boolean")) {
				gl();
			}
			else {
				err_proc(4);
			}
		}
		else {
			err_proc(3);
		}
	}
	void OPER() {
		if (is_eq("[")) {
			COMP();
			gl();
		}
		else if (id_check() || is_eq("as")) {
			ASSGN();
		}
		else if (is_eq("if")) {
			COND();
		}
		else if (is_eq("for")) {
			FL();
		}
		else if (is_eq("while")) {
			CL();
		}
		else if (is_eq("read")) {
			IN();
			gl();
		}
		else if (is_eq("write")) {
			OUT();
			gl();
		}
		else {
			err_proc(8);
		}
		
	}
	void COMP() {
		gl();
		OPER();
		while (!is_eq("]") && !is_eq("@")) {
			if (!is_eq(":") && !is_eq("endl")) { // Сделать лексему переноса строки или попросить без нее
				err_proc(13);
			}
			gl();
			OPER();
		}
		if (!is_eq("]")) {
			err_proc(14);
		}
	}
	void ASSGN() {
		if (id_check()) {
			gl();
		}
		if (is_eq("as")) {
			gl();
			EXP();
		}
		else {
			err_proc(15);
		}
	}
	void COND() {
		gl();
		EXP();
		if (is_eq("then")) {
			gl();
			OPER();
			if (is_eq("else")) {
				gl();
				OPER();
			}
		}
		else {
			err_proc(16);
		}
	}
	void FL() {
		gl();
		ASSGN();
		if (is_eq("to")) {
			gl();
			EXP();
			if (is_eq("do")) {
				gl();
				OPER();
			}
			else {
				err_proc(17);
			}
		}
		else {
			err_proc(18);
		}
	}
	void CL() {
		gl();
		EXP();
		if (is_eq("do")) {
			gl();
			OPER();
		}
		else {
			err_proc(19);
		}

	}
	void IN() {
		gl();
		if (is_eq("(")) {
			gl();
			if (!id_check()) {
				err_proc(10);
			}
			gl();
			while (is_eq(",")) {
				gl();
				if (!id_check()) {
					err_proc(10);
				}
				gl();
			}
			if (!is_eq(")")) {
				err_proc(12);
			}
		}
		else {
			err_proc(9);
		}
	}
	void OUT() {
		gl();
		if (is_eq("(")) {
			gl();
			EXP();
			while (is_eq(",")) {
				gl();
				EXP();
			}
			if (!is_eq(")")) {
				err_proc(20);
			}
		}
		else {
			err_proc(21);
		}
	}


	void EXP() {
		OPR();
		while (is_eq("<>") || is_eq("=") || is_eq("<") || is_eq("<=") || is_eq(">") || is_eq(">=")) {
			gl();
			OPR();
		}
	}
	void OPR() {
		TERM();
		while (is_eq("+") || is_eq("-") || is_eq("or")) {
			gl();
			TERM();
		}
	}
	void TERM() {
		MULT();
		while (is_eq("*") || is_eq("/") || is_eq("and")) {
			gl();
			MULT();
		}
	}
	void MULT() {
		if (id_check() || num_check() || is_eq("true") || is_eq("false")) {
			gl();
		}
		else if (is_eq("not")) {
			gl();
			MULT();
		}
		else if (is_eq("(")) {
			gl();
			EXP();
			if (!is_eq(")")) {
				err_proc(22);
			}
			gl();
		}
		else {
			err_proc(23);
		}
	}


	void gc() {
		if (fd)
			CH = fd.get();
		else {
			cout << "Ошибка открытия файла";
		}
	}
	bool let() {
		return isalpha(CH);
	}
	bool digit() {
		return isdigit(CH);
	}
	void nill() {
		S.clear();
	}
	void add() {
		S += CH;
	}
	void look(map<string, int>& t) {
		z = t[S];
		if (z == 0)
			t.erase(S);
	}
	void put(map<string, int>& t) {
		look(t);
		if (z == 0) {
			t[S] = t.size() + 1;
			z = t.size();
		}
	}

	string findKeysByValue(map<string, int>& myMap, int value) {
		for (auto& pair : myMap) {
			//int n = pair.second;
			if (pair.second == value) {
				return pair.first;
			}
		}
		return "";
	}




	void out(int n, int k) {
		string out = "(" + to_string(n) + "," + to_string(k) + ")";
		if (first_record) {
			outputFile << out;
			first_record = false;
		}		
		else 
			outputFile << endl << out;
		////cout << S << " " << out << " " <<  endl;
		//switch (n) {
		//case 1:
		//	cout << S << " " << out << " " << TW[S] << endl;
		//	//cout << findKeysByValue(TW, k) << endl;
		//	break;
		//case 2:
		//	cout << S << " " << out << " " << TL[S] << endl;
		//	//cout << findKeysByValue(TL, k) << endl;
		//	break;
		//case 3:
		//	cout << S << " " << out << " " << TN[S] << endl;
		//	//cout << findKeysByValue(TN, k) << endl;
		//	break;
		//case 4:
		//	cout << S << " " << out << " " << TI[S] << endl;
		//	//out << findKeysByValue(TI, k) << endl;
		//	break;
		//}
	}
	bool check_hex() {
		return digit() || ('A' <= CH && CH <= 'F') || ('a' <= CH && CH <= 'f');
	}
	bool AFH() {
		return  ('A' <= CH && CH <= 'F') || ('a' <= CH && CH <= 'f') || (CH == 'H') || (CH == 'h');
	}
	void translate(int base) {
		int num = 0;
		switch (base) {
		case 2:
			for (int i = 0; i < S.size() - 1; ++i) {
				num += (S[i] - '0') * pow(2, S.size() - 2 - i);
			}
			break;
		case 8:
			for (int i = 0; i < S.size() - 1; ++i) {
				num += (S[i] - '0') * pow(8, S.size() - 2 - i);
			}
			break;
		case 16:
			map<char, int> vals{ {'A', 10}, {'B', 11}, {'C', 12}, {'D', 13}, {'E', 14}, {'F', 15}, {'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15} };
			for (int i = 0; i < S.size() - 1; ++i) {
				if (isdigit(S[i])) {
					num += (S[i] - '0') * pow(16, S.size() - 2 - i);
				}
				else {
					num += (vals[S[i]]) * pow(16, S.size() - 2 - i);
				}
			}
			break;
		}
		nill();
		S = to_string(num);
	}
	void convert() {
		istringstream iss(S);
		double decimalNumber;
		iss >> decimalNumber;
		ostringstream oss;
		oss << fixed << setprecision(15) << decimalNumber;
		S = oss.str();
	}

	int get_lex_id() {
		return TI[LEX];
	}

public:
	void gl_checker() {
		while (!is_eq("@")) {
			gl();
			cout << LEX << endl;
		}
	}
	Resolver(string fileName) {
		fd.open(fileName);
		outputFile.open("out.txt");
	}
	void semantic() {
		LEX.clear();
		fin.open("out.txt");
		for (auto item : TI) {
			IDENTIFIER id;
			id.name = item.first;
			id.num = item.second;
			id.is_define = false;
			semanticTI.push_back(id);
		}
		stack<int> id_stack;
		gl();
		while (true) {
			while (!id_check() && !is_eq("@"))
				gl();
			if (is_eq("@")) break;
			id_stack.push(semanticTI[get_lex_id() - 1].num);
			gl();
			while (is_eq(",")) {
				gl();
				id_stack.push(semanticTI[get_lex_id() - 1].num);
				gl();
			}
			if (is_eq(":") && gl() && (is_eq("integer") || is_eq("real") || is_eq("boolean"))) {
				while (!id_stack.empty()) {
					if (semanticTI[id_stack.top() - 1].is_define) {
						err_proc(25);
					}
					semanticTI[id_stack.top() - 1].is_define = true;
					semanticTI[id_stack.top() - 1].type = LEX;
					id_stack.pop();
				}
			}
			else {
				while (!id_stack.empty()) {
					if (!semanticTI[id_stack.top() - 1].is_define) {
						err_proc(26);
					}
					id_stack.pop();
				}
			}
			gl();
		}
		cout << "SEMANTIC CORRECT" << endl;
		fin.close();
	}
	void parser() {
		fin.open("out.txt");
		PROG();
		fin.close();
	}
	bool scanner() {
		state CS;
		gc();
		CS = H;
		do {
			switch (CS) {
			case H:
				while (CH == ' ' || CH == '\n' || CH == '\t' && !fd.eof())
					gc();
				if (fd.eof())
					CS = ERR;
				if (let()) {
					nill();
					add();
					gc();
					CS = ID;
				}
				else if (CH == '0' || CH == '1') {
					nill();
					CS = NUM2;
					add();
					gc();
				}
				else if (CH >= '2' && CH <= '7') {
					nill();
					CS = NUM8;
					add();
					gc();
				}
				else if (CH >= '8' && CH <= '9') {
					nill();
					CS = NUM10;
					add();
					gc();
				}
				else if (CH == '.') {
					nill();
					add();
					gc();
					CS = P1;
				}
				else if (CH == '/') {
					gc();
					CS = C1;
				}
				else if (CH == '<') {
					gc();
					CS = M1;
				}
				else if (CH == '>') {
					gc();
					CS = M2;
				}
				else if (CH == '}') {
					out(2, 2); 
					CS = V;
				}
				else
					CS = OG;
				break;
			case ID:
				while (let() || digit()) {
					add();
					gc();
				}
				look(TW);
				if (z != 0) {
					out(1, z);
					CS = H;
				}
				else {
					put(TI);
					out(4, z);
					CS = H;
				}
				break;
			case NUM2:
				while (CH == '0' || CH == '1') {
					add();
					gc();
				}
				if (CH >= '2' && CH <= '7')
					CS = NUM8;
				else if (CH == '8' || CH == '9')
					CS = NUM10;
				else if (CH == 'A' || CH == 'a' || CH == 'C' || CH == 'c' ||
					CH == 'F' || CH == 'f')
					CS = NUM16;
				else if (CH == 'E' || CH == 'e') {
					add();
					gc();
					CS = E11;
				}
				else if (CH == 'D' || CH == 'd') {
					add();
					gc();
					CS = D;
				}
				else if (CH == 'O' || CH == 'o')
					CS = O;
				else if (CH == 'H' || CH == 'h') {
					gc();
					CS = HX;
				}
				else if (CH == '.') {
					add();
					gc();
					CS = P1;
				}
				else if (CH == 'B' || CH == 'b') {
					add();
					gc();
					CS = B;
				}
				else if (let())
					CS = ERR;
				else
					CS = NUM10;
				break;
			case NUM8:
				while (CH >= '0' && CH <= '7') {
					add();
					gc();
				}
				if (CH == '8' || CH == '9')
					CS = NUM10;
				else if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' ||
					CH == 'c' || CH == 'F' || CH == 'f')
					CS = NUM16;
				else if (CH == 'E' || CH == 'e') {
					add();
					gc();
					CS = E11;
				}
				else if (CH == 'D' || CH == 'd') {
					add();
					gc();
					CS = D;
				}
				else if (CH == 'H' || CH == 'h') {
					add();
					gc();
					CS = HX;
				}
				else if (CH == '.') {
					add();
					gc();
					CS = P1;
				}
				else if (CH == 'O' || CH == 'o') {
					add();
					gc();
					CS = O;
				}
				else if (let())
					CS = ERR;
				else CS = NUM10;
				break;
			case NUM10:
				while (CH >= '0' && CH <= '9') {
					add();
					gc();
				}
				if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' ||
					CH == 'c' || CH == 'F' || CH == 'f')
					CS = NUM16;
				else if (CH == 'E' || CH == 'e') {
					add();
					gc();
					CS = E11;
				}
				else if (CH == 'H' || CH == 'h') {
					add();
					gc();
					CS = HX;
				}
				else if (CH == '.') {
					add();
					gc();
					CS = P1;
				}
				else if (CH == 'D' || CH == 'd') {
					add();
					gc();
					CS = D;
				}
				else if (let())
					CS = ERR;
				else {
					put(TN); 
					out(3, z); 
					CS = H;
				}
				break;
			case NUM16:
				while (check_hex()) {
					add();
					gc();
				}
				if (CH == 'H' || CH == 'h') {
					add();
					gc();
					CS = HX;
				}
				else CS = ERR;
				break;
			case B:
				if (check_hex())
					CS = NUM16;
				else if (CH == 'H' || CH == 'h') {
					add();
					gc(); 
					CS = HX;
				}
				else if (let())
					CS = ERR;
				else {
					//cout << S << endl;
					translate(2);
					put(TN);
					out(3, z); 
					CS = H;
				}
				break;
			case O:
				if (let() || digit())
					CS = ERR;
				else {
					//cout << S << endl;
					translate(8);
					put(TN);
					out(3, z);
					CS = H;
				}
				break;
			case D:
				if (CH == 'H' || CH == 'h') {
					add();
					gc();
					CS = HX;
				}
				else if (check_hex())
					CS = NUM16;
				else if (let())
					CS = ERR;
				else {
					if (S[S.size() - 1] == 'd' || S[S.size() - 1] == 'D') {
						S = S.substr(0, S.size() - 1);
					}
					put(TN);
					out(3, z);
					CS = H; 
				}
					break;
			case HX:
				if (let() || digit())
					CS = ERR;
				else {
					//cout << S << endl;
					translate(16);
					put(TN);
					out(3, z);
					CS = H;
				}
				break;
			case E11:
				if (digit()) {
					add();
					gc();
					CS = E12;
				}
				else if (CH == '+' || CH == '-') {
					add();
					gc();
					CS = ZN;
				}
				else if (CH == 'H' || CH == 'h') {
					add();
					gc();
					CS = HX;
				}
				else if (check_hex()) {
					add();
					gc();
					CS = NUM16;
				}
				else CS = ERR;
				break;
			case ZN:
				if (digit()) {
					add();
					gc();
					CS = E13;
				}
				else CS = ERR;
				break;
			case E12:
				while (digit()) {
					add();
					gc();
				}
				if (check_hex())
					CS = NUM16;
				else if (CH == 'H' || CH == 'h') {
					gc();
					CS = HX;
				}
				else if (let())
					CS = ERR;
				else {
					convert();
					put(TN);
					out(3, z);
					CS = H;
				}
				break;
			case E13:
				while (digit()) {
					add();
					gc();
				}
				if (let() || CH == '.')
					CS = ERR;
				else {
					convert();
					put(TN);
					out(3, z);
					CS = H;
				}
				break;
			case P1:
				if (digit())
					CS = P2;
				else
					CS = ERR;
				break;
			case P2:
				while (digit()) {
					add();
					gc();
				}
				if (CH == 'E' || CH == 'e') {
					add();
					gc();
					CS = E21;
				}
				else if (let() || CH == '.')
					CS = ERR;
				else {
					convert();
					put(TN);
					out(3, z);
					CS = H;
				}
				break;
			case E21:
				if (CH == '+' || CH == '-') {
					add();
					gc();
					CS = ZN;
				}
				else if (digit())
					CS = E22;
				else CS = ERR;
				break;
			case E22:
				while (digit()) {
					add();
					gc();
				}
				if (let() || CH == '.')
					CS = ERR;
				else {
					convert();
					put(TN);
					out(3, z);
					CS = H;
				}
				break;
			case C1:
				if (CH == '*') {
					gc();
					CS = C2;				
				}
				else {	
					out(2, 13);
					CS = H;
				}
				break;
			case C2: 
				while (CH != '*' && CH != '}') {
					gc(); 
				}
				if (CH == '}') 
					CS = ERR;
				else { 
					gc(); 
					CS = C3; 
				}
				break; 
			case C3: 
				if (CH == '/') { 
					gc(); 
					CS = H; 
				}
				else 
					CS = C2;
				break; 
			case M1:
				if (CH == '>') {
					gc();
					out(2, 14);
					CS = H;
				}
				else if (CH == '=') {
					gc();
					out(2, 17); 
					CS = H;
				}
				else {
					out(2, 16);
					CS = H;
				}
				break;
			case M2:
				if (CH == '=') {
					gc();
					out(2, 19); 
					CS = H;
				}
				else {
					out(2, 18); 
					CS = H;
				}
				break;
			case OG:
				nill();
				add();
				look(TL);
				if (z != 0) {
					gc();
					out(2, z); 
					CS = H;
				}
				else CS = ERR;
				break;
			}
		} while (CS != V && CS != ERR); // Почему-то не перенос строки вызывает ERR
		if (CS == ERR) { 
			cout << "Возникла ошибка, символ ошибки: " << CH << endl;
			cout << "Текущая лексема: " << S << endl;
			outputFile.close();
			exit(1);
		}
		outputFile.close();
		return CS;	
	}
	
};

int main() {
	setlocale(LC_ALL, "Russian");
	Resolver* lexer = new Resolver("test3.txt");
	lexer->scanner();
	
	lexer->parser();
	lexer->semantic();
	//lexer->gl_checker();
}

