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
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#include "astnodes.hpp"

#include <nuria/callback.hpp>
#include <nuria/variant.hpp>
#include <nuria/debug.hpp>

#include "../templateloader.hpp"
#include "variableaccessor.hpp"
#include "builtins.hpp"
#include "compiler.hpp"

#define ENABLE_TRACING

#ifdef ENABLE_TRACING
#define TRACE(...) __VA_ARGS__
#else
#define TRACE(...)
#endif

template< typename Left, typename Right >
static inline Nuria::Template::Node *swapAndDestroy (Left *&target, Right *newNode) {
	if (target != newNode) {
		delete target;
		target = newNode;
	}
	
	return target;
}

Nuria::Template::Node *Nuria::Template::Node::compile (Nuria::Template::Compiler *compiler,
                                                       TemplateProgramPrivate *dptr) {
	Q_UNUSED(compiler);
	Q_UNUSED(dptr);
	return this;
}

QString Nuria::Template::ValueNode::render (TemplateProgramPrivate *dptr) {
	QVariant v = evaluate (dptr);
	
	if (v.userType () != QMetaType::QString) {
		v.convert (QMetaType::QString);
	}
	
	return v.toString ();
}

Nuria::Template::Node *Nuria::Template::ValueNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	Q_UNUSED(compiler)
	
	if (isConstant (dptr)) {
		QVariant value = evaluate (dptr);
		TRACE(nDebug() << "ValueNode, constant-folding" << this << "to" << value);
		return dptr->transferTrim (this, new LiteralValueNode (this->loc, value));
	}
	
	return this;
}

Nuria::Template::Node *Nuria::Template::MultipleValueNode::compile (Compiler *compiler,
                                                                    TemplateProgramPrivate *dptr) {
	for (int i = 0; i < this->values.length (); i++) {
		swapAndDestroy (this->values[i], (ValueNode *)this->values[i]->compile (compiler, dptr));
		
		if (!this->values[i]) {
			TRACE(nError() << "MultipleValueNode" << this << " failed to compile value at index" << i);
			return nullptr;
		}
		
	}
	
	return this;
}

QVariantList Nuria::Template::MultipleValueNode::evaluateAll (TemplateProgramPrivate *dptr) {
	QVariantList list;
	
	for (int i = 0; i < this->values.length (); i++) {
		list.append (this->values.at (i)->evaluate (dptr));
	}
	
	return list;
}

bool Nuria::Template::MultipleValueNode::isConstant (TemplateProgramPrivate *dptr) const {
	bool constant = true;
	for (int i = 0; i < values.length () && (constant = values.at (i)->isConstant (dptr)); i++);
	return constant;
}

Nuria::Template::Node *Nuria::Template::MethodCallValueNode::compile (Compiler *compiler,
                                                                      TemplateProgramPrivate *dptr) {
	swapAndDestroy (name, (VariableNode *)name->compile (compiler, dptr));
	swapAndDestroy (arguments, (MultipleValueNode *)arguments->compile (compiler, dptr));
	
	if (!this->name || !this->arguments) {
		return nullptr;
	}
	
	// parent() is somewhat magic...
	Builtins::Function builtin = Builtins::nameLookup (this->name->variable);
	if (builtin == Builtins::Parent) {
		return configureParentCall (dptr);
	}
	
	// Done.
	return this;
}

Nuria::Template::Node *Nuria::Template::MethodCallValueNode::configureParentCall (TemplateProgramPrivate *dptr) {
	if (!dptr->info->currentParentBlock) {
		TRACE(nDebug() << "MethodCallValueNode" << this << "is calling parent() illegaly");
		dptr->error = TemplateError (TemplateError::Compiler, TemplateError::NoParentBlock,
		                             QStringLiteral("parent() called outside of a block or inside "
		                                            "the initial block declaration"), this->loc);
		return nullptr;
	}
	
	// We have a parent. Replace this MethodCallValueNode with the BlockNode.
	// The body is reference counted for this reason to keep track of how many
	// need the body.
	TRACE(nDebug() << "MethodCallValueNode" << this << "is calling parent() - Replacing with BlockNode pointing to"
              << dptr->info->currentParentBlock);
	TRACE(nDebug() << "  Body" << dptr->info->currentParentBlock->body.get () << "now has" <<
	      (dptr->info->currentParentBlock->body.use_count () + 1) << "references");
	
	BlockNode *block = new BlockNode (this->loc, dptr->info->currentParentBlock->name,
	                                  dptr->info->currentParentBlock->body);
	return dptr->transferTrim (this, block);
}

QVariant Nuria::Template::MethodCallValueNode::evaluate (TemplateProgramPrivate *dptr) {
	Builtins::Function builtin = Builtins::nameLookup (this->name->variable);
	QVariantList args;
	
	if (arguments) {
		args = arguments->evaluateAll (dptr);
	}
	
	if (builtin != Builtins::Unknown) {
		return Builtins::invokeBuiltin (builtin, args, dptr);
	}
	
	return evaluateUserFunction (args, dptr);
}

QVariant Nuria::Template::MethodCallValueNode::evaluateUserFunction (const QVariantList &args,
                                                                     TemplateProgramPrivate *dptr) {
	bool isConst = false;
	Callback cb = name->asFunction (dptr, isConst);
	
	if (!cb.isValid ()) {
		// TODO: Issue error
		return QVariant ();
	}
	
	return cb.invoke (args);
}

bool Nuria::Template::MethodCallValueNode::isConstant (TemplateProgramPrivate *dptr) const {
	if (this->arguments && !this->arguments->isConstant (dptr)) {
		return false;
	}
	
	// Check for a built-in function
	Builtins::Function builtin = Builtins::nameLookup (this->name->variable);
	if (builtin != Builtins::Unknown) {
		return Builtins::isBuiltinConstant (builtin);
	}
	
	// Custom functions may be constant
	auto it = dptr->functions.constFind (this->name->variable);
	if (it == dptr->functions.constEnd ()) {
		return false;
	}
	
	// See if this is one.
	return it->isConstant;
}

static bool compareVariants (const QVariant &left, const QVariant &right, Nuria::Template::Operator op) {
	using namespace Nuria::Template;
	if (op == Operator::Equal) {
		return (left == right);
	} else if (op == Operator::NotEqual) {
		return (left != right);
	}
	
	// 
	if (!left.canConvert< double > () || !right.canConvert< double > ()) {
		return false;
	}
	
	// 
	double l = left.toDouble ();
	double r = right.toDouble ();
	
	switch (op) {
	default: return false;
	case Operator::Less: return (l < r);
	case Operator::LessEqual: return (l <= r);
	case Operator::Greater: return (l > r);
	case Operator::GreaterEqual: return ( l >= r);
	}
	
	return false;
}


static QVariant variantArithmetic (const QVariant &left, const QVariant &right, Nuria::Template::Operator op) {
	using namespace Nuria::Template;
	if (!left.canConvert< double > () || !right.canConvert< double > ()) {
		return 0;
	}
	
	// 
	double l = left.toDouble ();
	double r = right.toDouble ();
	
	switch (op) {
	default: return double (0);
	case Operator::Add: return (l + r);
	case Operator::Substract: return (l - r);
	case Operator::Multiply: return (l * r);
	case Operator::Divide: return (l / r);
	case Operator::Modulo: return fmod (l, r);
	case Operator::Power: return pow (l, r);
	}
	
}

static bool isLeftInRight (const QVariant &left, const QVariant &right) {
	
	if (right.userType () == QMetaType::QVariantList) {
		return right.toList ().contains (left);
	}
	
	// 
	QString key = left.toString ();
	if (key.isEmpty ()) {
		return false;
	}
	
	if (right.userType () == QMetaType::QVariantMap) {
		return right.toMap ().contains (key);
	}
	
	if (right.canConvert< QString > ()) {
		return right.toString ().contains (key);
	}
	
	// 
	return false;
	
}

static inline bool isValueTrue (const QVariant &value) {
	if (value.isNull ()) {
		return false;
	}
	
	// 
	if (value.userType () == QMetaType::Bool) {
		return value.toBool ();
	}
	
	return true;
}

static QVariant negateValue (const QVariant &value) {
	if (value.userType () == QMetaType::Double || value.userType () == QMetaType::Int) {
		return -value.toDouble ();
	}
	
	return !isValueTrue (value);
}

static bool evaluateStringTest (const QString &left, const QString &right, Nuria::Template::Operator op) {
	using namespace Nuria::Template;
	
	switch (op) {
	default: return false;
	case Operator::StartsWith:
		return left.startsWith (right);
	case Operator::EndsWith:
		return left.endsWith (right);
	}
	
}

static bool isLeftDivisibleByRight (double left, double right) {
	return (left != 0 && right != 0) && (fmod (left, right) == 0);
}

static bool isValueEmpty (const QVariant &value) {
	if (value.isNull ()) {
		return true;
	}
	
	// 
	if (value.userType () == QMetaType::QString) {
		return value.toString ().isEmpty ();
	}
	
	if (value.canConvert< QVariantList > ()) {
		QSequentialIterable it = value.value< QSequentialIterable > ();
		return (it.size () == 0);
	}
	
	if (value.canConvert< QVariantMap > ()) {
		QAssociativeIterable it = value.value< QAssociativeIterable > ();
		return (it.size () == 0);
	}
	
	// Something is in it, so it's probably not empty.
	return false;
}

static bool evaluateSingleArgumentTest (const QVariant &variant, Nuria::Template::Operator op) {
	using namespace Nuria::Template;
	
	switch (op) {
	default: return false;
	case Operator::IsDefined:
		// With the current implementation this is always true.
		// TODO: Fix this for ChainedVariableNodes.
		return true;
	case Operator::IsNull:
		return variant.isNull ();
	case Operator::IsIterable:
		return (variant.canConvert< QVariantList > () || variant.canConvert< QVariantMap > ());
	case Operator::IsEven:
	case Operator::IsOdd: {
		double num = variant.toDouble ();
		bool isEven = (remainder (num, 1) == 0 && (int (num) & 1) == 0);
		return (op == Operator::IsEven) ? isEven : !isEven;
	}
	}
	
}

Nuria::Template::Node *Nuria::Template::ExpressionNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	
	swapAndDestroy (this->left, (ValueNode *)this->left->compile (compiler, dptr));
	if (this->right) {
		swapAndDestroy (this->right, (ValueNode *)this->right->compile (compiler, dptr));
	}
	
	// For constant folding. Does transferTrim().
	return ValueNode::compile (compiler, dptr);
}

QVariant Nuria::Template::ExpressionNode::evaluate (TemplateProgramPrivate *dptr) {
	QVariant l = left->evaluate (dptr);
	QVariant r = (!right) ? QVariant () : right->evaluate (dptr);
	
	switch (action) {
	case Operator::NoOp: return l;
	case Operator::Not: return !isValueTrue (l);
	case Operator::Negate: return negateValue (l);
	case Operator::Add:
	case Operator::Substract:
	case Operator::Multiply:
	case Operator::Divide:
	case Operator::Modulo:
	case Operator::Power:
		return variantArithmetic (l, r, action);
	case Operator::Concatenate:
		return (l.toString () + r.toString ());
	case Operator::Equal:
	case Operator::NotEqual:
	case Operator::Less:
	case Operator::LessEqual:
	case Operator::Greater:
	case Operator::GreaterEqual:
		return compareVariants (l, r, action);
	case Operator::In:
		return isLeftInRight (l, r);
	case Operator::NotIn:
		return !isLeftInRight (l, r);
	case Operator::And:
		return (isValueTrue (l) && isValueTrue (r));
	case Operator::Or:
		return (isValueTrue (l) || isValueTrue (r));
	case Operator::DivisibleBy:
		return isLeftDivisibleByRight (l.toDouble (), r.toDouble ());
	case Operator::StartsWith:
	case Operator::EndsWith:
		return evaluateStringTest (l.toString (), r.toString (), action);
	case Operator::IsEmpty:
		return isValueEmpty (l);
	case Operator::IsDefined:
	case Operator::IsNull:
	case Operator::IsEven:
	case Operator::IsOdd:
	case Operator::IsIterable:
		return evaluateSingleArgumentTest (l, action);
	}
	
	return QVariant ();
}

bool Nuria::Template::ExpressionNode::isConstant (TemplateProgramPrivate *dptr) const {
	if (this->left && !this->left->isConstant (dptr)) return false;
	if (this->right && !this->right->isConstant (dptr)) return false;
	return true;
}

Nuria::Template::Node *Nuria::Template::VariableNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	Q_UNUSED(compiler)
	
	if (!this->isFunction) {
		this->index = dptr->addOrGetVariablePosition (this->variable);
		TRACE(nDebug() << "VariableNode" << this << "accesses variable"
		      << variable << "in slot" << index << "(writing:" << this->writeAccess
		      << "constant:" << this->constantValue << ")");
		
		dptr->addUsageRecord (this->index, this->loc, this->writeAccess,
		                      this->constantValue);
	}
	
	return this;
}

QVariant Nuria::Template::VariableNode::evaluate (TemplateProgramPrivate *dptr) {
        if (this->index < 0) {
	        return QVariant ();
        }
        
        // 
	return dptr->values[this->index];
}

Nuria::Callback Nuria::Template::VariableNode::asFunction (TemplateProgramPrivate *dptr, bool &isConst) {
	auto it = dptr->functions.constFind (this->variable);
	if (it == dptr->functions.constEnd ()) {
		return Callback ();
	}
	
	// 
	isConst = it->isConstant;
	return it->callback;
}

void Nuria::Template::VariableNode::write (TemplateProgramPrivate *dptr, const QVariant &value) {
	dptr->values[this->index] = value;
}

bool Nuria::Template::VariableNode::isConstant (TemplateProgramPrivate *dptr) const {
	if (this->index < 0 || dptr->usages.at (this->index).isEmpty ()) {
		return false;
	}
	
	// Return 'true' if the last write access yields a constant value.
	// In this case VariableNode::compile() will have set its current value already
	int lastWrite = lastWriteAccessRecord (dptr);
	return (lastWrite >= 0 && dptr->usages.at (this->index).at (lastWrite).isConstant);
}

int Nuria::Template::VariableNode::lastWriteAccessRecord (TemplateProgramPrivate *dptr) const {
	const VariableUsageList &list = dptr->usages.at (this->index);
	for (int i = list.length () - 1; i >= 0; i--) {
		if (list.at (i).isWriting) {
			return i;
		}
		
	}
	
	// No write accesses.
	return -1;
}

Nuria::Template::Node *Nuria::Template::ChainedVariableNode::compile (Compiler *compiler,
                                                                      TemplateProgramPrivate *dptr) {
	
	TRACE(nDebug() << "Compiling chain" << this->chain << "of ChainedValueNode" << this);
	swapAndDestroy (this->chain, (MultipleValueNode *)this->chain->compile (compiler, dptr));
	reduceChain (dptr);
	
	// 
	return VariableNode::compile (compiler, dptr);
}

void Nuria::Template::ChainedVariableNode::reduceChain (TemplateProgramPrivate *dptr) {
	for (int i = 0; i < this->chain->values.length (); i++) {
		if (!this->chain->values.at (i)->isConstant (dptr)) {
			return;
		}
		
	}
	
	// All elements in the chain are constant, we can reduce it now.
	this->chainList = this->chain->evaluateAll (dptr);
	delete this->chain;
	this->chain = nullptr;
	
	TRACE(nDebug() << "Reduced chain list of ChainedValueNode" << this << "to" << this->chainList);
}

QVariant Nuria::Template::ChainedVariableNode::evaluate (Nuria::TemplateProgramPrivate *dptr) {
	if (this->index < 0) {
		return QVariant ();
	}
	
	return evaluateChain (dptr);
}

Nuria::Callback Nuria::Template::ChainedVariableNode::asFunction (TemplateProgramPrivate *dptr, bool &isConst) {
	isConst = false;
	return evaluate (dptr).value< Callback > ();
}

QVariant Nuria::Template::ChainedVariableNode::evaluateChain (TemplateProgramPrivate *dptr) {
	QVariantList list = this->chainList;
	if (list.isEmpty () && this->chain) {
		list = this->chain->evaluateAll (dptr);
	}
	
	QVariant cur = dptr->values[this->index];
	if (!VariableAcessor::walkChain (cur, list, 0)) {
		// TODO: Output error
		return QVariant ();
	}
	
	return cur;
}

QString Nuria::Template::MultipleNodes::render (TemplateProgramPrivate *dptr) {
	QString data;
	
	auto it = this->nodes.begin ();
	auto end = this->nodes.end ();
	for (; it != end; ++it) {
		data.append ((*it)->render (dptr));
	}
	
	return data;
}

Nuria::Template::Node *Nuria::Template::MultipleNodes::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	
	TRACE(nDebug() << this << "MultipleNodes contains" << nodes.length () << "elements");
	for (int i = 0; i < nodes.length (); i++) {
		Node *n = nodes[i]->compile (compiler, dptr);
		TRACE(nDebug() << this << "MultipleNodes, reducing element Nr." << i << nodes[i] << "->" << n);
		
		if (!n) {
			return nullptr;
		}
		
		// 
		swapAndDestroy (nodes[i], n);
		
        }
	
	// Trim and merge
	trimInner (dptr);
	trimNodes (dptr);
	
	return dptr->transferTrim (this, mergeTexts (dptr));
}

void Nuria::Template::MultipleNodes::trimInner (TemplateProgramPrivate *dptr) {
	if (nodes.isEmpty ()) {
		return;
	}
	
	// Translate the InnerX modes to X
	int selfMode = dptr->info->trim.value (this, 0);
	int mode = 0;
	
	if (selfMode & Trim::InnerLeft) mode |= Trim::Right;
	if (selfMode & Trim::InnerRight) mode |= Trim::Left;
	
	// Trim
	trimNode (nodes.last (), nodes.first (), mode);
	
}

void Nuria::Template::MultipleNodes::trimNodes (TemplateProgramPrivate *dptr) {
	int len = nodes.length ();
	for (int i = 0; i < len; i++) {
		Node *node = nodes.at (i);
		int mode = dptr->info->trim.value (node, 0);
		
		if (mode) {
			Node *left = (i > 0) ? nodes.at (i - 1) : nullptr;
			Node *right = (i + 1 < len) ? nodes.at (i + 1) : nullptr;
			trimNode (left, right, mode);
		}
		
	}
	
}

void Nuria::Template::MultipleNodes::trimNode (Node *left, Node *right, int mode) {
	TextNode *leftText = dynamic_cast< TextNode * > (left);
	TextNode *rightText = dynamic_cast< TextNode * > (right);
	
	if (leftText && mode & Trim::Left) trimRight (leftText->text);
	if (rightText && mode & Trim::Right) trimLeft (rightText->text);
}

void Nuria::Template::MultipleNodes::trimLeft (QString &text) {
	int i = 0;
	for (; i < text.length () && text.at (i).isSpace (); i++);
	text = text.mid (i);
}

void Nuria::Template::MultipleNodes::trimRight (QString &text) {
	int i = text.length () - 1;
	for (; i >= 0 && text.at (i).isSpace (); i--);
	text.chop (text.length () - i - 1);
}

Nuria::Template::Node *Nuria::Template::MultipleNodes::mergeTexts (TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << this << "Merging text nodes in MultipleNodes" << this);
	
	int i = 0;
	while (i < nodes.length ()) {
		if (dynamic_cast< NoopNode * > (nodes[i])) {
			TRACE(nDebug() << "  Removing no-op at" << i);
			delete nodes.takeAt (i);
		} else if (dynamic_cast< LiteralValueNode * > (nodes[i])) {
			TRACE(nDebug() << "  Converting literal value at" << i << "to a text node");
			swapAndDestroy (nodes[i], new TextNode (nodes[i]->loc, nodes[i]->render (dptr)));
		} else if (!checkAndMergeText (i)) {
			i++;
		}
		
	}
	
	// 
	return staticReduction ();
}

#ifdef ENABLE_TRACING
static inline QString shorten (const QString &str) {
	if (str.length () < 8) {
		return str;
	}
	
	return str.left (3) + QLatin1String("..") + str.right (3);
}
#endif

bool Nuria::Template::MultipleNodes::checkAndMergeText (int at) {
	if (at < 1) return false;
	
	TextNode *self = dynamic_cast< TextNode * > (nodes[at]);
	TextNode *prev = dynamic_cast< TextNode * > (nodes[at - 1]);
	
	// Check
	if (!self || !prev) {
		return false;
	}
	
	// Merge
	TRACE(nDebug() << this << "  Merging text nodes" << (at - 1) << shorten(prev->text)
	               << "and" << at << shorten(self->text));
	prev->text.append (self->text);
	
	// Delete redundant node
	delete nodes.takeAt (at);
	return true;
}

Nuria::Template::Node *Nuria::Template::MultipleNodes::staticReduction () {
	
	// No elements = Nothing to do.
	if (nodes.isEmpty ()) {
		TRACE(nDebug() << this << "MultipleNodes is empty, reducing to NoopNode.");
		return new NoopNode (this->loc);
	}
	
	// One element = only do that.
	if (nodes.length () == 1) {
		TRACE(nDebug() << this << "MultipleNodes contains one element, reducing to" << nodes[0]);
		return nodes.takeFirst ();
	}
	
	// More than one sub-node
	return this;
}

Nuria::Template::Node *Nuria::Template::IfClauseNode::evaluateAndReturnNode (TemplateProgramPrivate *dptr) {
	bool success = isValueTrue (expression->evaluate (dptr));
	
	if (success) {
		return onSuccess;
	}
	
	return onFailure;
}

QString Nuria::Template::IfClauseNode::render (TemplateProgramPrivate *dptr) {
	Node *node = evaluateAndReturnNode (dptr);
	
	if (node) {
		return node->render (dptr);
	}
	
	return QString ();
}

Nuria::Template::Node *Nuria::Template::IfClauseNode::compileInternal (bool constantFolding, Compiler *compiler,
                                                                       TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "  compiling if/for, full folding active:" << constantFolding);
	swapAndDestroy (expression, (ValueNode *)expression->compile (compiler, dptr));
	
	if (!constantFolding || !expression->isConstant (dptr)) {
		dptr->info->conditionBranchDepth++;
		
		TRACE(nDebug() << "  Failed to constant-fold" << this << "=> Compiling child nodes");
	        swapAndDestroy (onSuccess, onSuccess->compile (compiler, dptr));
	        
	        if (onFailure) {
		        swapAndDestroy (onFailure, onFailure->compile (compiler, dptr));
	        }
	        
		dptr->info->conditionBranchDepth--;
	        return this;
	}
	
	// Return the to-be-evaluate node, or a noop-node if nothing is to be done.
	TRACE(nDebug() << "  Constant folding, expression evaluates to:"
	               << isValueTrue (expression->evaluate (dptr)));
	
	Node *branch = evaluateAndReturnNode (dptr);
	Node *node = branch ? branch->compile (compiler, dptr) : nullptr;
	
	TRACE(nDebug() << "  => Folding to branch" << branch << "which compiled to" << node);
	
	if (!node) {
		TRACE(nDebug() << "  ==> Folding to Noop");
	        node = new NoopNode (this->loc);
	}
	
	// Make sure that the dtor of IfClauseNode won't destroy the node
	// that is returned.
	if (node == onSuccess) onSuccess = nullptr;
	if (node == onFailure) onFailure = nullptr;
	
	// Return the evaluated node
	return dptr->transferTrim (this, node);
	
}

Nuria::Template::Node *Nuria::Template::IfClauseNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "Compiling IfClause" << this);
	return compileInternal (true, compiler, dptr);
}

QString Nuria::Template::ForLoopNode::render (TemplateProgramPrivate *dptr) {
	QVariant result = expression->evaluate (dptr);
	QVariant parent;
	QString target;
	
	// Else ?
	if (!isValueTrue (result)) {
		doElse (dptr, target);
		return target;
	}
	
	// Save parent context
	if (this->loopVariable >= 0) {
		QVariant parentLoop = dptr->values.at (this->loopVariable);
		QVariantMap parentMap { { QStringLiteral("loop"), parentLoop } };
		parent = parentMap;
	}
	
	// Then ..
	int itemCount = 0;
	if (result.canConvert< QVariantList > ()) {
		itemCount = iterateList (dptr, result, target, parent);
	} else if (result.canConvert< QVariantMap > ()) {
		itemCount = iterateMap (dptr, result, target, parent);
	} else {
		doRun (dptr, target, result, 0, 1, parent);
		itemCount = 1;
	}
	
	// No hits?
	if (itemCount < 1) {
		doElse (dptr, target);
	}
	
	// Restore parent context
	if (this->loopVariable >= 0) {
		dptr->values[this->loopVariable] = parent;
	}
	
	return target;
}

Nuria::Template::Node *Nuria::Template::ForLoopNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "Compiling ForLoop" << this);
	
	this->variable->writeAccess = true;
	swapAndDestroy (this->variable, (VariableNode *)this->variable->compile (compiler, dptr));
	
	if (this->condition) {
		swapAndDestroy (this->condition, (ValueNode *)this->condition->compile (compiler, dptr));
	}
	
	if (this->key) {
		this->key->writeAccess = true;
		swapAndDestroy (this->key, (VariableNode *)this->key->compile (compiler, dptr));
	}
	
	// Compile the body
	Node *result = compileInternal (false, compiler, dptr);
	
	setUpLoopVariable (dptr);
	return result;
}

int Nuria::Template::ForLoopNode::iterateList (TemplateProgramPrivate *dptr, const QVariant &data,
                                               QString &target, const QVariant &parent) {
	QSequentialIterable iter = data.value< QSequentialIterable > ();
	auto it = iter.begin ();
	auto end = iter.end ();
	int hits = 0;
	
	int length = iter.size ();
	for (; it != end; ++it) {
		hits += doRun (dptr, target, *it, hits, length, parent);
	}
	
	return hits;
}

int Nuria::Template::ForLoopNode::iterateMap (TemplateProgramPrivate *dptr, const QVariant &data,
                                              QString &target, const QVariant &parent) {
	QAssociativeIterable iter = data.value< QAssociativeIterable > ();
	auto it = iter.begin ();
	auto end = iter.end ();
	int hits = 0;
	
	int length = iter.size ();
	for (; it != end; ++it) {
	        hits += doMapRun (dptr, target, it.key (), it.value (), hits, length, parent);
	}
	
	return hits;
}

bool Nuria::Template::ForLoopNode::doRun (TemplateProgramPrivate *dptr, QString &target, const QVariant &current,
                                          int index, int length, const QVariant &parent) {
	variable->write (dptr, current);
	
	if (!checkCurrentForMatch (dptr)) {
	        return false;
	}
	
	updateLoopVariable (dptr, index, length, parent);
	target.append (onSuccess->render (dptr));
	return true;
}

bool Nuria::Template::ForLoopNode::doMapRun (TemplateProgramPrivate *dptr, QString &target,
                                             const QVariant &key, const QVariant &current,
                                             int index, int length, const QVariant &parent) {
	variable->write (dptr, current);
	
	if (this->key) {
		this->key->write (dptr, key);
	}
	
	if (!checkCurrentForMatch (dptr)) {
		return false;
	}
	
	updateLoopVariable (dptr, index, length, parent);
	target.append (onSuccess->render (dptr));
	return true;
}

bool Nuria::Template::ForLoopNode::checkCurrentForMatch (TemplateProgramPrivate *dptr) {
	if (!this->condition) {
		return true;
	}
	
	// Evaluate condition
	return isValueTrue (this->condition->evaluate (dptr));
}

void Nuria::Template::ForLoopNode::updateLoopVariable (TemplateProgramPrivate *dptr, int index,
                                                       int length, const QVariant &parent) {
	if (this->loopVariable < 0) {
		return;
	}
	
	// Populate map
	QVariant &loopVar = dptr->values[this->loopVariable];
	QVariantMap map {
		{ QStringLiteral("index"), (index + 1) },
		{ QStringLiteral("index0"), index },
		{ QStringLiteral("first"), (index == 0) },
		{ QStringLiteral("parent"), parent }
	};
	
	// Some values are only available if we know the total length
	if (!this->condition) {
		map.insert (QStringLiteral("revindex"), (length - index));
		map.insert (QStringLiteral("revindex0"), (length - index) - 1);
		map.insert (QStringLiteral("length"), length);
		map.insert (QStringLiteral("last"), (index == length - 1));
	}
	
	// 
	loopVar.setValue (map);
	
}

void Nuria::Template::ForLoopNode::setUpLoopVariable (TemplateProgramPrivate *dptr) {
	
	// Find index of the 'loop' variable if used anywhere
	this->loopVariable = dptr->variables.indexOf (QStringLiteral("loop"));
	
	if (this->loopVariable < 0) {
		return;
	}
	
	// Inject write access to tell the program that the variable will be
	// set upon reading from it.
	dptr->prependWriteUsageRecord (this->loopVariable, this->loc);
	
}

void Nuria::Template::ForLoopNode::doElse (TemplateProgramPrivate *dptr, QString &target) {
	if (onFailure) {
		target.append (onFailure->render (dptr));
	}
	
}

QVariant Nuria::Template::ValueMapNode::evaluate (TemplateProgramPrivate *dptr) {
	QVariantMap map;
	
	auto it = values.constBegin ();
	auto end = values.constEnd ();
	for (; it != end; ++it) {
		map.insert (it.key (), it.value ()->evaluate (dptr));
	}
	
	return map;
}

Nuria::Template::Node *Nuria::Template::ValueMapNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	
	TRACE(nDebug() << "Compiling ValueMap" << this);
	
	auto it = this->initValues.constBegin ();
	auto end = this->initValues.constEnd ();
	for (; it != end; ++it) {
		ValueNode *key = it.key ();
		ValueNode *value = it.value ();
		
		swapAndDestroy (key, (ValueNode *)key->compile (compiler, dptr));
		swapAndDestroy (value, (ValueNode *)value->compile (compiler, dptr));
		
		if (!key || !value) {
		        return nullptr;
	        }
		
		// Get key
		QString keyString = key->render (dptr);
		delete key;
		
		// 
		TRACE(nDebug() << "  Adding element with key" << keyString
		      << " from node" << key << "with value =" << value);
		
		// Store.
		this->values.insert (keyString, value);
	}
	
	// 
	this->initValues.clear ();
	return ValueNode::compile (compiler, dptr);
}

bool Nuria::Template::ValueMapNode::isConstant (TemplateProgramPrivate *dptr) const {
	auto it = this->values.begin ();
	auto end = this->values.end ();
	
	bool result = true;
	for (; it != end && (result = (*it)->isConstant (dptr)); ++it);
	return result;
}

Nuria::Template::BlockNode::~BlockNode () {
	
	// Remove ourself from the block map.
	if (d_ptr) {
		d_ptr->blocks.remove (name);
	}
	
}

QString Nuria::Template::BlockNode::render (TemplateProgramPrivate *dptr) {
	if (body) {
		return body->render (dptr);
	}
	
	return QString ();
}

Nuria::Template::Node *Nuria::Template::BlockNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	if (!body) {
		return this;
	}
	
	// For parent() calls
	BlockNode *prevParent = dptr->info->currentParentBlock;
	BlockNode *master = storeBlockIfFirst (dptr);
	dptr->info->currentParentBlock = master;
	
	// Compile
	TRACE(nDebug() << "Compiling Block" << this << name << "body" << body.get ());
	Node *newBody = this->body->compile (compiler, dptr);
	
	if (newBody != this->body.get ()) {
		std::shared_ptr< Node > newOne (newBody);
		this->body.swap (newOne);
	}
	
	// Sanity check
	if (!this->body) {
		return nullptr;
	}
	
	// 
	dptr->info->currentParentBlock = prevParent;
	
	// Replace into master block
	if (master) {
		// The old body of master will be thrown away when this instance
		// gets destroyed by the caller of compile().
		TRACE(nDebug() << "Block" << this << "replaces body" << master->body.get () << "(refs:"
		      << master->body.use_count () << ") of" << master << "with its own" << this->body.get () << "(refs:"
		      << this->body.use_count () << ") - Reducing to no-op node");
		
		master->body.swap (this->body);
		return new NoopNode (this->loc);
	}
	
	// 
	return this;
	
}

Nuria::Template::BlockNode *Nuria::Template::BlockNode::storeBlockIfFirst (TemplateProgramPrivate *dptr) {
	BlockNode *node = dptr->root->blocks.value (name);
	if (!node) {
		TRACE(nDebug() << "Block" << name << "is stored in" << body.get ());
		dptr->root->blocks.insert (name, this);
		this->d_ptr = dptr->root.data ();
	}
	
	// 
	TRACE(nDebug() << "Block" << name << "will be overwritten by contents of" << this->body.get ());
	return node;
}

Nuria::Template::Node *Nuria::Template::SetNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "Compiling SetNode" << this << "value" << value);
	
	// Compile value
	swapAndDestroy (value, (ValueNode *)value->compile (compiler, dptr));
	
	// If we're in a if/for branch, don't mark this variable as constant.
	bool isConst = false;
	if (dptr->info->conditionBranchDepth == 0) {
		isConst = value->isConstant (dptr);
	}
	
	// Update variable and compile
	variable->writeAccess = true;
	variable->constantValue = isConst;
	swapAndDestroy (variable, (VariableNode *)variable->compile (compiler, dptr));
	
	// If the value is constant, then apply the value now, so that we can
	// use the value later in the compile state.
	if (isConst) {
		variable->write (dptr, value->evaluate (dptr));
	}
	
	// Done
	return this;
}

Nuria::Template::Node *Nuria::Template::IncludeNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "IncludeNode" << this << "- Compiling sub-node" << subNode);
	swapAndDestroy(this->subNode, loadAndCompileTemplate (compiler, dptr));
	if (!subNode) {
		TRACE(nError() << "IncludeNode" << this << "- Failed to load template");
		return nullptr;
	}
	
	// Reduce
	TRACE(nDebug() << "IncludeNode" << this << "- Recuding to sub-node" << subNode);
	Node *node = subNode;
	subNode = nullptr;
	return dptr->transferTrim (this, node);
}

Nuria::Template::Node *Nuria::Template::IncludeNode::loadAndCompileTemplate (Compiler *compiler,
                                                                             TemplateProgramPrivate *dptr) {
	
	// Get raw template name
	QStringList names;
	QString name;
	
	if (!templateNames (dptr, names) ||
	    !ifTemplateNotKnownErrorOut (compiler, names, name, dptr)) {
		TRACE(nError() << "IncludeNode" << this << "- Template name is not constant.");
		return nullptr;
	}
	
	// TODO: Check that we're not including a template recursively
	if (!dptr->dependencies.contains (name)) {
		dptr->dependencies.append (name);
	}
	
	// Load template
	TRACE(nDebug() << "IncludeNode" << this <<  "- Loading template" << name << "out of" << names);
	Node *node = compiler->loadAndParse (name, dptr);
	
	// Sanity check
	if (!node) {
		TRACE(nError() << "IncludeNode" << this << "failed to load template" << name);
		return nullptr;
	}
	
	// Compile template
	TRACE(nDebug() << "IncludeNode" << this <<  "- Compiling sub-node" << node);
	swapAndDestroy (node, node->compile (compiler, dptr));
	if (!node) {
		TRACE(nError() << "IncludeNode" << this << "failed to compile template" << name);
		return nullptr;
	}
	
	// Done.
	return node;
}

bool Nuria::Template::IncludeNode::templateNames (TemplateProgramPrivate *dptr, QStringList &names) {
	
	// Make sure the value is constant
	if (!this->name->isConstant (dptr)) {
		TRACE(nError() << "IncludeNode" << this <<  "- non-constant template expression");
		dptr->error = TemplateError (TemplateError::Compiler, TemplateError::NonConstantExpression,
		                             QStringLiteral("Template name expressions must be constant"),
		                             this->loc);
		return false;
	}
	
	// Evalute expression
	names = this->name->evaluate (dptr).toStringList ();
	if (names.isEmpty () || names.contains (QString ())) {
		TRACE(nError() << "IncludeNode" << this <<  "- template expression evaluates to \"\"");
		dptr->error = TemplateError (TemplateError::Compiler, TemplateError::EmptyTemplateName,
		                             QStringLiteral("Given template name evaluates to \"\""),
		                             this->loc);
		return false;
	}
	
	return true;
}

bool Nuria::Template::IncludeNode::ifTemplateNotKnownErrorOut (Compiler *compiler, const QStringList &names,
                                                               QString &name, TemplateProgramPrivate *dptr) {
	
	for (int i = 0; i < names.length (); i++) {
		if (compiler->loader ()->hasTemplate (names.at (i))) {
			name = names.at (i);
			return true;
		}
		
	}
	
	// 
	TRACE(nError() << "IncludeNode" << this <<  "- Unknown template(s)" << names);
	dptr->error = TemplateError (TemplateError::Loader, TemplateError::TemplateNotFound,
	                             QStringLiteral("Unknown template(s) \"%1\"").arg (names.join (',')),
	                             this->loc);
	return false;
}

Nuria::Template::Node *Nuria::Template::StringNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "StringNode" << this << "parsing and compiling injected code");
	
	if (!populateIndexes (compiler, dptr)) {
		clear ();
		return nullptr;
	}
	
	// If this string does no interpolation, replace it with a TextNode.
	if (this->values.isEmpty ()) {
		TRACE(nDebug() << "StringNode" << this << "doesn't do interpolation and is reduced to a TextNode");
		return dptr->transferTrim (this, new TextNode (this->loc, this->string));
	}
	
	// String interpolation to be done in render()
	return this;
}

QString Nuria::Template::StringNode::render (TemplateProgramPrivate *dptr) {
	if (this->values.isEmpty ()) {
		return this->string;
	}
	
	// String interpolation
	QString str = this->string;
	
	int offset = 0;
	for (int i = 0; i < this->values.length (); i++) {
		Insert &cur = this->values[i];
		QString replacement = cur.value->render (dptr);
		str.replace (offset + cur.index, cur.length, replacement);
		offset += replacement.length () - cur.length;
	}
	
	// Done.
	return str;
	
}

bool Nuria::Template::StringNode::isConstant (TemplateProgramPrivate *) const {
	return this->values.isEmpty ();
}

bool Nuria::Template::StringNode::populateIndexes (Compiler *compiler, TemplateProgramPrivate *dptr) {
	int idx = 0;
	while ((idx = this->string.indexOf (QLatin1String ("#{"), idx)) != -1) {
		idx = addInterpolation (compiler, dptr, idx);
		if (idx < 0) {
			return false;
		}
		
	}
	
	return true;
}

int Nuria::Template::StringNode::addInterpolation (Compiler *compiler, TemplateProgramPrivate *dptr, int index) {
	int end = this->string.indexOf ("}", index);
	if (end < 0) {
		return -1;
	}
	
	// 
	int begin = index;
	index += 2; // Skip #{
	int length = end - index;
	QByteArray snippet = this->string.mid (index, length).toUtf8 ();
	
	// 
	Node *node = inlineCompile (snippet, begin, compiler, dptr);
	if (!node) {
		return -1;
	}
	
	// Store
	this->values.append ({ begin, end - begin + 1, node });
	
	// Done.
	return end + 1;
}

Nuria::Template::Node *Nuria::Template::StringNode::inlineCompile (QByteArray code, int offset,
                                                                   Compiler *compiler, 
                                                                   TemplateProgramPrivate *dptr) {	
	// TODO: Don't break if we allow the user to change the {{ .. }} tokens
	code.prepend ("{{ ");
	code.append (" }}");
	
	// Parse!
	Node *node = compiler->parseCode (code, dptr);
	if (!node) {
		return nullptr;
	}
	
	// Compile
	swapAndDestroy (node, node->compile (compiler, dptr));
	
	// Create value node
	Location location = this->loc;
	location.column += offset;
	node->loc = location;
	
	// 
	return node;
}

void Nuria::Template::StringNode::clear () {
	auto it = this->values.begin ();
	auto end = this->values.end ();
	for (; it != end; ++it) {
		delete it->value;
	}
	
	this->values.clear ();
}

Nuria::Template::Node *Nuria::Template::TernaryOperatorNode::compile (Compiler *compiler,
                                                                      TemplateProgramPrivate *dptr) {
	
	// Compile all sub nodes
	compileSubNodes (compiler, dptr);
	
	// 
	if (!this->expression->isConstant (dptr)) {
		TRACE(nDebug() << "TernaryOperatorNode" << this << "expression is not constant");
		return this;
	}
	
	// Evaluate expression
	QVariant value = this->expression->evaluate (dptr);
	ValueNode *chosenNode = nullptr;
	
	if (isValueTrue (value)) {
		chosenNode = this->onSuccess ? this->onSuccess : this->expression;
	} else {
		chosenNode = this->onFailure;
	}
	
	// Check if the value is not constant
	if (!chosenNode || !chosenNode->isConstant (dptr)) {
		if (!chosenNode) {
			TRACE(nDebug() << "TernaryOperatorNode" << this << "expression is constant, but value is not. "
			      "Reducing to noop-node");
			return new NoopNode (this->loc);
		}
		
		TRACE(nDebug() << "TernaryOperatorNode" << this << "expression is constant, but value is not. "
		      "Reducing to value" << chosenNode);
		
		// Save and return the chosen node.
		if (chosenNode == this->onSuccess) this->onSuccess = nullptr;
		else if (chosenNode == this->onFailure) this->onFailure = nullptr;
		else if (chosenNode == this->expression) this->expression = nullptr;
		return dptr->transferTrim (this, chosenNode);
	}
	
	// Expression *and* the resulting value is constant
	TRACE(nDebug() << "TernaryOperatorNode" << this << "expression and value are constant, reducing to value");
	return dptr->transferTrim (this, new LiteralValueNode (this->loc, this->evaluate (dptr)));
}

QVariant Nuria::Template::TernaryOperatorNode::evaluate (TemplateProgramPrivate *dptr) {
	QVariant value = this->expression->evaluate (dptr);
	
	// value is 'true'
	if (isValueTrue (value)) {
		if (onSuccess) {
			return onSuccess->evaluate (dptr);
		}
		
		return value;
	}
	
	// Else
	if (onFailure) {
		return onFailure->evaluate (dptr);
	}
	
	// No failure node, return empty string.
	return QString ();
}

bool Nuria::Template::TernaryOperatorNode::isConstant (TemplateProgramPrivate *dptr) const {
	return (this->expression->isConstant (dptr) &&
	        (!this->onSuccess || this->onSuccess->isConstant (dptr)) &&
	        (!this->onFailure || this->onFailure->isConstant (dptr)));
}

void Nuria::Template::TernaryOperatorNode::clear () {
	delete this->expression;
	delete this->onSuccess;
	delete this->onFailure;
}

void Nuria::Template::TernaryOperatorNode::compileSubNodes (Compiler *compiler, TemplateProgramPrivate *dptr) {
	swapAndDestroy (this->expression, (ValueNode *)this->expression->compile (compiler, dptr));
	
	if (this->onSuccess) {
		swapAndDestroy (this->onSuccess, (ValueNode *)this->onSuccess->compile (compiler, dptr));
	}
	
	if (this->onFailure) {
		swapAndDestroy (this->onFailure, (ValueNode *)this->onFailure->compile (compiler, dptr));
	}
	
}

Nuria::Template::Node *Nuria::Template::MatchesTestNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	
	swapAndDestroy (this->value, (ValueNode *)this->value->compile (compiler, dptr));
	swapAndDestroy (this->test, (ValueNode *)this->test->compile (compiler, dptr));
	
	// Precompile the regular expression if possible
	if (this->test->isConstant (dptr)) {
		this->regularExpr = evaluateRegEx (dptr);
		delete this->test;
		this->test = nullptr;
	}
	
	// Constant-fold
	if (!this->test && this->value->isConstant (dptr)) {
		return dptr->transferTrim (this, new LiteralValueNode (this->loc, evaluate (dptr)));
	}
	
	return this;
}

QVariant Nuria::Template::MatchesTestNode::evaluate (TemplateProgramPrivate *dptr) {
	QString value = this->value->evaluate (dptr).toString ();
	
	// Fast-path for constant regular expressions, like the case in 99% of all cases
	if (!this->test) {
		return this->regularExpr.match (value).hasMatch ();
	}
	
	QRegularExpression rx = evaluateRegEx (dptr);
	return rx.match (value).hasMatch ();
}

QRegularExpression Nuria::Template::MatchesTestNode::evaluateRegEx (TemplateProgramPrivate *dptr) {
	QString rxExpression = this->test->evaluate (dptr).toString ();
	QRegularExpression rx (rxExpression);
	
	if (!rx.isValid ()) {
		dptr->error = TemplateError (TemplateError::Compiler, TemplateError::InvalidRegularExpression,
		                             rx.errorString (), this->test->loc);
	}
	
	return rx;
}

Nuria::Template::Node *Nuria::Template::EmbedNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	MultipleNodes *nodes = dynamic_cast< MultipleNodes * > (subNode);
	if (!nodes) {
		TRACE(nCritical() << "EmbedNode" << this << "- The sub-node is not a 'MultipleNodes'!");
		return nullptr;
	}
	
	// Inject a 'extend' node into the sub-node
	TRACE(nDebug() << "EmbedNode" << this << "- Injecting 'extend' node to sub-node");
	nodes->nodes.prepend (new IncludeNode (this->loc, false, this->name));
	this->name = nullptr;
	
	// Swap out the current block mapping with a empty map
	TRACE(nDebug() << "EmbedNode" << this << "- Compiling sub-node with empty block mapping");
	SharedNode::BlockMap blocks;
	dptr->root->blocks.swap (blocks);
	
	// Compile body
	swapAndDestroy (this->subNode, this->subNode->compile (compiler, dptr));
	
	// Swap back to restore the old mapping
	dptr->root->blocks.swap (blocks);
	
	// Reduce to sub node.
	TRACE(nDebug() << "EmbedNode" << this << "- Reducing to sub-node" << subNode);
	Node *node = this->subNode;
	this->subNode = nullptr;
	return dptr->transferTrim (this, node);
	
}

Nuria::Template::Node *Nuria::Template::FilterNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	
	// Compile body
	TRACE(nDebug() << "FilterNode" << this << "compiling body" << this->body);
	swapAndDestroy (this->body, this->body->compile (compiler, dptr));
	
	MethodCallValueNode *innerMost = compileFunctions (compiler, dptr);
	if (!this->body || !innerMost) {
		TRACE(nError() << "FilterNode" << this << "failed to compile, body ="
		      << this->body << " inner-most =" << innerMost);
		return nullptr;
	}
	
	// Prepare inner method to receive the body render result as argument.
	innerMost->arguments->values.prepend (new LiteralValueNode (this->loc, QString ()));
	
	return this;
}

QString Nuria::Template::FilterNode::render (TemplateProgramPrivate *dptr) {
	
	// Inject body result as first argument to the inner-most method
	LiteralValueNode *value = dynamic_cast< LiteralValueNode * > (this->inner->arguments->values.first ());
	value->value = this->body->render (dptr);
	
	// Return result of the outer method
	return this->outer->render (dptr);
}

Nuria::Template::MethodCallValueNode *Nuria::Template::FilterNode::compileFunctions (Compiler *compiler,
                                                                                     TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "FilterNode" << this << "compiling" << funcs.length () << "filters");
	for (int i = 0; i < this->funcs.length (); i++) {
		swapAndDestroy (this->funcs[i], (MethodCallValueNode *)this->funcs[i]->compile (compiler, dptr));
		
		if (!this->funcs[i]) {
			return nullptr;
		}
		
		// Combine a|b|c to c(b(a(..)))
		if (i > 0) {
			this->funcs[i]->arguments->values.prepend (this->funcs[i - 1]);
		}
		
	}
	
	// Save inner and outer method for later access
	this->inner = this->funcs.first ();
	this->outer = this->funcs.last ();
	this->funcs.clear ();
	
	return inner;
}

Nuria::Template::Node *Nuria::Template::AutoescapeNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	TRACE(nDebug() << "AutoescapeNode" << this << "compiling body" << this->body << "for mode" << this->mode);
	
	swapAndDestroy (this->body, this->body->compile (compiler, dptr));
	this->escapeMode = Builtins::parseEscapeMode (this->mode);
	
	// Sanity check
	if (!this->body || this->escapeMode == EscapeMode::Verbatim) {
		if (this->escapeMode == EscapeMode::Verbatim) {
			dptr->error = TemplateError (TemplateError::Compiler, TemplateError::InvalidEscapeMode,
			                             QStringLiteral("Invalid escape mode '%1'").arg (this->mode),
			                             this->loc);
		}
		
		return nullptr;
	}
	
	// Done.
	return this;
}

// Template class, which keeps the value of a certain variable and resets the
// variables value to the kept value when the instance is destroyed.
template< typename T >
class VariableKeeper {
	T &variable;
	T oldValue;
public:
	constexpr VariableKeeper (T &variable)
	        : variable (variable), oldValue (variable)
	{ }
	
	VariableKeeper (T &variable, const T &newValue)
	        : variable (variable), oldValue (variable)
	{ variable = newValue; }
	
	~VariableKeeper () { variable = oldValue; }
};

QString Nuria::Template::AutoescapeNode::render (TemplateProgramPrivate *dptr) {

	// Set current escape mode
	VariableKeeper< EscapeMode > modeKeeper (dptr->escapeMode, this->escapeMode);
	Q_UNUSED(modeKeeper);
	
	// Render and escape
	QString result = Builtins::escape (this->body->render (dptr), this->escapeMode);
	
	// Done.
	return result;
}

Nuria::Template::Node *Nuria::Template::SpacelessNode::compile (Compiler *compiler, TemplateProgramPrivate *dptr) {
	VariableKeeper< bool > spacelessKeeper (dptr->spaceless, true);
	Q_UNUSED(spacelessKeeper);
	
	swapAndDestroy (this->body, this->body->compile (compiler, dptr));
	return this;
}

QString Nuria::Template::SpacelessNode::render (TemplateProgramPrivate *dptr) {
	VariableKeeper< bool > spacelessKeeper (dptr->spaceless, true);
	Q_UNUSED(spacelessKeeper);
	
	return this->body->render (dptr);
}

Nuria::Template::Node *Nuria::Template::TextNode::compile (Compiler *, TemplateProgramPrivate *dptr) {
	if (dptr->spaceless) {
		trimSpacesBetweenHtmlTags ();
	}
	
	return this;
}

void Nuria::Template::TextNode::trimSpacesBetweenHtmlTags () {
	static const QRegularExpression rx (QStringLiteral("(^|>)[[:space:]]+(<|$)"));
	
	// Remove space between tags
	this->text.replace (rx, QStringLiteral("\\1\\2"));
	
}
