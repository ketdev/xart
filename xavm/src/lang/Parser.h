

#if !defined(xalang_COCO_PARSER_H__)
#define xalang_COCO_PARSER_H__

#include <wchar.h>
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>
using namespace std;

#include "Code.h"


#include "Scanner.h"

namespace xalang {


class Errors {
public:
	int count;			// number of errors detected

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const wchar_t *s);
	void Warning(int line, int col, const wchar_t *s);
	void Warning(const wchar_t *s);
	void Exception(const wchar_t *s);

}; // Errors

class Parser {
private:
	enum {
		_EOF=0,
		_ident=1,
		_integer=2,
		_real=3,
		_string=4
	};
	int maxT;

	Token *dummyToken;
	int errDist;
	int minErrDist;

	void SynErr(int n);
	void Get();
	void Expect(int n);
	bool StartOf(int s);
	void ExpectWeak(int n, int follow);
	bool WeakSeparator(int n, int syFol, int repFol);

public:
	Scanner *scanner;
	Errors  *errors;

	Token *t;			// last recognized token
	Token *la;			// lookahead token

static inline std::wstring s2ws(const std::string& str) {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.from_bytes(str);
    }
    static inline std::string ws2s(const std::wstring& wstr) {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.to_bytes(wstr);
    }

	// Code generation

	Module code;


/*--------------------------------------------------------------------------*/


	Parser(Scanner *scanner);
	~Parser();
	void SemErr(const wchar_t* msg);

	void Xalang();
	void Ident(wchar_t* &name);
	void Expression();
	void Declaration();
	void Transformation();
	void Evaluation();
	void Element();
	void Parameters();

	void Parse();

}; // end Parser

} // namespace


#endif

