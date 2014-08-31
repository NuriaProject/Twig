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

#include "templateprogram.hpp"

#include "private/templateengine_p.hpp"
#include "private/astnodes.hpp"

Nuria::TemplateProgram::TemplateProgram ()
        : d (nullptr)
{
	
}

Nuria::TemplateProgram::TemplateProgram (const Nuria::TemplateProgram &other)
        : d (other.d)
{
	refNode ();
}

Nuria::TemplateProgram::TemplateProgram (TemplateProgramPrivate *dptr)
        : d (dptr)
{
	refNode ();
	
}

void Nuria::TemplateProgram::refNode () {
	if (this->d && this->d->root) {
		this->d->root->ref.ref ();
	}
	
}

void Nuria::TemplateProgram::derefNode () {
	if (this->d && this->d->root) {
		this->d->root->ref.deref ();
	}
	
}

Nuria::TemplateProgram &Nuria::TemplateProgram::operator= (const TemplateProgram &other) {
	if (this->d != other.d) {
		derefNode ();
		this->d = other.d;
		refNode ();
	}
	
	return *this;
}

Nuria::TemplateProgram::~TemplateProgram () {
	derefNode ();
}

bool Nuria::TemplateProgram::isValid () const {
	return this->d;
}

QStringList Nuria::TemplateProgram::dependencies () const {
	if (!this->d) {
		return QStringList ();
	}
	
	return this->d->dependencies;
}

QStringList Nuria::TemplateProgram::neededVariables () const {
	if (!this->d) {
		return QStringList ();
	}
	
	return this->d->variables;
}

QVariant Nuria::TemplateProgram::value (const QString &variable) const {
	if (!this->d) {
		return QVariant ();
	}
	
	int idx = this->d->variables.indexOf (variable);
	if (idx >= 0) {
		return this->d->values.at (idx);
	}
	
	return QVariant ();
}

bool Nuria::TemplateProgram::setValue (const QString &variable, const QVariant &value) {
	if (!this->d) {
		return false;
	}
	
	int idx = this->d->variables.indexOf (variable);
	if (idx >= 0) {
		this->d->values.replace (idx, value);
		return true;
	}
	
	return false;
	
}

QLocale Nuria::TemplateProgram::locale () const {
	if (this->d) {
		return this->d->locale;
	}
	
	return QLocale ();
}

void Nuria::TemplateProgram::setLocale (const QLocale &locale) {
	if (this->d) {
		this->d->locale = locale;
	}
	
}

void Nuria::TemplateProgram::addFunction (const QString &name, const Nuria::Callback &function) {
	if (this->d) {
		this->d->functions.insert (name, { function, false });
	}
	
}

bool Nuria::TemplateProgram::hasFunction (const QString &name) {
	if (this->d) {
		return this->d->functions.contains (name);
	}
	
	return false;
}

bool Nuria::TemplateProgram::canRender () const {
	if (!this->d) {
		return false;
	}
	
	// Program
	if (!this->d->root || !this->d->root->node) {
		this->d->error = TemplateError (TemplateError::Renderer, TemplateError::NoProgram,
		                                QStringLiteral("There's no program to render"),
		                                Template::Location ());
		return false;
	}
	
	// Variable check
	for (int i = 0; i < this->d->values.length (); i++) {
		if (!checkVariable (i)) {
			this->d->error = TemplateError (TemplateError::Renderer, TemplateError::VariableNotSet,
			                                this->d->variables.at (i), Template::Location ());
			return false;
		}
		
	}
	
	// 
	return true;
	
}

bool Nuria::TemplateProgram::checkVariable (int index) const {
	if (this->d->values.at (index).isValid ()) {
		return true;
	}
	
	// Check if the variable is written to before it's first read from.
	// If that's the case, then the variable will be set until it's used.
	return this->d->isFirstUsageRecordWriting (index);
}

QString Nuria::TemplateProgram::render () {
	if (!canRender ()) {
		return QString ();
	}
	
	// Render.
	TemplateProgramPrivate *dptr = const_cast< TemplateProgramPrivate * > (this->d.constData ());
	return this->d->root->node->render (dptr);
	
}

Nuria::TemplateError Nuria::TemplateProgram::lastError () const {
	if (!this->d) {
		return TemplateError (TemplateError::Renderer, TemplateError::NoProgram,
		                      QStringLiteral("There's no program to render"),
                                      Template::Location ());
	}
	
	return this->d->error;
}
