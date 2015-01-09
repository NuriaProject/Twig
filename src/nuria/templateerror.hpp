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

#ifndef NURIA_TEMPLATEERROR_HPP
#define NURIA_TEMPLATEERROR_HPP

#include "twig_global.hpp"
#include <QSharedData>
#include <QDebug>

namespace Nuria {

class TemplateErrorPrivate;

namespace Template {

/**
 * \brief Storage for a location in Twig code.
 * 
 * This plain structure is used while all stages of the TemplateEngine to store
 * positions of code in the Twig code.
 * 
 * The structure is flat, meaning you directly access the fields. This was
 * chosen due to the runtime overhead of shared structures to avoid performance
 * issues while tokenizing and parsing the Twig code.
 * 
 * Both the row and column begin counting from zero.
 * 
 * \note The debug operator of TemplateError also pretty-prints the location.
 */
struct NURIA_TWIG_EXPORT Location {
	constexpr Location (int row = 0, int column = 0)
	        : row (row), column (column)
	{ }
	
	/** Row */
	int row;
	
	/** Column */
	int column;
	
	/** Returns \c true if this instance comes before \a right. */
	bool operator< (const Location &right) const;
	
};

}

/**
 * \brief Container for exposing Twig errors to user code.
 * 
 * This class is used to indicate errors when processing Twig code through
 * various stages. It's essentially like a exception.
 * 
 * You can query the component, in which the error occured in, and the
 * corresponding error code using the getters. For easier debugging, you can
 * directly log instances of this class to QDebug and Nuria::Debug.
 * 
 * \sa component error what location
 * 
 */
class NURIA_TWIG_EXPORT TemplateError {
public:
	
	/** List of components to indicate where processing failed. */
	enum Component {
		None = 0,
		Engine,
		Loader,
		Tokenizer,
		Parser,
		Compiler,
		Renderer
	};
	
	/** Error codes. */
	enum Error {
		NoError = 0,
		
		/** Loader: A template was not found. */
		TemplateNotFound = 100,
		
		/** Tokenizer: Unknown token. */
		UnknownToken = 200,
		
		/** Parser: Syntax error. */
		SyntaxError = 300,
		
		/** Parser: The block name given in a blockend tag did not match
		 * the one given in the block tag.
		 */
		BadEndblockName,
		
		/**
		 * Compiler: A value which is needed to be constant was not a
		 * constant.
		 */
		NonConstantExpression = 400,
		
		/**
		 * Compiler: Template name given in include or extend command is
		 * empty.
		 */
		EmptyTemplateName,
		
		/**
		 * Compiler: A call to to parent() outside of a block or in the
		 * initial block declaration was found.
		 */
		NoParentBlock,
		
		/**
		 * Compiler: The regular expression for a 'matches' test was
		 * invalid.
		 */
		InvalidRegularExpression,
		
		/**
		 * Compiler, Renderer: Unknown escape-mode given to a autoescape
		 * block or the escape() filter.
		 */
		InvalidEscapeMode,
		
		/** Renderer: No parsed program available. */
		NoProgram = 500,
		
		/** Renderer: A needed variable is not set. */
		VariableNotSet,
		
	};
	
	/** Constructor. */
	TemplateError (Component component = None, Error error = NoError, const QString &what = QString(),
	               Template::Location location = Template::Location ());
	
	/** Copy constructor. */
	TemplateError (const TemplateError &other);
	
	/** Move constructor. */
	TemplateError (TemplateError &&other);
	
	/** Destructor. */
	~TemplateError ();
	
	/** Assignment operator. */
	TemplateError &operator= (const TemplateError &other);
	
	/** Move assignment operator. */
	TemplateError &operator= (TemplateError &&other);
	
	/** Returns \c true if this object contains an error. */
	bool hasFailed () const;
	
	/** Returns the component which threw the error. */
	Component component () const;
	
	/** Returns the name of the component. */
	QString componentName () const;
	
	/** Returns the error code. */
	Error error () const;
	
	/** Returns the name of the error code. */
	QString errorName () const;
	
	/** Returns the further description of the error. */
	QString what () const;
	
	/** Returns the location where in the Twig code the error occured. */
	Template::Location location () const;
	
	/** Returns the name of the component. */
	static QString componentName (Component component);
	
	/** Returns the human-readable name for \a error. */
	static QString errorName (Error error);
	
private:
	QExplicitlySharedDataPointer< TemplateErrorPrivate > d;
};

}

/** Debug operator for Nuria::TemplateError. */
NURIA_TWIG_EXPORT QDebug operator<< (QDebug dbg, const Nuria::TemplateError &error);

#endif // NURIA_TEMPLATEERROR_HPP
