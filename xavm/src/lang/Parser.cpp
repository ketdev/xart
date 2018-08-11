

#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"


namespace xalang_parser {


void Parser::SynErr(int n) {
	if (errDist >= minErrDist) errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const wchar_t* msg) {
	if (errDist >= minErrDist) errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::Get() {
	for (;;) {
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT) { ++errDist; break; }

		if (dummyToken != t) {
			dummyToken->kind = t->kind;
			dummyToken->pos = t->pos;
			dummyToken->col = t->col;
			dummyToken->line = t->line;
			dummyToken->next = NULL;
			coco_string_delete(dummyToken->val);
			dummyToken->val = coco_string_create(t->val);
			t = dummyToken;
		}
		la = t;
	}
}

void Parser::Expect(int n) {
	if (la->kind==n) Get(); else { SynErr(n); }
}

void Parser::ExpectWeak(int n, int follow) {
	if (la->kind == n) Get();
	else {
		SynErr(n);
		while (!StartOf(follow)) Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol) {
	if (la->kind == n) {Get(); return true;}
	else if (StartOf(repFol)) {return false;}
	else {
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0))) {
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::value(Value &val) {
		val = {}; 
		switch (la->kind) {
		case _t_integer: {
			Get();
			val.type = Value::Integer; 
			break;
		}
		case _t_real: {
			Get();
			val.type = Value::Real; 
			break;
		}
		case _t_string: {
			Get();
			val.type = Value::String; 
			break;
		}
		case _t_true: {
			Get();
			val.type = Value::Boolean; 
			break;
		}
		case _t_false: {
			Get();
			val.type = Value::Boolean; 
			break;
		}
		case _t_null: {
			Get();
			val.type = Value::Null; 
			break;
		}
		default: SynErr(19); break;
		}
		val.text = tokenText(); 
}

void Parser::ident(Ident &t) {
		Expect(_t_ident);
		t.name = tokenText();
		t.isPrivate = (t.name[0] == _t_priv);
		t.isConstant = (t.isPrivate ? 
		iswupper(t.name[1]) : 
		iswupper(t.name[0]));   
		
}

void Parser::symbol(Symbol& t) {
		t = {}; Ident id; 
		ident(id);
		t.push_back(id); 
		while (la->kind == _t_comp) {
			Get();
			ident(id);
			t.push_back(id); 
		}
}

void Parser::Xalang() {
		Expect(_t_module);
		symbol(root.name);
		block(root.scope);
}

void Parser::block(Scope &s) {
		while (StartOf(1)) {
			while (la->kind == _t_eol) {
				Get();
			}
			list(s);
			Expect(_t_eol);
			while (la->kind == _t_eol) {
				Get();
			}
		}
}

void Parser::list(Scope &s) {
		if (StartOf(2)) {
			item(s);
			while (la->kind == _t_sep) {
				Get();
				while (la->kind == _t_eol) {
					Get();
				}
				item(s);
			}
		} else if (la->kind == _t_par_start) {
			Get();
			block(s);
			Expect(_t_par_end);
		} else SynErr(20);
}

void Parser::item(Scope &s) {
		Expr e; 
		if (StartOf(3)) {
			value(e.val);
			e.type = Expr::Value; 
		} else if (isDecl()) {
			declaration(e.decl);
			e.type = Expr::Declaration; 
		} else if (la->kind == _t_ident) {
			evaluation(e.eval);
			e.type = Expr::Evaluation; 
		} else SynErr(21);
		s.push_back(e); 
}

void Parser::declaration(Declaration& d) {
		symbol(d.sym);
		Expect(_t_decl);
		while (la->kind == _t_eol) {
			Get();
		}
		list(d.layout);
}

void Parser::evaluation(Evaluation& e) {
		symbol(e.sym);
		list(e.layout);
}




// If the user declared a method Init and a mehtod Destroy they should
// be called in the contructur and the destructor respctively.
//
// The following templates are used to recognize if the user declared
// the methods Init and Destroy.

template<typename T>
struct ParserInitExistsRecognizer {
	template<typename U, void (U::*)() = &U::Init>
	struct ExistsIfInitIsDefinedMarker{};

	struct InitIsMissingType {
		char dummy1;
	};
	
	struct InitExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static InitIsMissingType is_here(...);

	// exist only if ExistsIfInitIsDefinedMarker is defined
	template<typename U>
	static InitExistsType is_here(ExistsIfInitIsDefinedMarker<U>*);

	enum { InitExists = (sizeof(is_here<T>(NULL)) == sizeof(InitExistsType)) };
};

template<typename T>
struct ParserDestroyExistsRecognizer {
	template<typename U, void (U::*)() = &U::Destroy>
	struct ExistsIfDestroyIsDefinedMarker{};

	struct DestroyIsMissingType {
		char dummy1;
	};
	
	struct DestroyExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static DestroyIsMissingType is_here(...);

	// exist only if ExistsIfDestroyIsDefinedMarker is defined
	template<typename U>
	static DestroyExistsType is_here(ExistsIfDestroyIsDefinedMarker<U>*);

	enum { DestroyExists = (sizeof(is_here<T>(NULL)) == sizeof(DestroyExistsType)) };
};

// The folloing templates are used to call the Init and Destroy methods if they exist.

// Generic case of the ParserInitCaller, gets used if the Init method is missing
template<typename T, bool = ParserInitExistsRecognizer<T>::InitExists>
struct ParserInitCaller {
	static void CallInit(T *t) {
		// nothing to do
	}
};

// True case of the ParserInitCaller, gets used if the Init method exists
template<typename T>
struct ParserInitCaller<T, true> {
	static void CallInit(T *t) {
		t->Init();
	}
};

// Generic case of the ParserDestroyCaller, gets used if the Destroy method is missing
template<typename T, bool = ParserDestroyExistsRecognizer<T>::DestroyExists>
struct ParserDestroyCaller {
	static void CallDestroy(T *t) {
		// nothing to do
	}
};

// True case of the ParserDestroyCaller, gets used if the Destroy method exists
template<typename T>
struct ParserDestroyCaller<T, true> {
	static void CallDestroy(T *t) {
		t->Destroy();
	}
};

void Parser::Parse() {
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	Get();
	Xalang();
	Expect(0);
}

Parser::Parser(Scanner *scanner) {
	maxT = 18;

	ParserInitCaller<Parser>::CallInit(this);
	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	errors = new Errors();
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[4][20] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x},
		{x,T,T,T, T,T,x,x, x,x,x,x, T,x,x,T, T,T,x,x},
		{x,x,T,T, T,T,x,x, x,x,x,x, x,x,x,T, T,T,x,x},
		{x,x,x,T, T,T,x,x, x,x,x,x, x,x,x,T, T,T,x,x}
	};



	return set[s][la->kind];
}

Parser::~Parser() {
	ParserDestroyCaller<Parser>::CallDestroy(this);
	delete errors;
	delete dummyToken;
}

Errors::Errors() {
	count = 0;
}

void Errors::SynErr(int line, int col, int n) {
	wchar_t* s;
	switch (n) {
			case 0: s = coco_string_create(L"EOF expected"); break;
			case 1: s = coco_string_create(L"t_eol expected"); break;
			case 2: s = coco_string_create(L"t_ident expected"); break;
			case 3: s = coco_string_create(L"t_integer expected"); break;
			case 4: s = coco_string_create(L"t_real expected"); break;
			case 5: s = coco_string_create(L"t_string expected"); break;
			case 6: s = coco_string_create(L"t_priv expected"); break;
			case 7: s = coco_string_create(L"t_comp expected"); break;
			case 8: s = coco_string_create(L"t_sep expected"); break;
			case 9: s = coco_string_create(L"t_decl expected"); break;
			case 10: s = coco_string_create(L"t_params_start expected"); break;
			case 11: s = coco_string_create(L"t_params_end expected"); break;
			case 12: s = coco_string_create(L"t_par_start expected"); break;
			case 13: s = coco_string_create(L"t_par_end expected"); break;
			case 14: s = coco_string_create(L"t_module expected"); break;
			case 15: s = coco_string_create(L"t_true expected"); break;
			case 16: s = coco_string_create(L"t_false expected"); break;
			case 17: s = coco_string_create(L"t_null expected"); break;
			case 18: s = coco_string_create(L"??? expected"); break;
			case 19: s = coco_string_create(L"invalid value"); break;
			case 20: s = coco_string_create(L"invalid list"); break;
			case 21: s = coco_string_create(L"invalid item"); break;

		default:
		{
			wchar_t format[20];
			coco_swprintf(format, 20, L"error %d", n);
			s = coco_string_create(format);
		}
		break;
	}
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	count++;
}

void Errors::Warning(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
}

void Errors::Warning(const wchar_t *s) {
	wprintf(L"%ls\n", s);
}

void Errors::Exception(const wchar_t* s) {
	wprintf(L"%ls", s); 
	exit(1);
}

} // namespace

