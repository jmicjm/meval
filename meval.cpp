#include "meval.h"
#include "meval_math_functions.h"

#include <cstring>
#include <cctype>
#include <cmath>
#include <array>
#include <vector>

#include <iostream>


struct op
{
	const std::string name;
	const unsigned int rank;
	const bool ltor;//associativity
	const int postfix;//1 -> postfix, 0 -> prefix, -1 -> does not apply
	const unsigned int n_ary;
	union
	{
		operand_t (* const fbinary)(operand_t&, operand_t&);
		operand_t (* const funary)(operand_t&);
	};
	op(const std::string& name, unsigned int rank, bool ltor, int postfix, unsigned int n_ary, operand_t (* const fbinary)(operand_t&, operand_t&))
		: name(name), rank(rank), ltor(ltor), postfix(postfix), n_ary(n_ary), fbinary(fbinary) {}
	op(const std::string& name, unsigned int rank, bool ltor, int postfix, unsigned int n_ary, operand_t (* const funary)(operand_t&))
		: name(name), rank(rank), ltor(ltor), postfix(postfix), n_ary(n_ary), funary(funary) {}
};

std::array<op, 23> operators =
{
	{
	{"+",  5, true,  -1, 2, add      },
	{"-",  5, true,  -1, 2, sub      },
	{"*",  6, true,  -1, 2, mul      },
	{"/",  6, true,  -1, 2, div      },
	{"%",  6, true,  -1, 2, mfmod    },
	{"^",  7, false, -1, 2, mpow     },

	{"+",  7, false,  0, 1, uplus    },
	{"-",  7, false,  0, 1, umin     },
	{"!",  9, true,   1, 1, fct      },

	{"<",  4, true,  -1, 2, lt       },
	{">",  4, true,  -1, 2, gt       },
	{"<=", 4, true,  -1, 2, elt      },
	{">=", 4, true,  -1, 2, egt      },
	{"==", 3, true,  -1, 2, eq       },
	{"!=", 3, true,  -1, 2, neq      },

	{"!",  7, false,  0, 1, lnot     },
	{"&&", 2, true,  -1, 2, land     },
	{"||", 1, true,  -1, 2, lor      },

	{"=",  0, false,  -1, 2,  assign     },
	{"+=", 0, false,  -1, 2,  assignadd  },
	{"-=", 0, false,  -1, 2,  assignsub  },
	{"*=", 0, false,  -1, 2,  assignmul  },
	{"/=", 0, false,  -1, 2,  assigndiv  }
	}
};

//checks if there is n_ary, postfix/prefix version of operator with provided id, if there is return its id
int xopid(int id, int n_ary, int postfix)
{
	if (operators[id].n_ary == n_ary && operators[id].postfix == postfix) { return id; }
	const std::string& name = operators[id].name;
	for (int i = 0; i < operators.size(); i++)
	{
		if (   operators[i].name    == name
			&& operators[i].n_ary   == n_ary
			&& operators[i].postfix == postfix)
		{
			return i;
		}
	}
	return -1;
}
//binary operator id
int bopid(int id)     { return xopid(id, 2, -1); }
//unary prefix operator id
int upreopid(int id)  { return xopid(id, 1, 0);  }
//unary postfix operator id
int upostopid(int id) { return xopid(id, 1, 1);  }

//parses operator name starting from b, stores operator id in id and returns pointer to last char of operator name
const char* opid(const char* b, const char* e, int& id);

//returns pointer to end of parenthesis starting at b
const char* paEnd(const char* b, const char* e, char opening_c, char closng_c);
const char* paEnd(const char* b, const char* e) { return paEnd(b, e, '(', ')'); }
const char* bracketEnd(const char* b, const char* e) { return paEnd(b, e, '{', '}'); }

//parses real value starting from b, stores it in val and returns pointer to last char of value
const char* num(const char* b, const char* e, operand_t& val);

//returns pointer to next operator with respect to op_id rank
const char* next(const char* b, const char* e, unsigned int op_id);

struct fidp
{
	std::string fname;
	operand_t (*fptr)(operand_t);
};
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


struct cidv
{
	std::string cname;
	operand_t cval;
};
std::array<cidv,2> c =
{
	{
	{"pi", std::acos(-1.0)},
	{"e", std::exp(1.0)}
	}
};


const char* nextscolon(const char* b, const char* e);


struct kword
{
	std::string name;
};
std::array<kword, 1> keywords
{
	{"var"}
};

struct variable
{
	std::string name;
	operand_t value;
};

template<typename T, typename Y>
const char* xid(const char* b, const char* e, int& id, T& c, std::string Y::* s_ptr)
{
	id = -1;
	size_t l_match = 0;
	for (int i = c.size()-1; i >=0; i--)//reverse order so varid() will return newest variable(from current scope)
	{
		const size_t c_size = (c[i].*s_ptr).size();
		if (e - b < c_size) { continue; }
		if (c_size > l_match && !memcmp((c[i].*s_ptr).data(), b, c_size))
		{
			id = i;
			l_match = c_size;
		}
	}

	if (id >= 0) { b += (c[id].*s_ptr).size() - 1; }
	return b;
}
//parses constant name starting from b, stores id in id and returns pointer to last char of constant name
const char* cid(const char* b, const char* e, int& id) { return xid(b, e, id, c, &cidv::cname); }
//parses function name starting from b, stores id in id and returns pointer to last char of function name
const char* fid(const char* b, const char* e, int& id) { return xid(b, e, id, fn, &fidp::fname); }
//parses keyword name starting from b, stores id in id and returns pointer to last char of keyword name
const char* kid(const char* b, const char* e, int& id) { return xid(b, e, id, keywords, &kword::name); }
//parses variable name starting from b, stores id in id and returns pointer to last char of variable name
const char* varid(const char* b, const char* e, std::vector<variable>& vars, int& id) { return xid(b, e, id, vars, &variable::name); }


struct state
{
	std::vector<variable> variables;
	//functions
};

//evaluates expression between b and e
operand_t eval(const char* b, const char* e, state& st)
{
	operand_t s = 0;

	int op_id;
	const char* op_e = opid(b, e, op_id);
	op_id = upreopid(op_id);
	if (op_id >=0)//unary prefix operator
	{
		b = op_e+1;

		const char* next_op = next(b, e, op_id);
		operand_t ps = eval(b, next_op, st);	
		s = operators[op_id].funary(ps);

		b = next_op - 1;
	}
	else if (isdigit(*b))//plain number
	{
		b = num(b, e, s);
	}
	else if (*b == '(')//sub expression
	{
		s = eval(b+1, paEnd(b, e), st);
		b = paEnd(b, e);
	}
	else if (isalpha(*b))//functions, constants and variables
	{
		int id;
		b = fid(b, e, id);
		if (id >= 0)
		{
			b++;
			if (*b == '(')
			{
				s = fn[id].fptr(eval(b+1, paEnd(b, e), st));
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
				b = varid(b, e, st.variables, id);
				if (id >= 0)
				{
					int assgn_id;
					const char* asgn = opid(b + 1, e, assgn_id);
					if (assgn_id >= 18)
					{
						
						const char* next_op = next(b+1, e, assgn_id);
						operand_t ps = eval(asgn+1, next_op, st);

						s = operators[assgn_id].fbinary(st.variables[id].value, ps);

						b = next_op - 1;
					}
					else
					{
						s = st.variables[id].value;
					}
				}
				else
				{
					return NAN;
				}
			}
		}
	}
	else
	{
		return NAN;
	}
	b++;
	op_e = opid(b, e, op_id);
	while (upostopid(op_id) >= 0)//unary postfix operators
	{
		op_id = upostopid(op_id);
		if (op_id >= 0)
		{
			s = operators[op_id].funary(s);
			b = op_e + 1;
		}
		op_e = opid(b, e, op_id);
	}

	//subsequent binary operators
	while (b < e)
	{
		op_e = opid(b, e, op_id);
		op_id = bopid(op_id);

		if (op_id >= 18) { return NAN; }
		if (op_id >= 0)
		{
			b=op_e+1;
		}
		else//implicit multiplication eg. 2pi
		{
			op_id = 2;
		}
		const char* next_op = next(b, e, op_id);
		if (next_op == b) { return NAN; }

		operand_t ps = 0;
		if (!(op_id == 17 && s || op_id == 16 && !s))//|| && short circuit
		{
			ps = eval(b, next_op, st);
		}
		
		if (std::isnan(s) || std::isnan(ps)) { return NAN; }
		s = operators[op_id].fbinary(s, ps);

		b = next_op;
	}

	return s;
}

int variableExist(std::vector<variable>& vars, const std::string& var_name, size_t c)
{
	for (int i = 0; i < c; i++)
	{
		if (var_name == vars[vars.size()-1-i].name) { return i; }
	}
	return -1;
}

operand_t evalScope(const char* const b, const char* const e, state& st, bool returning)
{
	unsigned int var_c = 0;
	bool invalid = false;

	const char* const f_sc = returning ? nextscolon(b, e) : b;
	const char* sc = returning ? f_sc + 1 : b;

	while (sc != nextscolon(sc, e))
	{
		int keid;
		const char* ke = kid(sc, nextscolon(sc, e), keid);
		sc = ke + 1;

		switch (keid)
		{
		case 0://var...
			{
				variable v;
				while (sc < e && isalpha(*sc))
				{
					v.name += *sc;
					sc++;
				}
				int id = variableExist(st.variables, v.name, var_c);//in current scope
				if (id == -1)
				{
					st.variables.push_back(v);
					var_c++;
				}
				else
				{
					invalid = true;
				}
				eval(ke + 1, nextscolon(sc, e), st);
			}
			break;
		default:
			if (*ke == '{')
			{
				sc = bracketEnd(ke, e);
				evalScope(ke+1, sc, st, false);
			}
			else
			{
				eval(ke, nextscolon(sc, e), st);
			}
		}

		sc = nextscolon(sc, e) + 1;
	}

	operand_t result = returning ? eval(b, f_sc, st) : NAN;

	st.variables.erase(st.variables.end() - var_c, st.variables.end());//remove variables added in current scope

	return invalid ? NAN : result;
}

operand_t eval(const std::string& e)
{
	std::string p;

	for (int i = 0; i < e.size(); i++)
	{
		if (e[i] != ' ') { p.push_back(e[i]); }
	}
	state st;

	return evalScope(p.data(), p.data() + p.size(), st, true);
}

const char* nextscolon(const char* b, const char* e)
{
	while (b < e && *b != ';')
	{
		if (*b == '(')
		{
			b = paEnd(b, e) + 1;
		}
		else if (*b == '{')
		{
			b = bracketEnd(b, e) + 1;
		}
		else
		{
			b++;
		}
	}
	return b;
}

const char* paEnd(const char* b, const char* e, char opening_c, char closing_c)
{
	b++;
	while (b != e && *b != closing_c)
	{
		if (*b != opening_c) { b++; }
		else
		{
			b = paEnd(b, e, opening_c, closing_c) + 1;
		}
	}
	return std::min(b, e);
}

const char* num(const char* b, const char* e, operand_t& val)
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

	operand_t mul = 1;
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

const char* next(const char* b, const char* e, unsigned int op_id)
{
	bool l_op = true;
	while (b < e)
	{
		if (*b == '(')
		{
			b = paEnd(b, e);
		}

		int id;
		b = opid(b, e, id);
		if (id >= 0 && upostopid(id) == -1)
		{
			if (!l_op)//handle expression like x*-y
			{
				if (operators[op_id].ltor)
				{
					if (operators[id].rank <= operators[op_id].rank) 
					{
						b -= operators[id].name.size() - 1;
						break; 
					}
				}
				else
				{
					if (operators[id].rank < operators[op_id].rank) 
					{
						b -= operators[id].name.size() - 1;
						break; 
					}
				}
			}
			l_op = true;
		}
		else
		{
			l_op = false;
		}
		b++;
	}
	return b;
}

struct op_seq_node
{
	int id = -1;
	std::vector<op_seq_node> ids;
};

void build(const char* b, const char* e, op_seq_node& n)
{
	for (int i = 0; i < operators.size(); i++)
	{
		const size_t c_size = operators[i].name.size();
		if (e - b < c_size) { continue; }
		if (!memcmp(operators[i].name.data(), b, c_size))
		{
			op_seq_node tmp;
			tmp.id = i;
			n.ids.push_back(tmp);
		}
	}
	for (int i = 0; i < n.ids.size(); i++)
	{
		size_t len = operators[n.ids[i].id].name.size();
		build(b+len, e, n.ids[i]);
	}
}
//returns length(in characters) of longest node of n, if rem==true removes everything except longest nodes
size_t nlen(op_seq_node& n, bool rem)
{
	size_t len = operators[n.id].name.size();
	std::vector<size_t> lens(n.ids.size());

	size_t max = 0;
	for (int i = 0; i < n.ids.size(); i++)
	{
		lens[i] = nlen(n.ids[i], true);
		if (lens[i] > max)
		{
			max = lens[i];
		}
	}
	if (rem)
	{	
		for (int i = 0; i < n.ids.size();)
		{
			if (lens[i] != max)
			{
				n.ids.erase(n.ids.begin()+i);
			}
			else
			{
				i++;
			}
		}
	}

	return len + max;
}
//returns depth(in nodes) of shallowest node of n, if rem==true removes everything except shallowest nodes
size_t ndepth(op_seq_node& n, bool rem)
{
	if (n.ids.size() > 0)
	{
		std::vector<size_t> depths(n.ids.size());

		size_t min = ndepth(n.ids[0], true);
		depths[0] = min;
		for (int i = 1; i < n.ids.size(); i++)
		{
			depths[i] = ndepth(n.ids[i], true);
			if (depths[i] < min)
			{
				min = depths[i];
			}
		}
		if (rem)
		{
			for (int i = 0; i < n.ids.size();)
			{
				if (depths[i] != min)
				{
					n.ids.erase(n.ids.begin() + i);
				}
				else
				{
					i++;
				}
			}
		}
		return 1 + min;
	}
	return 1;
}
//parses operator name starting from b, stores operator id in id and returns pointer to last char of operator name
const char* opid(const char* b, const char* e, int& id)
{
	id = -1;

	op_seq_node n;
	build(b, e, n);

	
	if (n.ids.size() > 0)
	{
		//todo: remove nodes with multiple binary operators eg. AB(b), C(u), D(u), CD(b) ABCD -> AB C D not AB CD
		nlen(n, true);//remove everything except longest pathes
		ndepth(n, true);//remove everything except shallowest nodes

		id = n.ids[0].id;
	}

	if (id >= 0) { b += operators[id].name.size() - 1; }
	return b;
}