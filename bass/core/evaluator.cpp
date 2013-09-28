int64_t Bass::evaluate(const string& expression) {
  return evaluate(Eval::parse(expression));
}

int64_t Bass::evaluate(Eval::Node* node) {
  #define p(n) evaluate(node->link[n])

  switch(node->type) {
  case Eval::Node::Type::Literal: return evaluateLiteral(node);
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
  print("warning: malformed expression\n");
}

int64_t Bass::evaluateLiteral(Eval::Node* node) {
  string& s = node->literal;

  if(auto variable = variables.find({s})) return variable().value;

  if(s[0] == '0' && s[1] == 'b') return binary(s);
  if(s[0] == '0' && s[1] == 'o') return octal(s);
  if(s[0] == '0' && s[1] == 'x') return hex(s);
  return integer(s);
}
