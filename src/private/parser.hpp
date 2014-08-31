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

#ifndef NURIA_TEMPLATE_PARSER_HPP
#define NURIA_TEMPLATE_PARSER_HPP

#include "../templateerror.hpp"
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
