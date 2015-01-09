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

#ifndef NURIA_TEMPLATE_PARSER_HPP
#define NURIA_TEMPLATE_PARSER_HPP

#include "../nuria/templateerror.hpp"
#include <nuria/tokenizer.hpp>
#include "astnodes.hpp"
#include <QObject>

namespace Nuria {

class TemplateEngine;

namespace Template {

class ParserPrivate;

/**
 * \brief Internal parser for Twig code.
 */
class Parser : public QObject {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit Parser (TemplateEngine *engine);
	
	/** Destructor. */
	~Parser () override;
	
	/** Parses \a tokens. Returns \c true on success. */
	bool parse (const QVector<Token> &tokens, TemplateProgramPrivate *dptr);
	
	/** Returns the base node after a successful parsing operation. */
	Node *baseNode ();
	
	/** Steals the base node. Ownership is transferred to the caller. */
	Node *stealBaseNode ();
	
	/** Destroys the AST. */
	void clear ();
	
	/** Returns the last error. */
	TemplateError lastError () const;
	
private:
	ParserPrivate *d_ptr;
	
};

}
}

#endif // NURIA_TEMPLATE_PARSER_HPP
