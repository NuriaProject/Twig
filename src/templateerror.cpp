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

#include "templateerror.hpp"

namespace Nuria {
class TemplateErrorPrivate : public QSharedData {
public:
	
	TemplateError::Component component;
	TemplateError::Error error;
	Template::Location location;
	QString what;
	
};

}

Nuria::TemplateError::TemplateError (Component component, Error error,
                                     const QString &what, Template::Location location)
        : d (new TemplateErrorPrivate)
{
	
	this->d->component = component;
	this->d->error = error;
	this->d->what = what;
	this->d->location = location;
	
}

Nuria::TemplateError::TemplateError (const TemplateError &other)
        : d (other.d)
{
	// ..
}

Nuria::TemplateError::TemplateError (TemplateError &&other)
        : d (std::move (other.d))
{
	
}

Nuria::TemplateError::~TemplateError () {
	// ..
}

Nuria::TemplateError &Nuria::TemplateError::operator= (const TemplateError &other) {
	this->d = other.d;
	return *this;
}

Nuria::TemplateError &Nuria::TemplateError::operator= (Nuria::TemplateError &&other) {
	this->d = std::move (other.d);
	return *this;
}

bool Nuria::TemplateError::hasFailed () const {
	return (this->d->component != None && this->d->error != NoError);
}

Nuria::TemplateError::Component Nuria::TemplateError::component () const {
	return this->d->component;
}

QString Nuria::TemplateError::componentName () const {
	return componentName (this->d->component);
}

Nuria::TemplateError::Error Nuria::TemplateError::error () const {
	return this->d->error;
}

QString Nuria::TemplateError::errorName () const {
	return errorName (this->d->error);
}

QString Nuria::TemplateError::what () const {
	return this->d->what;
}

Nuria::Template::Location Nuria::TemplateError::location () const {
	return this->d->location;
}

QString Nuria::TemplateError::componentName (Nuria::TemplateError::Component component) {
	switch (component) {
	default: return QStringLiteral("<Unknown>");
	case None: return QStringLiteral("None");
	case Engine: return QStringLiteral("Engine");
	case Loader: return QStringLiteral("Loader");
	case Tokenizer: return QStringLiteral("Tokenizer");
	case Parser: return QStringLiteral("Parser");
	case Compiler: return QStringLiteral("Compiler");
	case Renderer: return QStringLiteral("Renderer");
	}
	
}

QString Nuria::TemplateError::errorName (Nuria::TemplateError::Error error) {
	switch (error) {
	default: return QStringLiteral("<Unknown>");
	case NoError: return QStringLiteral("NoError");
	case TemplateNotFound: return QStringLiteral("TemplateNotFound");
	case UnknownToken: return QStringLiteral("UnknownToken");
	case SyntaxError: return QStringLiteral("SyntaxError");
	case BadEndblockName: return QStringLiteral("BadEndblockName");
	case NonConstantExpression: return QStringLiteral("NonConstantExpression");
	case EmptyTemplateName: return QStringLiteral("EmptyTemplateName");
	case NoParentBlock: return QStringLiteral("NoParentBlock");
	case InvalidRegularExpression: return QStringLiteral("InvalidRegularExpression");
	case InvalidEscapeMode: return QStringLiteral("InvalidEscapeMode");
	case NoProgram: return QStringLiteral("NoProgram");
	case VariableNotSet: return QStringLiteral("VariableNotSet");
	}
	
}

QDebug operator<< (QDebug dbg, const Nuria::TemplateError &error) {
	dbg.nospace () << "TemplateError(";
	
	if (!error.hasFailed ()) {
		dbg << "<No error>";
	} else {
		Nuria::Template::Location loc = error.location ();
		dbg << error.componentName () << " -> " << error.errorName ()
		    << " -> " << error.what () << " at [" << loc.column << "|" << loc.row << "]";
	}
	
	dbg << ") ";
	return dbg.space ();
}

bool Nuria::Template::Location::operator<(const Location &right) const {
	if (this->row <= right.row && this->column < right.column) {
		return true;
	}
	
	return (this->row < right.row);
}
