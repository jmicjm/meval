#pragma once
#include <string>
#include <cstring>
#include <iostream>
#include <cctype>
#include <cmath>

int opRank(char op)
{
	if (op == '+' || op == '-') { return 1; }
	if (op == '*' || op == '/' || op == '%') { return 2; }
	if (op == '^') { return 3; }
	return 999;
}
bool isOp(char c)
{
	return c =='+' || c == '-' || c == '*' || c == '/' || c == '%' || c  == '^';
}

const char* paEnd(const char* b, const char* e)
{
	b++;
	while (b != e && *b != ')')
	{
		if (*b != '(') { b++; }
		else
		{
			b = paEnd(b, e)+1;
		}
	}
	return std::min(b,e);
}

const char* num(const char* b, const char* e, double& val)
{
	unsigned int len = 0;
	unsigned int dot_p = e - b;

	unsigned int dot_c = 0;
	for (;b + len < e &&((dot_c == 0 && b[len] == '.') || isdigit(b[len])); len++)
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
	for (int i = dot_p-1; i >=0; i--)
	{
		val += (b[i] - '0') * mul;
		mul *= 10;
	}

	mul = 1.f / 10;
	for (int i = dot_p + 1; b + i < e && i <len; i++)
	{
		val += (b[i] - '0') * mul;
		mul /= 10;
	}

	return b + len-1;
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
			return fmod(l,r);
		case '^':
			return pow(l, r);
	}
	return NAN;
}

const char* nextRank(const char* b, const char* e, char op)
{
	unsigned int non_op_c = 0;
	while (b != e)
	{
		if (*b == '(')
		{
			b = paEnd(b, e);
		}
		if (non_op_c > 0)//handle expression like x*-y
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
		if (!isOp(*b)) { non_op_c++; }
		b++;
	}
	return b;
}

struct fidp
{
	const char* fname;
	double (*fptr)(double);
};

fidp fn[] =
{
	{"sin", sin},
	{"cos", cos},
	{"tan", tan},
	{"abs", abs},
	{"log", log},
	{"floor", floor},
	{"ceil", ceil},
	{"round", round}
};

const char* fid(const char* b, const char* e, int& id)
{


	id = -1;
	for (int i = 0; i < sizeof(fn)/sizeof(fidp); i++)
	{
		if (e - b < strlen(fn[i].fname) - 1) { continue; }
		bool r = memcmp(b, fn[i].fname, strlen(fn[i].fname)-1);	
		if (r == 0) { id = i; }
	}
	
	
	if (id >= 0) { b += strlen(fn[id].fname) - 1; }
	return b;
}


inline double eval(const char* b, const char* e)
{
	double s = 0;
	double s_mul = 1;

	if (*b == '-')
	{
		s_mul = -1;
		b++;
	}
	else if (*b == '+')
	{
		b++;
	}

	if (isdigit(*b))
	{
		b = num(b, e, s);
	}
	else if (*b == '(')
	{
		s = eval(b + 1, paEnd(b, e));
		b = paEnd(b, e);
	}
	else if (isalpha(*b))
	{
		int id;
		b = fid(b, e, id);
		if (id >= 0)
		{
			s = fn[id].fptr(eval(b + 1, paEnd(b+1, e))); 
			b = paEnd(b + 1, e);	
		}
	}
	else
	{
		return NAN;
	}

	s *= s_mul;

	b++;
	
	while (b < e)
	{
		char op = *b++;
		const char* next_op = nextRank(b, e, op);
		double ps = 0;
		ps = eval(b, next_op);
		s = calc(s, op, ps);

		b = next_op;
	}

	return s;
}

inline double eval(std::string e)
{
	std::string p;

	for (int i = 0; i < e.size(); i++)
	{
		if (e[i] == ' ') { continue; }

		p.push_back(e[i]);

	}

	return eval(p.data(), p.data() + p.size());
}