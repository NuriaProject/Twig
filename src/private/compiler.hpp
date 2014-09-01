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

#ifndef NURIA_TEMPLATE_COMPILER_HPP
#define NURIA_TEMPLATE_COMPILER_HPP

#include "../nuria/templateerror.hpp"
#include <QVariant>
#include <QObject>

namespace Nuria {

class TemplateProgramPrivate;
class TemplateEnginePrivate;
class TemplateProgram;
class TemplateEngine;
class TemplateLoader;

namespace Template {

class Node;

/**
 * \internal
 * \brief Internal class for compiling nodes.
 * 
 * This internal class takes a Twig AST node and shuffles around some nodes to
 * implement Twig features like extend or include (Loading the associated
 * templates).
 */
class Compiler : public QObject {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit Compiler (TemplateEngine *engine, TemplateEnginePrivate *dptr);
	
	/** Destructor. */
	~Compiler () override;
	
	/**
	 * Loads and parses \a templateName, returning \c nullptr and setting
	 * \a error on failure. Else, the parsed node is returned.
	 * 
	 * \note Ownership of the returned instance is transferred to the
	 * caller.
	 * \note The returned node is not compiled.
	 */
	Node *loadAndParse (const QString &templateName, TemplateProgramPrivate *dptr);
	
	/**
	 * Like loadAndParse(), but takes a \a code snippet instead of a template.
	 */
	Node *parseCode (const QByteArray &code, TemplateProgramPrivate *dptr);
	
	/** Compiles \a program and returns \c true on success. */
	bool compile (TemplateProgramPrivate *program);
	
	TemplateEngine *engine () const;
	TemplateLoader *loader () const;
	
private:
	
	TemplateEnginePrivate *d_ptr;
	
};

}
}

#endif // NURIA_TEMPLATE_COMPILER_HPP
