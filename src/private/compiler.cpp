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

#include "compiler.hpp"

#include "../nuria/templateengine.hpp"
#include "../nuria/templateloader.hpp"
#include "templateengine_p.hpp"
#include "tokenizer.hpp"
#include "astnodes.hpp"
#include "parser.hpp"
#include <QDateTime>

Nuria::Template::Compiler::Compiler (TemplateEngine *engine, TemplateEnginePrivate *dptr)
        : QObject (engine), d_ptr (dptr)
{
	
}

Nuria::Template::Compiler::~Compiler () {
	// d_ptr is owned by the template engine instance.
}

bool Nuria::Template::Compiler::compile (TemplateProgramPrivate *program) {
	Node *result = program->root->node->compile (this, program);
	
	// 
	if (result != program->root->node) {
		delete program->root->node;
		program->root->node = result;
	}
	
	program->compiledAt = QDateTime::currentDateTime ();
	return result;
}

Nuria::Template::Node *Nuria::Template::Compiler::loadAndParse (const QString &templateName, TemplateProgramPrivate *dptr) {
	QByteArray templ = this->d_ptr->loader->load (templateName);
	dptr->dependencies.append (templateName);
	
	if (templ.isEmpty ()) {
		dptr->error = TemplateError (TemplateError::Loader,
		                             TemplateError::TemplateNotFound,
		                             templateName);
		return nullptr;
	}
	
	// 
	return parseCode (templ, dptr);
}

Nuria::Template::Node *Nuria::Template::Compiler::parseCode (const QByteArray &code, TemplateProgramPrivate *dptr) {

	// Tokenize ..
	dptr->error = TemplateError ();
	this->d_ptr->tokenizer->read (code);
	
	// Parse ..
	if (!this->d_ptr->parser->parse (this->d_ptr->tokenizer->allTokens (), dptr)) {
		dptr->error = this->d_ptr->parser->lastError ();
		return nullptr;
	}
	
	// Get node
	return this->d_ptr->parser->stealBaseNode ();
}

Nuria::TemplateEngine *Nuria::Template::Compiler::engine () const {
	return this->d_ptr->q_ptr;
}

Nuria::TemplateLoader *Nuria::Template::Compiler::loader () const {
	return this->d_ptr->loader;
}
