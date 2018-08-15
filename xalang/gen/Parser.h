

#if !defined(xalang_parser_COCO_PARSER_H__)
#define xalang_parser_COCO_PARSER_H__

#include <wchar.h>
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>
using namespace std;

#include "Code.h"
using namespace xalang;


#include "Scanner.h"

namespace xalang_parser {


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
		_t_eol=1,
		_t_ident=2,
		_t_integer=3,
		_t_real=4,
		_t_string=5,
		_t_priv=6,
		_t_comp=7,
		_t_sep=8,
		_t_decl=9,
		_t_params_start=10,
		_t_params_end=11,
		_t_par_start=12,
		_t_par_end=13,
		_t_module=14,
		_t_true=15,
		_t_false=16,
		_t_null=17
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

	wstring tokenText(){
		return coco_string_create(t->val);
	}

	bool isDecl() {
		scanner->ResetPeek();
		Token* next = scanner->Peek();
		while(next->kind == _t_ident || next->kind == _t_comp)
			next = scanner->Peek();
		return la->kind == _t_ident && next->kind == _t_decl;
	} 
	bool isFunc() {
		scanner->ResetPeek();
		Token *next = scanner->Peek();
		while(next->kind == _t_ident || next->kind == _t_comp)
			next = scanner->Peek();
		return la->kind == _t_ident && next->kind == _t_params_start;
	} 

	// Code generation

	Module root;


/*--------------------------------------------------------------------------*/


	Parser(Scanner *scanner);
	~Parser();
	void SemErr(const wchar_t* msg);

	void value(Value &val);
	void ident(Ident &t);
	void symbol(Symbol& t);
	void Xalang();
	void block(Scope &s);
	void list(Scope &s);
	void item(Scope &s);
	void declaration(Declaration& d);
	void evaluation(Evaluation& e);
	void layout(Scope& s);
	void elem(Scope& s);

	void Parse();

}; // end Parser

} // namespace


#endif

