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

#include "nuria/templateengine.hpp"

#include <nuria/logger.hpp>

#include "private/templateengine_p.hpp"
#include "nuria/templateloader.hpp"
#include "private/tokenizer.hpp"
#include "private/compiler.hpp"
#include "private/parser.hpp"

Nuria::TemplateEngine::TemplateEngine (QObject *parent)
	: QObject (parent), d_ptr (new TemplateEnginePrivate)
{
	
	this->d_ptr->q_ptr = this;
	this->d_ptr->renderer = new Template::Compiler (this, this->d_ptr);
	
	this->d_ptr->parser = new Template::Parser (this);
	this->d_ptr->tokenizer = new Template::Tokenizer (this);
	this->d_ptr->loader = new TemplateLoader (this);
	connectToLoaderSignals ();
	
}

Nuria::TemplateEngine::~TemplateEngine () {
	delete this->d_ptr;
}

QLocale Nuria::TemplateEngine::locale () const {
	return this->d_ptr->locale;
}

void Nuria::TemplateEngine::setLocale (const QLocale &locale) {
	this->d_ptr->locale = locale;
}

Nuria::TemplateLoader *Nuria::TemplateEngine::loader () const {
	return this->d_ptr->loader;
}

int Nuria::TemplateEngine::maxCacheSize () const {
	return this->d_ptr->cache.maxCost ();
}

void Nuria::TemplateEngine::setMaxCacheSize (int size) {
	this->d_ptr->cache.setMaxCost (size);
}

int Nuria::TemplateEngine::currentCacheSize () const {
	return this->d_ptr->cache.count ();
}

bool Nuria::TemplateEngine::isTemplateInCache (const QString &templateName) const {
	return this->d_ptr->cache.contains (templateName);
}

void Nuria::TemplateEngine::setLoader (Nuria::TemplateLoader *loader) {
	if (loader != this->d_ptr->loader) {
		delete this->d_ptr->loader;
		this->d_ptr->loader = loader;
		loader->setParent (this);
		
		connectToLoaderSignals ();
	}
	
}

QVariant Nuria::TemplateEngine::value (const QString &name) const {
	return this->d_ptr->values.value (name);
}

QVariantMap Nuria::TemplateEngine::values () const {
	return this->d_ptr->values;
}

void Nuria::TemplateEngine::setValues (const QVariantMap &map) {
	this->d_ptr->versionId++;
	this->d_ptr->values = map;
}

void Nuria::TemplateEngine::mergeValues (const QVariantMap &map) {
	this->d_ptr->versionId++;
	
	auto it = map.constBegin ();
	auto end = map.constEnd ();
	for (; it != end; ++it) {
		this->d_ptr->values.insert (it.key (), it.value ());
	}
	
}

void Nuria::TemplateEngine::setValue (const QString &name, const QVariant &value) {
	this->d_ptr->versionId++;
	this->d_ptr->values.insert (name, value);
}

void Nuria::TemplateEngine::addFunction (const QString &name, const Nuria::Callback &function, bool isConstant) {
	this->d_ptr->versionId++;
	this->d_ptr->functions.insert (name, { function, isConstant });
}

bool Nuria::TemplateEngine::hasFunction (const QString &name) {
	return this->d_ptr->functions.contains (name);
}

Nuria::TemplateProgram Nuria::TemplateEngine::program (const QString &templateName) {
	TemplateProgram *ptr = this->d_ptr->cache.object (templateName);
	if (ptr && !isProgramOutdated (*ptr)) {
		return updateProgramVariables (templateName, ptr);
	}
	
	// Cache miss, load and compile the program.
	TemplateProgram program (createProgram (templateName));
	
	if (this->d_ptr->cache.maxCost () > 0) {
		this->d_ptr->cache.insert (templateName, new TemplateProgram (program));
	}
	
	return program;
}

QString Nuria::TemplateEngine::render (const QString &templateName) {
	TemplateProgram instance = program (templateName);
	this->d_ptr->lastError = instance.lastError ();
	
	// Error check
	if (this->d_ptr->lastError.hasFailed ()) {
		return QString ();
	}
	
	// Render.
	QString result = instance.render ();
	this->d_ptr->lastError = instance.lastError ();
	return result;
}

Nuria::TemplateError Nuria::TemplateEngine::lastError () const {
	return this->d_ptr->lastError;
}

Nuria::TemplateProgramPrivate *Nuria::TemplateEngine::createProgram (const QString &templateName) {
	TemplateProgramPrivate *program = new TemplateProgramPrivate;
	program->info = new CompileInformation;
	
	Template::Node *node = this->d_ptr->renderer->loadAndParse (templateName, program);
	if (!node) {
		if (!program->error.hasFailed ()) {
			program->error = this->d_ptr->parser->lastError ();
		}
		
		delete program->info;
		program->info = nullptr;	
		return program;
	}
	
	// Copy function map to allow for custom constant functions
	program->functions = this->d_ptr->functions;
	
	// Compile
	program->root = new Template::SharedNode (node);
	this->d_ptr->renderer->compile (program);
	
	// Cleanup
	delete program->info;
	program->info = nullptr;	
	
	// Populate variables
	for (int i = 0; i < program->variables.length (); i++) {
		const QString &name = program->variables.at (i);
		program->values[i] = this->d_ptr->values.value (name);
	}
	
	// Done.
	return program;
}

void Nuria::TemplateEngine::removeChangedTemplateFromCache (const QString &templateName) {
	this->d_ptr->cache.remove (templateName);
	
	// Remove all templates which depend on the changed template.
	QStringList allTemplates = this->d_ptr->cache.keys ();
	for (int i = 0, total = allTemplates.length (); i < total; ++i) {
		TemplateProgram *prog = this->d_ptr->cache.object (allTemplates.at (i));
		
		// Does 'prog' depend on the changed template?
		if (prog && prog->d->dependencies.contains (templateName)) {
			this->d_ptr->cache.remove (allTemplates.at (i));
		}
		
	}
	
}

Nuria::TemplateProgram Nuria::TemplateEngine::updateProgramVariables (const QString &templateName,
                                                                      TemplateProgram *prog) {
	if (prog->d->versionId == this->d_ptr->versionId) {
		return *prog;
	}
	
	// NOTE: If TemplateEngine would become thread-safe, this is a place where things could go wrong.
	
	// Version mismatch
	// Only copy if we're not the only one holding a reference.
	if (prog->d->ref.load () > 1) {
		TemplateProgram *instance = new TemplateProgram (*prog);
		this->d_ptr->cache.insert (templateName, instance);
		prog = instance;
	}
	
	// Update functions
	prog->d->functions = this->d_ptr->functions;
	
	// Update variables
	for (int i = 0; i < prog->d->variables.length (); i++) {
		prog->d->values.replace (i, this->d_ptr->values.value (prog->d->variables.at (i)));
	}
	
	// Done
	prog->d->versionId = this->d_ptr->versionId;
	return *prog;
}

void Nuria::TemplateEngine::connectToLoaderSignals () {
	connect (this->d_ptr->loader, &TemplateLoader::templateChanged,
	         this, &TemplateEngine::removeChangedTemplateFromCache);
	connect (this->d_ptr->loader, &TemplateLoader::allTemplatesChanged,
	         this, &TemplateEngine::flushCache);
}

bool Nuria::TemplateEngine::isProgramOutdated (const TemplateProgram &program) {
	auto it = program.d->dependencies.constBegin ();
	auto end = program.d->dependencies.constEnd ();
	
	for (; it != end; ++it) {
		if (this->d_ptr->loader->hasTemplateChanged (*it, program.d->compiledAt)) {
			return true;
		}
		
	}
	
	return false;
}

void Nuria::TemplateEngine::flushCache () {
	this->d_ptr->cache.clear ();
}
