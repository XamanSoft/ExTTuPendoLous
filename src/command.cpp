#include <extpl/command.hpp>
#include <extpl/symbol.hpp>
#include <extpl/context.hpp>
#include <cctype>
#include <map>

using ExTPL::Context;
using ExTPL::Command;
using ExTPL::Error;
using ExTPL::IStream;

std::map<int, int> pairTable {
	{ '`', '`' },
	{ '{', '}' },
	{ '(', ')' },
	{ '[', ']' },
	{ '"', '"' },
	{ '\'', '\'' }
};

Command::Command(const Context& cxt): context(cxt), currentSymbol(nullptr) { }
Command::~Command() { delete currentSymbol; }

IStream& Command::parse(IStream &is) {
	delete currentSymbol;
	currentSymbol = nullptr;
	textBlock = "";
	std::string symbolName = name(is);
	
	int c = is.peek();
	if (!startSymbol(c)) {
		is.error("wrong start symbol");
		return is;
	}
	is.ignore();
	
	int es = endSymbol(c);
	
	symbolName += c;
	symbolName += es;
	
	if (!(currentSymbol = context.symbol(symbolName))) {
		is.error("symbol not exist");
		return is;
	}
	
	textBlock = text(is);

	if (is.peek() != es) {
		delete currentSymbol;
		currentSymbol = nullptr;
		is.error("wrong end symbol");
	}
	
	is.ignore();
	return is;
}

Error& Command::exec(std::ostream& out) {
	if (currentSymbol) {
		return currentSymbol->exec(out, textBlock);
	}

	return err;
}

Error& Command::error() {
	if (currentSymbol) {
		return currentSymbol->error();
	}

	return err;
}

std::string Command::name(std::istream &is) {
	std::string resName;
	int c = is.peek();
	
	if (!(c == '_' || std::isalpha(c)))
		return resName;
	
	while (c == '.' || c == '_' || std::isalnum(c)) {
		resName += c;
		is.ignore();
		c = is.peek();
	}

	return resName;
}

bool Command::startSymbol(int c) {
	return pairTable.count(c) > 0;
}

int Command::endSymbol(int st) {
	if (!pairTable.count(st))
		return std::istream::traits_type::eof();
	
	return pairTable[st];
}

std::string Command::text(std::istream &is) {
	std::string retText;
	int c = is.peek();
	
	while (c != EOF && currentSymbol->validText(c)) {
		retText += c;
		is.ignore();
		c = is.peek();
	}
	
	return retText;
}
