#pragma once
#include <wchar.h>
#include <string>
#include <cwchar>
#include <locale>
#include <codecvt>
#include <iostream>
#include <vector>
#include <memory>

namespace xalang {
	using namespace std;
	
	using string_t = wstring;
	using integer_t = int64_t;
	using real_t = double_t;
	using boolean_t = bool;

	// Symbol identifiers
	struct Ident {
		string_t name;
		bool isPrivate;
		bool isConstant;
	};
	using Symbol = vector<Ident>;

	struct Value {
		enum Type {
			Integer,
			Real,
			String,
			Boolean, 
			Null,
			//Token,
		} type;
		string_t text;
	};
	
	using Layout = vector<struct Expr>;

	// declarations associate a symbol with code
	struct Declaration {
		Symbol sym; 
		Layout layout;
	};

	// evaluation invokes symbol to a function with a parameter layout
	struct Evaluation {
		Symbol sym;
		Layout layout;
	};
	
	struct Expr {
		enum Type {
			Value,
			Symbol,
			Declaration,
			Evaluation,
		} type;
		xalang::Value val;
		xalang::Symbol sym;
		xalang::Declaration decl;
		xalang::Evaluation eval;
		
	};
	using Scope = vector<Expr>;
	
	//	Function - data transformation
	//	applies to any data with matching components (in the same module?)
	//*/
	//struct Function {
	//	Ident label;
	//	ValueList params;
	//	Scope scope;
	//};
	//
	//struct Code {
	//	enum Type {
	//		Call,
	//	} type;
	//	Ident label;
	//};


	class Module {
	public:
		Symbol name;
		//vector<Ident> imports;
		Scope scope;

		void run() {

			for (auto e : scope) {
				switch (e.type) {
				case Expr::Declaration:
					// 1. CT define symbol (check not already defined)
					// 2. set memory data
					break;
				case Expr::Evaluation:
					// 1. CT lookup symbol to evaluate
					// 2. 
					break;
				}
			}

		}

	};

}