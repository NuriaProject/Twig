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

#include "compiler.hpp"

#include "../templateengine.hpp"
#include "../templateloader.hpp"
#include "templateengine_p.hpp"
#include "tokenizer.hpp"
#include "astnodes.hpp"
#include "parser.hpp"

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
	
	return result;
}

Nuria::Template::Node *Nuria::Template::Compiler::loadAndParse (const QString &templateName, TemplateProgramPrivate *dptr) {
	QByteArray templ = this->d_ptr->loader->load (templateName);
	
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
