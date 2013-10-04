int64_t Bass::evaluate(const string& expression) {
  if(queryPhase() || writePhase()) {
    if(expression == "--") return getVariable({"lastLabel#", lastLabelCounter - 2});
    if(expression == "-" ) return getVariable({"lastLabel#", lastLabelCounter - 1});
    if(expression == "+" ) return getVariable({"nextLabel#", nextLabelCounter + 0});
    if(expression == "++") return getVariable({"nextLabel#", nextLabelCounter + 1});
  }

  Eval::Node* node = nullptr;
  try {
    node = Eval::parse(expression);
  } catch(...) {
    error("malformed expression: ", expression);
  }
  return evaluate(node);
}

int64_t Bass::evaluate(Eval::Node* node) {
  #define p(n) evaluate(node->link[n])

  switch(node->type) {
  case Eval::Node::Type::Member: return evaluateMember(node);
  case Eval::Node::Type::Literal: return evaluateLiteral(node);
  case Eval::Node::Type::Positive: return +p(0);
  case Eval::Node::Type::Negative: return -p(0);
  case Eval::Node::Type::Multiply: return p(0) * p(1);
  case Eval::Node::Type::Divide: return p(0) / p(1);
  case Eval::Node::Type::Modulo: return p(0) % p(1);
  case Eval::Node::Type::Add: return p(0) + p(1);
  case Eval::Node::Type::Subtract: return p(0) - p(1);
  case Eval::Node::Type::ShiftLeft: return p(0) << p(1);
  case Eval::Node::Type::ShiftRight: return p(0) >> p(1);
  case Eval::Node::Type::BitwiseAnd: return p(0) & p(1);
  case Eval::Node::Type::BitwiseOr: return p(0) | p(1);
  case Eval::Node::Type::BitwiseXor: return p(0) ^ p(1);
  case Eval::Node::Type::Equal: return p(0) == p(1);
  case Eval::Node::Type::NotEqual: return p(0) != p(1);
  case Eval::Node::Type::LessThanEqual: return p(0) <= p(1);
  case Eval::Node::Type::GreaterThanEqual: return p(0) >= p(1);
  case Eval::Node::Type::LessThan: return p(0) < p(1);
  case Eval::Node::Type::GreaterThan: return p(0) > p(1);
  case Eval::Node::Type::LogicalAnd: return p(0) ? p(1) : 0;
  case Eval::Node::Type::LogicalOr: return !p(0) ? p(1) : 1;
  case Eval::Node::Type::Condition: return p(0) ? p(1) : p(2);
  }

  #undef p
  error("malformed expression");
}

int64_t Bass::evaluateMember(Eval::Node* node) {
  lstring p;
  p.prepend(node->link[1]->literal);
  node = node->link[0];
  while(node->type == Eval::Node::Type::Member) {
    p.prepend(node->link[1]->literal);
    node = node->link[0];
  }
  p.prepend(node->literal);
  string s = p.merge(".");

  if(queryPhase() || writePhase()) {
    if(auto variable = findVariable(s)) return variable();
    if(queryPhase()) return pc();
  }

  error("unrecognized variable: ", s);
}

int64_t Bass::evaluateLiteral(Eval::Node* node) {
  string& s = node->literal;

  if(s[0] == '0' && s[1] == 'b') return binary(s);
  if(s[0] == '0' && s[1] == 'o') return octal(s);
  if(s[0] == '0' && s[1] == 'x') return hex(s);
  if(s[0] >= '0' && s[0] <= '9') return integer(s);
  if(s[0] == '%') return binary(s);
  if(s[0] == '$') return hex(s);
  if(s.match("'?'")) return s[1];

  if(queryPhase() || writePhase()) {
    if(auto variable = findVariable(s)) return variable();
    if(queryPhase()) return pc();
  }

  error("unrecognized variable: ", s);
}
