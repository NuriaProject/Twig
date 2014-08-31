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
