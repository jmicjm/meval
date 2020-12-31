#pragma once
#include "meval.h"
#include <cmath>

operand_t add(operand_t& l, operand_t& r) { return l + r; }
operand_t sub(operand_t& l, operand_t& r) { return l - r; }
operand_t mul(operand_t& l, operand_t& r) { return l * r; }
operand_t div(operand_t& l, operand_t& r) { return l / r; }

operand_t uplus(operand_t& x) { return x; }
operand_t umin(operand_t& x)  { return -x; }
operand_t fct(operand_t& x)   { return std::tgamma(x + 1); }

operand_t eq(operand_t& l, operand_t& r)  { return l == r; }
operand_t neq(operand_t& l, operand_t& r) { return l != r; }
operand_t lt(operand_t& l, operand_t& r)  { return l < r;  }
operand_t gt(operand_t& l, operand_t& r)  { return l > r;  }
operand_t elt(operand_t& l, operand_t& r) { return l <= r; }
operand_t egt(operand_t& l, operand_t& r) { return l >= r; }

operand_t lnot(operand_t& x) { return !x; }
operand_t land(operand_t& l, operand_t& r) { return l && r; }
operand_t lor(operand_t& l, operand_t& r)  { return l || r; }

operand_t cot(operand_t x) { return cos(x) / sin(x); }
operand_t acot(operand_t x) { return acos(-1.0) / 2 - std::atan(x); }

operand_t mfmod(operand_t& l, operand_t& r) { return std::fmod(l,r); }
operand_t mpow(operand_t& l, operand_t& r) { return std::pow(l,r); }

operand_t assign(operand_t& l, operand_t& r)    { return l = r; }
operand_t assignadd(operand_t& l, operand_t& r) { return l += r; }
operand_t assignsub(operand_t& l, operand_t& r) { return l -= r; }
operand_t assignmul(operand_t& l, operand_t& r) { return l *= r; }
operand_t assigndiv(operand_t& l, operand_t& r) { return l /= r; }