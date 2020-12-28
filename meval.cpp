#include "meval.h"

#include <cstring>
#include <cctype>
#include <cmath>
#include <array>


int opRank(char op);

bool isOp(char c);

//returns pointer to end of parenthesis starting at b
const char* paEnd(const char* b, const char* e);

//parses real value starting from b, stores it in val and returns pointer to last char of value
const char* num(const char* b, const char* e, double& val);

double calc(double l, char op, double r);

//returns pointer to next operator with respect to rank
const char* nextRank(const char* b, const char* e, char op);

struct fidp
{
	std::string fname;
	double (*fptr)(double);
};
double cot(double x) { return cos(x) / sin(x); }
double acot(double x) { return acos(-1.0)/2 - std::atan(x); }
std::array<fidp, 18> fn =
{
	{
	{"sin", std::sin},
	{"cos", std::cos},
	{"tan", std::tan},
	{"cot", cot},

	{"asin", std::asin},
	{"acos", std::acos},
	{"atan", std::atan},
	{"acot", acot},

	{"log", std::log},
	{"ln", std::log},
	{"log10", std::log10},
	{"log2", std::log2},

	{"sqrt", std::sqrt},
	{"cbrt", std::cbrt},

	{"abs", std::fabs},
	{"floor", std::floor},
	{"ceil", std::ceil},
	{"round", std::round}
	}
};
//parses function name starting from b, stores id in id and returns pointer to last char of function name
const char* fid(const char* b, const char* e, int& id);

struct cidv
{
	std::string cname;
	double cval;
};
std::array<cidv,2> c =
{
	{
	{"pi", std::acos(-1.0)},
	{"e", std::exp(1.0)}
	}
};
//parses constant name starting from b, stores id in id and returns pointer to last char of constant name
const char* cid(const char* b, const char* e, int& id);

//evaluates expression between b and e
double eval(const char* b, const char* e)
{
	double s = 0;

	bool sign_op = false;
	if (*b == '-' || *b == '+')//leading sign
	{
		s = (*b == '-') ? -1 : 1;
		sign_op = true;
	}
	else if (isdigit(*b))//plain number
	{
		b = num(b, e, s);
	}
	else if (*b == '(')//sub expression
	{
		s = eval(b+1, paEnd(b, e));
		b = paEnd(b, e);
	}
	else if (isalpha(*b))//functions and constants
	{
		int id;
		b = fid(b, e, id);
		if (id >= 0)
		{
			b++;
			if (*b == '(')
			{
				s = fn[id].fptr(eval(b+1, paEnd(b, e)));
				b = paEnd(b, e);
			}
			else
			{
				return NAN;
			}
		}
		else
		{
			b = cid(b, e, id);
			if (id >= 0)
			{
				s = c[id].cval;
			}
			else
			{
				return NAN;
			}
		}
	}
	else
	{
		return NAN;
	}
	b++;
	if (*b == '!')//factorial
	{
		s = tgamma(s + 1);
		b++;
	}

	while (b < e)
	{
		char op;
		if (!sign_op && isOp(*b))
		{
			op = *b++;
		}
		else//implicit multiplication eg. 2pi, -pi
		{
			op = '*';
			sign_op = false;
		}
		const char* next_op = nextRank(b, e, op);
		double ps = 0;
		ps = eval(b, next_op);
		s = calc(s, op, ps);

		b = next_op;
	}

	return s;
}

double eval(const std::string& e)
{
	std::string p;

	for (int i = 0; i < e.size(); i++)
	{
		if (e[i] != ' ') { p.push_back(e[i]); }
	}

	return eval(p.data(), p.data() + p.size());
}

int opRank(char op)
{
	if (op == '+' || op == '-') { return 1; }
	if (op == '*' || op == '/' || op == '%') { return 2; }
	if (op == '^') { return 3; }
	return 999;
}

bool isOp(char c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^';
}

const char* paEnd(const char* b, const char* e)
{
	b++;
	while (b != e && *b != ')')
	{
		if (*b != '(') { b++; }
		else
		{
			b = paEnd(b, e) + 1;
		}
	}
	return std::min(b, e);
}

const char* num(const char* b, const char* e, double& val)
{
	unsigned int len = 0;
	unsigned int dot_p = e - b;

	unsigned int dot_c = 0;
	for (; b + len < e && ((dot_c == 0 && b[len] == '.') || isdigit(b[len])); len++)
	{
		if (b[len] == '.' && dot_c == 0)
		{
			dot_c++;
			dot_p = len;
		}
	}
	if (dot_c == 0) { dot_p = len; }

	val = 0;

	double mul = 1;
	for (int i = dot_p - 1; i >= 0; i--)
	{
		val += (b[i] - '0') * mul;
		mul *= 10;
	}

	mul = 1.f / 10;
	for (int i = dot_p + 1; b + i < e && i < len; i++)
	{
		val += (b[i] - '0') * mul;
		mul /= 10;
	}

	return b + len - 1;
}

double calc(double l, char op, double r)
{
	switch (op)
	{
	case '+':
		return l + r;
	case '-':
		return l - r;
	case '*':
		return l * r;
	case '/':
		return l / r;
	case '%':
		return fmod(l, r);
	case '^':
		return pow(l, r);
	}
	return NAN;
}

const char* nextRank(const char* b, const char* e, char op)
{
	bool l_op = true;
	while (b != e)
	{
		if (*b == '(')
		{
			b = paEnd(b, e);
		}
		if (!l_op)//handle expression like x*-y
		{
			if (op == '^')
			{
				if (opRank(*b) < opRank(op)) { break; }
			}
			else
			{
				if (opRank(*b) <= opRank(op)) { break; }
			}
		}
		l_op = isOp(*b);
		b++;
	}
	return b;
}

const char* fid(const char* b, const char* e, int& id)
{
	id = -1;
	size_t l_match = 0;
	for (int i = 0; i < fn.size(); i++)
	{
		const size_t c_size = fn[i].fname.size();
		if (e - b < c_size) { continue; }
		if(c_size > l_match && !memcmp(fn[i].fname.data(), b, c_size))
		{
			id = i;
			l_match = c_size;
		}
	}

	if (id >= 0) { b += fn[id].fname.size() - 1; }
	return b;
}

const char* cid(const char* b, const char* e, int& id)
{
	id = -1;
	size_t l_match = 0;
	for (int i = 0; i < c.size(); i++)
	{
		const size_t c_size = c[i].cname.size();
		if (e - b < c_size) { continue; }
		if (c_size > l_match && !memcmp(c[i].cname.data(), b, c_size))
		{
			id = i;
			l_match = c_size;
		}
	}

	if (id >= 0) { b += c[id].cname.size() - 1; }
	return b;
}