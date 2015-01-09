/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NURIA_TEMPLATE_ASTNODES_HPP
#define NURIA_TEMPLATE_ASTNODES_HPP

#include "../nuria/templateerror.hpp"
#include "templateengine_p.hpp"
#include <nuria/callback.hpp>
#include <QRegularExpression>
#include <QSharedData>
#include <memory>

namespace Nuria {

class TemplateProgramPrivate;
enum class EscapeMode;
class Token;

namespace Template {

class Compiler;

/** \brief Abstract class for AST nodes in Twig code. */
class Node {
public:
	
	/** Constructor. */
	Node (Location l) : loc (l) { }
	
	/** Destructor. */
	virtual ~Node () { }
	
	/** Renders the token itself. */
	virtual QString render (TemplateProgramPrivate *dptr) = 0;
	
	/**
	 * Prepares this node for rendering, compiling against \a compiler.
	 * Return non-NULL on success, and \c nullptr on failure setting
	 * \a error to an appropriate error.
	 * 
	 * Each node must invoke compile() on their children if available.
	 * 
	 * If the returned node is not the node itself, then the returned node
	 * shall replace the compiled node and the compiled node should be
	 * destroyed.
	 * 
	 * The default implementation does nothing and returns \c true.
	 */
	virtual Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr);
	
	//
	Location loc;
	
};

class BlockNode;
class SharedNode : public QSharedData {
public:
	
	typedef QMap< QByteArray, Template::BlockNode * > BlockMap;
	
	SharedNode (Template::Node *n = nullptr) : node (n) { }
	~SharedNode () { delete node; }
	
	Template::Node *node;
	BlockMap blocks;
	
};

class MultipleNodes : public Node {
public:
	MultipleNodes (Location l) : Node (l) {}
	MultipleNodes (Location l, const QVector< Node * > n)
	        : Node (l), nodes (n) {}
	
	~MultipleNodes () override
	{ qDeleteAll (nodes); }
	
	/** Invokes render() on all \c nodes and returns the concatenated string. */
	QString render (TemplateProgramPrivate *dptr) override;
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	void trimInner (TemplateProgramPrivate *dptr);
	void trimNodes (TemplateProgramPrivate *dptr);
	void trimNode (Node *left, Node *right, int mode);
	void trimLeft (QString &text);
	void trimRight (QString &text);
	
	Node *mergeTexts (TemplateProgramPrivate *dptr);
	bool checkAndMergeText (int at);
	Node *staticReduction ();
	
	// 
	QVector< Node * > nodes;
		
};

/** Stores plain text data */
class TextNode : public Node {
public:
	TextNode (Location l, QString t) : Node (l), text (t) {}
	
	Node *compile (Compiler *, TemplateProgramPrivate *dptr) override;
	
	QString render (TemplateProgramPrivate *) override
	{ return text; }
	
	void trimSpacesBetweenHtmlTags ();
	
	// 
	QString text;
	
};

/** Represents a evaluate-able value. */
class ValueNode : public Node {
public:
	
	ValueNode (Location l) : Node (l) { }
	
	/** Returns the evaluated variant converted to a QString. */
	QString render (TemplateProgramPrivate *dptr) override;
	
	/** Returns the value as evaluated variant. */
	virtual QVariant evaluate (TemplateProgramPrivate *dptr) = 0;
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	
	/** Returns if the value of the value node is constant. */
	virtual bool isConstant (TemplateProgramPrivate *dptr) const
	{ Q_UNUSED(dptr); return true; }
};

/** Dummy node doing nothing. */
class NoopNode : public ValueNode {
public:
	
	NoopNode (Location l) : ValueNode (l) {}
	
	Node *compile (Compiler *, TemplateProgramPrivate *) override
	{ return this; }
	
	QString render (TemplateProgramPrivate *) override
	{ return QString (); }
	
	QVariant evaluate (TemplateProgramPrivate *) override
	{ return QVariant (); }
	
};

// Helper structure for 'objectElement' grammar rule
struct ObjectElement { ValueNode *key; ValueNode *value; };

/** Key -> node mapping node. */
class ValueMapNode : public ValueNode {
public:
	typedef QMap< QString, ValueNode * > Map;
	
	ValueMapNode (Location l) : ValueNode (l) {}
	~ValueMapNode () override
	{ qDeleteAll (values); }
	
	/** Evaluates all nodes and returns a QVariantMap. */
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	bool isConstant (TemplateProgramPrivate *dptr) const override;
	
	// 
	QMap< ValueNode *, ValueNode * > initValues;
	Map values;
	
};

/** A 'literal' stored in a QVariant. */
class LiteralValueNode : public ValueNode {
public:
	LiteralValueNode (Location l, QVariant v) : ValueNode (l), value (v) {}
	
	QVariant evaluate (TemplateProgramPrivate *) override
	{ return value; }
	
	Node *compile (Compiler *, TemplateProgramPrivate *) override
	{ return this; }
	
	// 
	QVariant value;
	
};

class StringNode : public ValueNode {
public:
	struct Insert {
		int index;
		int length;
		Node *value;
	};
	
	StringNode (Location l, const QString &str)
	        : ValueNode (l), string (str) {}
	~StringNode () override
	{ clear (); }
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	QString render (TemplateProgramPrivate *dptr) override;
	QVariant evaluate (TemplateProgramPrivate *dptr) override
	{ return render (dptr); }
	
	bool isConstant (TemplateProgramPrivate *) const override;
	
	bool populateIndexes (Compiler *compiler, TemplateProgramPrivate *dptr);
	int addInterpolation (Compiler *compiler, TemplateProgramPrivate *dptr, int index);
	Node *inlineCompile (QByteArray code, int offset, Compiler *compiler,
	                     TemplateProgramPrivate *dptr);
	void clear ();
	
	// 
	QString string;
	QVector< Insert > values;
};

/** Operators for ExpressionNode. */
enum class Operator {
	NoOp = 0,
	Not,
	Negate,
	Add,
	Substract,
	Multiply,
	Divide,
	Modulo,
	Power,
	Concatenate,
	Equal,
	NotEqual,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	In,
	NotIn,
	And,
	Or,
	DivisibleBy,
	StartsWith,
	EndsWith,
	IsDefined,
	IsNull,
	IsEmpty,
	IsEven,
	IsOdd,
	IsIterable
};

/** An expression. */
class ExpressionNode : public ValueNode {
public:
	ExpressionNode (Location l, ValueNode *lhs, Operator op, ValueNode *rhs)
	        : ValueNode (l), left (lhs), right (rhs), action (op) {}
	
	~ExpressionNode () override {
		delete left;
		delete right;
	}
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	bool isConstant (TemplateProgramPrivate *dptr) const override;
	
	// 
	ValueNode *left;
	ValueNode *right;
	Operator action;
};

/** 'x' matches 'y' */
class MatchesTestNode : public ValueNode {
public:
	
	MatchesTestNode (Location l, ValueNode *left, ValueNode *right)
	        : ValueNode (l), value (left), test (right)
	{ }
	
	~MatchesTestNode () override {
		delete value;
		delete test;
	}
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr);
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	bool isConstant (TemplateProgramPrivate *) const override
	{ return false; }
	
	QRegularExpression evaluateRegEx (TemplateProgramPrivate *dptr);
	
	// 
	ValueNode *value;
	ValueNode *test;
	QRegularExpression regularExpr;
	
};

/** Multiple values in the form of "(value, value, ...)". */
class MultipleValueNode : public ValueNode {
public:
	MultipleValueNode (Location l, QVector< ValueNode * > v) : ValueNode (l), values (v) {}
	MultipleValueNode (Location l) : ValueNode (l) {}
	~MultipleValueNode () override { qDeleteAll (values); }
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	
	/** Calls evalue() on all values, returning all results. */
	QVariantList evaluateAll (TemplateProgramPrivate *dptr);
	
	/** Calls evalue() on all values, returning a QVariantList. */
	QVariant evaluate (TemplateProgramPrivate *dptr) override
	{ return evaluateAll (dptr); }
	
	bool isConstant (TemplateProgramPrivate *dptr) const override;
	
	// 
	QVector< ValueNode * > values;
	
};

/** The ternary operator. */
class TernaryOperatorNode : public ValueNode {
public:
	
	TernaryOperatorNode (Location l, ValueNode *test, ValueNode *success, ValueNode *failure)
	        : ValueNode (l), expression (test), onSuccess (success), onFailure (failure) { }
	
	~TernaryOperatorNode () override
	{ clear (); }
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	void compileSubNodes (Compiler *compiler, TemplateProgramPrivate *dptr);
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	bool isConstant (TemplateProgramPrivate *dptr) const override;
	void clear ();
	
	// 
	ValueNode *expression;
	ValueNode *onSuccess;
	ValueNode *onFailure;
	
};

/** A variable. */
class VariableNode : public ValueNode {
public:
	
	VariableNode (Location l, const QString &name)
	        : ValueNode (l), variable (name) {}
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	
	/** Reads the value. */
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	
	virtual Callback asFunction (TemplateProgramPrivate *dptr, bool &isConst);
	
	/** Writes the value. */
	void write (TemplateProgramPrivate *dptr, const QVariant &value);
	
	/** Checks if the variable is constant up to this point. */
	bool isConstant (TemplateProgramPrivate *dptr) const override;
	int lastWriteAccessRecord (TemplateProgramPrivate *dptr) const;
	
	// 
	bool isFunction = false;
	QString variable;
	int index = -1;
	bool writeAccess = false;
	bool constantValue = false;
	
};

/** A variable with a 'path' like a.b["c"] */
class ChainedVariableNode : public VariableNode {
public:
	ChainedVariableNode (Location l, const QString &base, MultipleValueNode *next)
	        : VariableNode (l, base), chain (next) {}
	
	~ChainedVariableNode () override
	{ delete chain; }
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	void reduceChain (TemplateProgramPrivate *dptr);
	
	bool isConstant (TemplateProgramPrivate *) const
	{ return false; }
	
	/** Reads the value. */
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	Callback asFunction (TemplateProgramPrivate *dptr, bool &isConst) override;
	QVariant evaluateChain (TemplateProgramPrivate *dptr);
	
	// 
	MultipleValueNode *chain;
	QVariantList chainList;
	
};

/** A stand-alone method. */
class MethodCallValueNode : public ValueNode {
public:
	MethodCallValueNode (Location l, VariableNode *var, MultipleValueNode *args)
	        : ValueNode (l), arguments (args), name (var) {}
	
	~MethodCallValueNode () override {
		delete arguments;
		delete name;
	}
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	Node *configureParentCall (TemplateProgramPrivate *dptr);
	
	/** Invokes the method and returns the result. */
	QVariant evaluate (TemplateProgramPrivate *dptr) override;
	QVariant evaluateUserFunction (const QVariantList &args, TemplateProgramPrivate *dptr);
	
	bool isConstant (TemplateProgramPrivate *dptr) const override;
	
	// 
	MultipleValueNode *arguments;
	VariableNode *name;
	
};

/** {% set X = Y %} */
class SetNode : public Node {
public:
	SetNode (Location l, VariableNode *var, ValueNode *val)
	        : Node (l), variable (var), value (val)
	{}
	
	~SetNode () {
		delete variable;
		delete value;
	}
	
	// Renders to nothing, but sets the value of 'variable'
	QString render (TemplateProgramPrivate *dptr) override {
		variable->write (dptr, value->evaluate (dptr));
		return QString ();
	}
	
	// 
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	
	// 
	VariableNode *variable;
	ValueNode *value;
};

struct ConditionEnd { const Token *U, *V, *X, *Y; Node *elseBranch; };

/** {% if X %} */
class IfClauseNode : public Node {
public:
	IfClauseNode (Location l, ValueNode *expr,
	              Node *then, Node *elseThis = nullptr)
	        : Node (l), expression (expr), onSuccess (then), onFailure (elseThis)
	{}
	
	~IfClauseNode () override {
		delete expression;
		delete onSuccess;
		delete onFailure;
	}
	
	Node *evaluateAndReturnNode (TemplateProgramPrivate *dptr);
	QString render (TemplateProgramPrivate *dptr) override;
	Node *compileInternal (bool constantFolding, Compiler *compiler, TemplateProgramPrivate *dptr);
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	
	// 
	ValueNode *expression;
	Node *onSuccess;
	Node *onFailure;
	
};

struct ForLoopVariables { VariableNode *key; VariableNode *value; };
struct ForLoopIf { ValueNode *condition; const Token *T; };

/** {% for X in Y %} */
class ForLoopNode : public IfClauseNode {
public:
	ForLoopNode (Location l, VariableNode *var, ValueNode *expr,
	             Node *then, Node *elseThis = nullptr, VariableNode *keyVar = nullptr,
	             ValueNode *onlyIf = nullptr)
	        : IfClauseNode (l, expr, then, elseThis),
	          variable (var), key (keyVar), condition (onlyIf)
	{}
	
	~ForLoopNode () override {
		delete condition;
		delete variable;
		delete key;
	}
	
	QString render (TemplateProgramPrivate *dptr) override;
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	int iterateList (TemplateProgramPrivate *dptr, const QVariant &data, QString &target, const QVariant &parent);
	int iterateMap (TemplateProgramPrivate *dptr, const QVariant &data, QString &target, const QVariant &parent);
	bool doRun (TemplateProgramPrivate *dptr, QString &target, const QVariant &current,
	            int index, int length, const QVariant &parent);
	bool doMapRun (TemplateProgramPrivate *dptr, QString &target, const QVariant &key,
	               const QVariant &current, int index, int length, const QVariant &parent);
	void doElse (TemplateProgramPrivate *dptr, QString &target);
	bool checkCurrentForMatch (TemplateProgramPrivate *dptr);
	void updateLoopVariable (TemplateProgramPrivate *dptr, int index, int length, const QVariant &parent);
	void setUpLoopVariable (TemplateProgramPrivate *dptr);
	
	// 
	VariableNode *variable;
	VariableNode *key;
	ValueNode *condition;
	int loopVariable = -1;
	
};

struct BlockEnd { const Token *end; const Token *endName; };

/** {% block X %} */
class BlockNode : public ValueNode {
public:
	BlockNode (Location l, const QByteArray &n, std::shared_ptr< Node > b)
	        : ValueNode (l), name (n), body (b) {}
	~BlockNode () override;
	
	QString render (TemplateProgramPrivate *dptr) override;
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	BlockNode *storeBlockIfFirst (TemplateProgramPrivate *dptr);
	
	// BlockNode is a ValueNode so we can replace a call to parent()
	// which is a MethodCallValueNode with a BlockNode at compile-time
	// without breaking things. 
	QVariant evaluate (TemplateProgramPrivate *dptr) override
	{ return render (dptr); }
	
	bool isConstant (TemplateProgramPrivate *) const override
	{ return false; }
	
	// 
	QByteArray name;
	std::shared_ptr< Node > body;
	SharedNode *d_ptr = nullptr;
	
};

/** {% include X %} and {% extends X %} */
class IncludeNode : public Node {
public:
	IncludeNode (Location l, bool includes, ValueNode *path)
	        : Node (l), include (includes), name (path) {}
	
	~IncludeNode () override {
		delete name;
		delete subNode;
	}
	
	QString render (TemplateProgramPrivate *dptr) override {
		if (subNode) return subNode->render (dptr);
		return QString ();
	}
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	Node *loadAndCompileTemplate (Compiler *compiler, TemplateProgramPrivate *dptr);
	bool templateNames (TemplateProgramPrivate *dptr, QStringList &names);
	bool ifTemplateNotKnownErrorOut (Compiler *compiler, const QStringList &names, QString &name,
	                                 TemplateProgramPrivate *dptr);
	
	// 
	bool include;
	ValueNode *name;
	Node *subNode = nullptr;
	
};

/** {% embed '..' %} .. {% endembed %} */
class EmbedNode : public IncludeNode {
public:
	
	EmbedNode (Location l, ValueNode *path, Node *inner)
	        : IncludeNode (l, true, path)
	{
		this->subNode = inner;
	}
	
	// 
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	
};

/** {% filter .. %} .. {% endfilter %} */
class FilterNode : public Node {
public:
	
	FilterNode (Location l) : Node (l) {}
	~FilterNode () override {
		qDeleteAll (funcs);
		delete outer; // Will delete 'inner'
		delete body;
	}
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	QString render (TemplateProgramPrivate *dptr) override;
	
	MethodCallValueNode *compileFunctions (Compiler *compiler, TemplateProgramPrivate *dptr);
	
	// 
	QVector< MethodCallValueNode * > funcs;
	MethodCallValueNode *outer = nullptr;
	MethodCallValueNode *inner = nullptr;
	Node *body = nullptr;
	
};

/** {% autoescape ['..'] %} .. {% endautoescape %} */
class AutoescapeNode : public Node {
public:
	
	AutoescapeNode (Location l, Node *body, QString mode = QString ())
	        : Node (l), body (body), mode (mode) {}
	
	~AutoescapeNode () override
	{ delete body; }
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	QString render (TemplateProgramPrivate *dptr) override;
	
	// 
	Node *body;
	QString mode;
	EscapeMode escapeMode;
	
};

/** {% spaceless %} .. {% endspaceless %} */
class SpacelessNode : public Node {
public:
	
	SpacelessNode (Location l, Node *body)
	        : Node (l), body (body) { }
	
	~SpacelessNode ()
	{ delete body; }
	
	Node *compile (Compiler *compiler, TemplateProgramPrivate *dptr) override;
	QString render (TemplateProgramPrivate *dptr) override;
	
	// 
	Node *body;
};

}
}

#endif // NURIA_TEMPLATE_ASTNODES_HPP
