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
