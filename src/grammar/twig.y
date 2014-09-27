/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.m
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

%name TwigParser

/* Basic configuration */
%token_prefix TOK_
%token_type { const Token * }
%default_type { Node * }

%extra_argument { Template::ParserPrivate *dptr }

/* Types */
%type chainedVariableList { MultipleValueNode * }
%type ternaryExpression { TernaryOperatorNode * }
%type chainedVariable { ChainedVariableNode * }
%type forLoopVariables { ForLoopVariables * }
%type maybeChainedVariable { VariableNode * }
%type chainedVariableElement { ValueNode * }
%type expressionList { MultipleValueNode * }
%type filterFunc { MethodCallValueNode * }
%type filterList { MethodCallValueNode * }
%type objectElementList { ValueMapNode * }
%type funcCall { MethodCallValueNode * }
%type objectElement { ObjectElement * }
%type baseNodeList { MultipleNodes * }
%type blockFilterList { FilterNode * }
%type autoescape { AutoescapeNode * }
%type ifClauseEnd { ConditionEnd * }
%type array { MultipleValueNode * }
%type forLoopEnd { ConditionEnd * }
%type spaceless { SpacelessNode * }
%type variable { VariableNode * }
%type expression { ValueNode * }
%type forLoopIf { ForLoopIf * }
%type object { ValueMapNode * }
%type blockEnd { BlockEnd * }
%type filter { FilterNode * }
%type comment { NoopNode * }
%type string { ValueNode * }
%type operator { Operator }
%type value { ValueNode * }

%type expansionBegin { const Token * }
%type commandBegin { const Token * }
%type expansionEnd { const Token * }
%type commandEnd { const Token * }
%type isNot { const Token * }

/* Destructors */
%destructor objectElement {
	delete $$->key;
	delete $$->value;
	delete $$;
}

%destructor forLoopVariables {
	delete $$->key;
	delete $$->value;
	delete $$;
}

%destructor ifClauseEnd { delete $$->elseBranch; delete $$; }
%destructor forLoopEnd { delete $$->elseBranch; delete $$; }
%destructor forLoopIf { delete $$->condition; delete $$; }
%destructor blockEnd { delete $$; }

%default_destructor { delete $$; }

/* Dummy destructors */
%destructor expansionBegin { }
%destructor commandBegin { }
%destructor expansionEnd { }
%destructor commandEnd { }
%destructor operator { }
%destructor isNot { }

/* Terminals and operator associativity */
%nonassoc WITH OBJECT_BEGIN OBJECT_END ELSE.

%left IS DIVISIBLE BY NULL DEFINED.
%left LESS LESS_EQUAL GREATER GREATER_EQUAL EQUALS NOT_EQUALS.

%left IN STARTS ENDS MATCHES.
%left QUESTION QUESTION_COLON PERIOD_PERIOD.
%left PLUS MINUS CONCAT OR COLON.
%left DIVIDE MULTIPLY POWER MODULO ROUND AND ARRAY_BEGIN ARRAY_END.
%left PIPE PAREN_OPEN PAREN_CLOSE NOT.
%left PERIOD.

/* Syntax error handler */
%syntax_error {
	static const QString parserErrorWhat = QStringLiteral("Unexpected token %1 %2");
	
	QString value = TOKEN->value.toString();
	if (value.isEmpty ()) {
		value = QStringLiteral("<No value>");
	} else {
		value.prepend ("'");
		value.append ("'");
	}
	
	QString what = parserErrorWhat.arg(yyTokenName[TOKEN->tokenId]).arg(value);
	dptr->error = TemplateError (TemplateError::Parser, TemplateError::SyntaxError,
	                             what, toLoc (TOKEN));
}

/*** Grammar ***/
main ::= .
main ::= baseNodeList(B). { dptr->node = B; }

baseNode(A) ::= TEXT(B). { A = new TextNode (toLoc (B), B->value.toString ()); }
baseNode(A) ::= comment(B). { A = B; }
baseNode(A) ::= expansion(B). { A = B; }
baseNode(A) ::= block(B). { A = B; }
baseNode(A) ::= ifClause(B). { A = B; }
baseNode(A) ::= forLoop(B). { A = B; }
baseNode(A) ::= include(B). { A = B; }
baseNode(A) ::= extends(B). { A = B; }
baseNode(A) ::= embed(B). { A = B; }
baseNode(A) ::= filter(B). { A = B; }
baseNode(A) ::= autoescape(B). { A = B; }
baseNode(A) ::= spaceless(B). { A = B; }
baseNode(A) ::= set(B). { A = B; }

baseNodeList(A) ::= baseNodeList(B) baseNode(C). { B->nodes.append (C); A = B; }
baseNodeList(A) ::= baseNode(B). { A = new MultipleNodes (B->loc, { B }); }

/* Twig block markers */
commandBegin(A) ::= COMMAND_BEGIN(B). { A = B; }
commandBegin(A) ::= COMMAND_BEGIN TRIM(B). { A = B; }

commandEnd(A) ::= COMMAND_END(B). { A = B; }
commandEnd(A) ::= TRIM(B) COMMAND_END. { A = B; }

expansionBegin(A) ::= EXPANSION_BEGIN(B). { A = B; }
expansionBegin(A) ::= EXPANSION_BEGIN TRIM(B). { A = B; }

expansionEnd(A) ::= EXPANSION_END(B). { A = B; }
expansionEnd(A) ::= TRIM(B) EXPANSION_END. { A = B; }

/* Comment */
comment(A) ::= COMMENT_L(B). {
	A = new NoopNode (toLoc (B));
	addTrim (dptr, A, true, false);
}

comment(A) ::= COMMENT_R(B). {
	A = new NoopNode (toLoc (B));
	addTrim (dptr, A, false, true);
}

comment(A) ::= COMMENT_LR(B). {
	A = new NoopNode (toLoc (B));
	addTrim (dptr, A, true, true);
}

/* Expansion */
expansion(A) ::= expansionBegin(B) expression(C) expansionEnd(D). {
	A = C;
	addTrim (dptr, A, B, D);
}

/* Blocks */
blockEnd(A) ::= commandEnd(C). { A = new BlockEnd { C, nullptr }; }
blockEnd(A) ::= SYMBOL(B) commandEnd(C). { A = new BlockEnd { C, B }; }

block(A) ::= commandBegin(U) BLOCK_BEGIN SYMBOL(C) commandEnd(V)
                 baseNodeList(D)
             commandBegin(X) BLOCK_END blockEnd(Y). {
	A = new BlockNode (toLoc (U), C->value.toByteArray (), std::shared_ptr< Node > (D));
	addTrim (dptr, A, U, Y->end, V, X);
	addTrim (dptr, D, U, Y->end, V, X);
	
	if (Y->endName && Y->endName->value != C->value) {
		QString got = QString::fromUtf8 (Y->endName->value.toByteArray ());
		QString expected = QString::fromUtf8 (C->value.toByteArray ());
		QString what = QStringLiteral("Named endblock for '%1', expected '%2'").arg(got, expected);
		dptr->error = TemplateError (TemplateError::Parser, TemplateError::BadEndblockName,
	                                     what, toLoc (Y->endName));
	}
	
	delete Y;
}

block(A) ::= commandBegin(X) BLOCK_BEGIN SYMBOL(B) expression(C) commandEnd(Y). {
	A = new BlockNode (toLoc (X), B->value.toByteArray (), std::shared_ptr< Node > (C));
	addTrim (dptr, A, X, Y);
	addTrim (dptr, C, X, Y);
}

/* include and extends */
include(A) ::= commandBegin(X) INCLUDE(B) expression(C) commandEnd(Y). {
	A = new IncludeNode (toLoc (B), true, C);
	addTrim (dptr, A, X, Y);
}

extends(A) ::= commandBegin(X) EXTENDS(B) expression(C) commandEnd(Y). {
	A = new IncludeNode (toLoc (B), false, C);
	addTrim (dptr, A, X, Y);
}

/* filter */
filter(A) ::= commandBegin(U) FILTER_BEGIN(B) blockFilterList(C) commandEnd(V)
                  baseNodeList(D)
              commandBegin(X) FILTER_END commandEnd(Y). {
	A = C;
	A->loc = toLoc (B);
	A->body = D;
	addTrim (dptr, A, U, Y, V, X);
	addTrim (dptr, D, U, Y, V, X);
}
                  
blockFilterList(A) ::= filterFunc(B). {
	A = new FilterNode (B->loc);
	A->funcs.append (B);
}

blockFilterList(A) ::= blockFilterList(B) PIPE filterFunc(C).
{ A = B; A->funcs.append (C); }

/* spaceless */
spaceless(A) ::= commandBegin(U) SPACELESS_BEGIN(B) commandEnd(V)
                     baseNodeList(C)
                 commandBegin(X) SPACELESS_END commandEnd(Y). {
	A = new SpacelessNode (toLoc (B), C);
	addTrim (dptr, A, U, Y, V, X);
	addTrim (dptr, C, U, Y, V, X);
}

/* autoescape */
autoescape(A) ::= commandBegin(U) AUTOESCAPE_BEGIN(B) commandEnd(V)
                      baseNodeList(C)
                  commandBegin(X) AUTOESCAPE_END commandEnd(Y). {
	A = new AutoescapeNode (toLoc (B), C);
	addTrim (dptr, A, U, Y, V, X);
	addTrim (dptr, C, U, Y, V, X);
}

autoescape(A) ::= commandBegin(U) AUTOESCAPE_BEGIN(B) STRING(C) commandEnd(V)
                      baseNodeList(D)
                  commandBegin(X) AUTOESCAPE_END commandEnd(Y). {
	A = new AutoescapeNode (toLoc (B), D, C->value.toString ().mid (1));
	addTrim (dptr, A, U, Y, V, X);
	addTrim (dptr, D, U, Y, V, X);
}

/* embed */
embed(A) ::= commandBegin(U) EMBED_BEGIN(B) expression(C) commandEnd(V)
                 baseNodeList(D)
             commandBegin(X) EMBED_END commandEnd(Y). {
	A = new EmbedNode (toLoc (B), C, D);
	addTrim (dptr, A, U, Y, V, X);
	addTrim (dptr, D, U, Y, V, X);
}

/* if helpers */
ifClauseEnd(A) ::= commandBegin(X) IF_END commandEnd(Y).
{ A = new ConditionEnd { X, Y, X, Y, nullptr }; }

ifClauseEnd(A) ::= commandBegin(U) ELSE commandEnd(V)
                    baseNodeList(B)
                commandBegin(X) IF_END commandEnd(Y).
{ A = new ConditionEnd { U, V, X, Y, B }; }

/* if */
ifClause(A) ::= commandBegin(S) IF_BEGIN(B) expression(C) commandEnd(T)
                    baseNodeList(D)
                ifClauseEnd(E). {
	A = new IfClauseNode (toLoc (B), C, D, E->elseBranch);
	addTrim (dptr, A, S, E->Y);
	addTrim (dptr, D, S, E->Y, T, E->U);
	addTrim (dptr, E->elseBranch, S, E->Y, E->V, E->X);
	delete E;
}

/* For loop helpers */
forLoopVariables(A) ::= variable(B). { A = new ForLoopVariables { nullptr, B }; }
forLoopVariables(A) ::= variable(B) COMMA variable(C).
{ A = new ForLoopVariables { B, C }; }

/* [else ...] endfor */
forLoopEnd(A) ::= commandBegin(X) FOR_END commandEnd(Y).
{ A = new ConditionEnd { Y, Y, X, Y, nullptr }; }

forLoopEnd(A) ::= commandBegin(U) ELSE commandEnd(V)
                   baseNodeList(B)
               commandBegin(X) FOR_END commandEnd(Y).
{ A = new ConditionEnd { U, V, X, Y, B }; }

/* for inner if */
forLoopIf(A) ::= IF_BEGIN expression(B) commandEnd(T). { A = new ForLoopIf { B, T }; }
forLoopIf(A) ::= commandEnd(T). { A = new ForLoopIf { nullptr, T }; }

/* for w[,x] in y [if z] */
forLoop(A) ::= commandBegin(S) FOR_BEGIN(B) forLoopVariables(C) IN expression(D) forLoopIf(E)
                   baseNodeList(F)
               forLoopEnd(G). {
	A = new ForLoopNode (toLoc (B), C->value, D, F, G->elseBranch, C->key, E->condition);
	addTrim (dptr, A, S, G->Y);
	addTrim (dptr, F, S, G->Y, E->T, G->U);
	addTrim (dptr, G->elseBranch, S, G->Y, G->V, G->X);
	delete E;
	delete C;
	delete G;
}

/* "set x = y" */
set(A) ::= commandBegin(X) SET(B) variable(C) ASSIGN expression(D) commandEnd(Y). {
	A = new SetNode (toLoc (B), C, D);
	addTrim (dptr, A, X, Y);
}

/* Variable */
variable(A) ::= SYMBOL(B). { A = new VariableNode (toLoc (B), B->value.toString ()); }
maybeChainedVariable(A) ::= variable(B). { A = B; }
maybeChainedVariable(A) ::= chainedVariable(B). { A = B; }

chainedVariable(A) ::= SYMBOL(B) chainedVariableList(C). {
	A = new ChainedVariableNode (toLoc (B), B->value.toString (), C);
}
chainedVariableList(A) ::= chainedVariableElement(B). {
	A = new MultipleValueNode (B->loc);
	A->values.append (B);
}

chainedVariableList(A) ::= chainedVariableList(B) chainedVariableElement(C). {
	B->values.append (C);
	A = B;
}

chainedVariableElement(A) ::= ARRAY_BEGIN expression(B) ARRAY_END. { A = B; }
chainedVariableElement(A) ::= PERIOD SYMBOL(B). {
	A = new LiteralValueNode (toLoc (B), B->value.toString ());
}

/* Value */
value(A) ::= INTEGER(B). { A = new LiteralValueNode (toLoc (B), B->value); }
value(A) ::= NUMBER(B). { A = new LiteralValueNode (toLoc (B), B->value); }
value(A) ::= variable(B). { A = B; }
value(A) ::= chainedVariable(B). { A = B; }
value(A) ::= string(B). { A = B; }
value(A) ::= TRUE(B). { A = new LiteralValueNode (toLoc (B), true); }
value(A) ::= FALSE(B). { A = new LiteralValueNode (toLoc (B), false); }
value(A) ::= array(B). { A = B; }
value(A) ::= object(B). { A = B; }

string(A) ::= STRING(B). {
	QString raw = B->value.toString ();
	QString value = raw.mid (1);
	
	if (raw.at (0) == QLatin1Char ('"')) {
		A = new StringNode (toLoc (B), value);
	} else {
		A = new LiteralValueNode (toLoc (B), value);
	}
	
}

expressionList(A) ::= . { A = new MultipleValueNode (Location ()); }
expressionList(A) ::= expression(B). { A = new MultipleValueNode (B->loc, { B }); }
expressionList(A) ::= expressionList(B) COMMA expression(C). { B->values.append (C); A = B; }

/* Array */
array(A) ::= ARRAY_BEGIN(B) expressionList(C) ARRAY_END. {
	if (!C) {
		A = new MultipleValueNode (toLoc (B));
	} else {
		C->loc = toLoc (B);
		A = C;
	}
}

/* Object */
object(A) ::= OBJECT_BEGIN(B) objectElementList(C) OBJECT_END.
{ C->loc = toLoc (B); A = C; }

object(A) ::= OBJECT_BEGIN(B) OBJECT_END. { A = new ValueMapNode (toLoc (B)); }
objectElement(A) ::= string(B) COLON value(C). {
	A = new ObjectElement { B, C };
}

objectElementList(A) ::= objectElement(B). {
	A = new ValueMapNode (Location ());
	A->initValues.insert (B->key, B->value);
	delete B;
}

objectElementList(A) ::= objectElementList(B) COMMA objectElement(C). {
	B->initValues.insert (C->key, C->value);
	delete C;
	A = B;
}

/* Filters */
expression(A) ::= filterList(B). { A = B; }
filterList(A) ::= expression(B) PIPE filterFunc(C). {
	C->arguments->values.prepend (B);
	A = C;
}

filterFunc(A) ::= funcCall(B). { A = B; }
filterFunc(A) ::= maybeChainedVariable(B). {
	B->isFunction = true;
	A = new MethodCallValueNode (B->loc, B, new MultipleValueNode (B->loc));
}

/* Function call */
value(A) ::= funcCall(B). { A = B; }
funcCall(A) ::= maybeChainedVariable(B) PAREN_OPEN expressionList(C) PAREN_CLOSE. {
	B->isFunction = true;
	A = new MethodCallValueNode (B->loc, B, C);
}

/* Ternary operator */
expression(A) ::= ternaryExpression(B). { A = B; }

ternaryExpression(A) ::= expression(B) QUESTION expression(C).
{ A = new TernaryOperatorNode (B->loc, B, C, nullptr); }

ternaryExpression(A) ::= expression(B) QUESTION_COLON expression(C).
{ A = new TernaryOperatorNode (B->loc, B, nullptr, C); }

ternaryExpression(A) ::= expression(B) QUESTION expression(C) COLON expression(D).
{ A = new TernaryOperatorNode (B->loc, B, C, D); }

/* Test operators */
isNot(A) ::= IS(B). { A = B; }
isNot(A) ::= IS NOT(B). { A = B; }

expression(A) ::= expression(B) STARTS WITH expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::StartsWith, D); }

expression(A) ::= expression(B) ENDS WITH expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::EndsWith, D); }

expression(A) ::= expression(B) MATCHES expression(D).
{ A = new MatchesTestNode (B->loc, B, D); }

expression(A) ::= expression(B) isNot(C) DIVISIBLE BY expression(D).
{ A = maybeNegate (C, new ExpressionNode (B->loc, B, Operator::DivisibleBy, D)); }

expression(A) ::= expression(B) isNot(C) DEFINED.
{ A = maybeNegate (C, new ExpressionNode (B->loc, B, Operator::IsDefined, nullptr)); }

expression(A) ::= expression(B) isNot(C) NULL.
{ A = maybeNegate (C, new ExpressionNode (B->loc, B, Operator::IsNull, nullptr)); }

expression(A) ::= expression(B) isNot(C) EVEN. {
	Operator op = (C->tokenId == TOK_NOT) ? Operator::IsOdd : Operator::IsEven;
	A = new ExpressionNode (B->loc, B, op, nullptr);
}

expression(A) ::= expression(B) isNot(C) ODD. {
	Operator op = (C->tokenId == TOK_NOT) ? Operator::IsEven : Operator::IsOdd;
	A = new ExpressionNode (B->loc, B, op, nullptr);
}

expression(A) ::= expression(B) isNot(C) EMPTY.
{ A = maybeNegate (C, new ExpressionNode (B->loc, B, Operator::IsEmpty, nullptr)); }

expression(A) ::= expression(B) isNot(C) ITERABLE.
{ A = maybeNegate (C, new ExpressionNode (B->loc, B, Operator::IsIterable, nullptr)); }

/* Expressions */
expression(A) ::= value(B). { A = B; }

expression(A) ::= expression(B) IN expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::In, D); }

expression(A) ::= expression(B) NOT IN expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::NotIn, D); }

expression(A) ::= expression(B) EQUALS expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Equal, D); }

expression(A) ::= expression(B) NOT_EQUALS expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::NotEqual, D); }

expression(A) ::= expression(B) GREATER expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Greater, D); }

expression(A) ::= expression(B) GREATER_EQUAL expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::GreaterEqual, D); }

expression(A) ::= expression(B) LESS expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Less, D); }

expression(A) ::= expression(B) LESS_EQUAL expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::LessEqual, D); }

expression(A) ::= expression(B) CONCAT expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Concatenate, D); }

expression(A) ::= expression(B) PLUS expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Add, D); }

expression(A) ::= expression(B) MINUS expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Substract, D); }

expression(A) ::= expression(B) MULTIPLY expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Multiply, D); }

expression(A) ::= expression(B) DIVIDE expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Divide, D); }

/* "B // D" is equivalent to "round(B / D)" */
expression(A) ::= expression(B) ROUND expression(D). {
	VariableNode *var = new VariableNode (B->loc, QStringLiteral("round"));
	ExpressionNode *expr = new ExpressionNode (B->loc, B, Operator::Divide, D);
	MultipleValueNode *args = new MultipleValueNode (B->loc, { expr });
	var->isFunction = true;
	A = new MethodCallValueNode (B->loc, var, args);
}

expression(A) ::= expression(B) MODULO expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Modulo, D); }

expression(A) ::= expression(B) POWER expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Power, D); }

expression(A) ::= expression(B) AND expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::And, D); }

expression(A) ::= expression(B) OR expression(D).
{ A = new ExpressionNode (B->loc, B, Operator::Or, D); }

expression(A) ::= NOT(B) expression(C).
{ A = new ExpressionNode (toLoc (B), C, Operator::Not, nullptr); }

expression(A) ::= PAREN_OPEN expression(B) PAREN_CLOSE. { A = B; }

expression(A) ::= MINUS(B) expression(C). [NOT]
{ A = new ExpressionNode (toLoc (B), C, Operator::Negate, nullptr); }

/* x..y */
expression(A) ::= expression(B) PERIOD_PERIOD expression(D). {
	VariableNode *var = new VariableNode (B->loc, QStringLiteral("range"));
	MultipleValueNode *args = new MultipleValueNode (B->loc, { B, D });
	var->isFunction = true;
	A = new MethodCallValueNode (B->loc, var, args);
}

/* x[y:z] or x[:z] or x[y:] */
/*
expression(A) ::= expression(B) ARRAY_BEGIN(C) expression(D) COLON expression(E) ARRAY_END.
{ A = sliceShorthandHelper (toLoc (C), B, D, E); }
expression(A) ::= expression(B) ARRAY_BEGIN(C) COLON expression(E) ARRAY_END.
{ A = sliceShorthandHelper (toLoc (C), B, nullptr, E); }

expression(A) ::= expression(B) ARRAY_BEGIN(C) expression(D) COLON ARRAY_END.
{ A = sliceShorthandHelper (toLoc (C), B, D, nullptr); }
*/