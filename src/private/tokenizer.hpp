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

#ifndef NURIA_TEMPLATE_TOKENIZER_HPP
#define NURIA_TEMPLATE_TOKENIZER_HPP

#include <QObject>
#include "astnodes.hpp"
#include <nuria/tokenizer.hpp>

namespace Nuria {
namespace Template {

class TokenizerPrivate;

/**
 * \brief Tokenizer for Twig code.
 * 
 * Used by TemplateEngine to tokenize Twig code.
 */
class Tokenizer : public QObject {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit Tokenizer (QObject *parent = 0);
	
	/** Destructor. */
	~Tokenizer () override;
	
	/** Tokenizes Twig code in \a data. */
	void read (const QByteArray &data);
	
	/** Returns a list of all parsed tokens. */
	QVector< Token > allTokens () const;
	
	/** Returns the next token for testing purposes. */
	Token nextToken ();
	
	/** Returns the current parser position. */
	int pos () const;
	
	/** Returns \c true if all available tokens were read. */
	bool atEnd () const;
	
	/** Clears the internal state. */
	void reset ();
	
private:
	
	void pushTextToken (Token token, int previousTokId);
	bool tokenizeAndPush (QByteArray code, Location &loc, bool switchToDefault);
	bool hasTrimTokenStart (const QByteArray &code);
	bool hasTrimTokenEnd (const QByteArray &code);
	void parseFirstStage (const QByteArray &data);
	void pushComment (Token token);
	bool tokenizeExpansionsAndCommands ();
	void removeTrailingEmptyLineInLastTextToken ();
	
	TokenizerPrivate *d_ptr;
	
};

}
}

#endif // NURIA_TEMPLATETOKENIZER_HPP
