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

#ifndef NURIA_TEMPLATEENGINE_HPP
#define NURIA_TEMPLATEENGINE_HPP

#include <nuria/callback.hpp>
#include "twig_global.hpp"
#include <QVariant>
#include <QObject>

#include "templateprogram.hpp"
#include "templateerror.hpp"

namespace Nuria {

class TemplateEnginePrivate;
class TemplateLoader;
class TemplateStack;

/**
 * \brief Templating engine for rendering Twig code
 * 
 * This is a templating engine capable of rendering Twig code. Twig is a
 * template language popular in PHP and is developed by SensioLabs at
 * http://twig.sensiolabs.org/
 * 
 * It features a rich amount of possibilities to efficiently write templates
 * especially for HTML pages, though in its core it's target agnostic, meaning
 * you could also e.g. write templates for INI files.
 * 
 * For a beginners tutorial, please go to
 * http://twig.sensiolabs.org/doc/templates.html
 * 
 * \note The implementation is based on Twig version 1.16.0
 * 
 * \par Features
 * 
 * Supported features:
 * 
 * - Twig command blocks, expansions and comments
 * - Flow control and loops (if, for)
 * - All operators
 * - All tags, commands, filters and functions if not noted otherwise
 * - Accessing fields and methods in maps, lists, QObjects and structures registered to the Nuria meta system
 * 
 * Unsupported features are:
 * 
 * - The use command
 * - verbatim and sandbox blocks.
 * - The convert_encoding() filter. Everything is UTF-8.
 * - macros
 * - attribute() - Please use the square-braces syntax instead.
 * - The date_modify filter.
 * - The format filter.
 * - The raw filter.
 * - The constant() test function.
 * - The sameas/same as test function is obsolete, as == is the same.
 * - Binary operations
 * - The flush command, as it wouldn't do anything at all.
 * - The source() function.
 * - The slice() shorthand notation "[x:y]"
 * 
 * Implementation differences:
 * 
 * - Input is always considered to be UTF-8
 * - Template names in include and extends commands \b must be constant
 * - Include and extends only support the basic form of {% include/extends "foo" %}
 * - The date filter expects a QDateTime or string in the Qt::TextDate format in the C locale.
 * - The date filter does not support the timezone argument.
 * - The format string for the date filter is optional. It defaults to the locale long format.
 * - The json_encode filter does not support additional arguments.
 * - The url_encode filter does not support the 'rawurlencode' bool argument.
 * - The reverse filter doesn't support maps.
 * - "!", "&&" and "||" are aliases for "not", "and" and "or" respectively.
 * - The == operator is equivalent to PHPs ===.
 * - The matches test uses QRegularExpression syntax.
 * - The 'loop.parent' variable inside for-loops only contains a 'loop' variable.
 * 
 * \par Performance
 * 
 * The internal AST is not transformed to C++ nor JIT'ed. The only real but
 * powerful optimization done is constant folding. Thus, Twig code like
 * \code
 * {% if 1 > 2 %}
 * Foo
 * {% else %}
 * Bar
 * {% endif %}
 * \endcode
 * 
 * Is optimized to simply "Bar". If a template were to only consist out of
 * such constant constructs, it is folded into a single QString. Thus, to
 * use TemplateEngine as constant folding calculator, you could do:
 * \code
 * nDebug() << TemplateEngine ().render ("{{ 1 + 2 }}");
 * \endcode
 * 
 * \par Caching of programs
 * 
 * Another optimization worth noting is that programs are cached. You can
 * control this behaviour using setMaxCacheSize().
 * 
 * \par Variable inheritance and strictness
 * 
 * This engine will always, even if only internally, generate instances of
 * TemplateProgram. To provide a balanced mix between ease of use and
 * performance, this class can pass on variables to all generated programs.
 * 
 * In simpler words, all variables you set to a TemplateEngine will be
 * automatically set in all generated programs. Changing variables of the
 * engine will \b not affect already generated programs.
 * 
 * You can use this to e.g. set company or product specific variables which
 * are used in all templates and set template-specific variables in the
 * TemplateProgram.
 * 
 * Please note that this implementation of a Twig engine is pretty strict when
 * it comes to variables and functions. All functions referenced by a template
 * must be present when the template is loaded. Variables must be present upon
 * rendering.
 * 
 * If a function or a variable is missing, by default, a error will be issued
 * and the operation is cancelled.
 * 
 * \par Variable access
 * 
 * The engine supports all C++ POD types and QVariantList and QVariantMap,
 * including all types being accessible through QSequentialIterable and
 * QAssociativeIterable.
 * 
 * QObjects are supported too, but only access to properties is provided.
 * Objects registered to the Nuria meta system support access to fields and
 * methods, though the first method is always chosen when the method is
 * overloaded.
 * 
 * \par Custom functions
 * 
 * It's possible to register custom functions using the addFunction() method.
 * It's also possible for the TemplateEngine version to define a function as
 * constant, meaning that for the same arguments they produce the same result.
 * This will enable the internal compiler to optimize functions, which are
 * constant, meaning they'll be invoked at compile-time.
 * 
 * This can also be used to register custom filters. A filter will simply be
 * passed the result of the previous statement as first argument. All additional
 * arguments are then passed after that.
 * 
 * Example: "foo|bar(1,2)" is equivalent to "bar(foo,1,2)".
 * 
 * \par Locale
 * 
 * Locale-dependent functions will use the application-wide default locale by
 * default. You can also set this using setLocale() on the template engine
 * or program.
 * 
 * \warning setLocale() has no effect on cached programs. To force this,
 * call flushCache() afterwards.
 * 
 */
class NURIA_TWIG_EXPORT TemplateEngine : public QObject {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit TemplateEngine (QObject *parent = 0);
	
	/** Destructor. */
	~TemplateEngine () override;
	
	/**
	 * Returns the default locale for programs. If not set, this is
	 * equivalent to the default application-wide QLocale.
	 */
	QLocale locale () const;
	
	/** Sets the locale for programs. */
	void setLocale (const QLocale &locale);
	
	/** Returns the used template loader. */
	TemplateLoader *loader () const;
	
	/** Returns the maximum count of programs to be cached. */
	int maxCacheSize () const;
	
	/**
	 * Resizes the internal cache to \a size. If \a size is \c 0, the cache
	 * is effectively disabled.
	 */
	void setMaxCacheSize (int size);
	
	/** Returns the amount of currently cached programs. */
	int currentCacheSize () const;
	
	/** Returns \c true if \a templateName is already cached. */
	bool isTemplateInCache (const QString &templateName) const;
	
	/**
	 * Replaces the currently used template loader.
	 * Ownership of \a loader is transferred to the environment.
	 */
	void setLoader (TemplateLoader *loader);
	
	/** Returns the value of \a name. */
	QVariant value (const QString &name) const;
	
	/** Returns the map of values to render templates. */
	QVariantMap values () const;
	
	/** Sets the values to \a map, replacing the internal one. */
	void setValues (const QVariantMap &map);
	
	/**
	 * Merges \a map with the internal value map.
	 * 
	 * All values of \a map will be inserted into the internal map,
	 * overriding the internal one if a key already exists.
	 */
	void mergeValues (const QVariantMap &map);
	
	/** Inserts \a value as \a name into the value map of the engine. */
	void setValue (const QString &name, const QVariant &value);
	
	/**
	 * Adds \a function, making it known as \a name. Functions that are
	 * constant, meaning that for the same arguments they always output the
	 * same result, should set \a isConstant to \c true to make it subject
	 * for constant folding during the compilation step.
	 * 
	 * \note Built-in functions can not be overridden.
	 */
	void addFunction (const QString &name, const Callback &function, bool isConstant = false);
	
	/**
	 * Returns \c true, if there's a user-defined function called \a name.
	 */
	bool hasFunction (const QString &name);
	
	/**
	 * Loads and compiles a Twig template called \a templateName and returns
	 * a TemplateProgram, which can be cached by the user if wanted.
	 * The program will be prepopulated with the variables set to the
	 * engine.
	 * 
	 * \note A TemplateProgram is \b not dependent on a TemplateEngine.
	 * \note Caching does not interfer with the variable being passed on to
	 * the program!
	 */
	TemplateProgram program (const QString &templateName);
	
	/**
	 * Loads the template \a templateName and renders it into a string.
	 * If \a templateName is unknown, an empty string is returned.
	 * 
	 * Same as:
	 * \code
	 * myTemplateEngine->program ("foo").render ();
	 * \endcode
	 */
	QString render (const QString &templateName);
	
	/**
	 * Returns the last occured error.
	 * \note render() clears this.
	 */
	TemplateError lastError () const;
	
	/**
	 * Checks if \a program is outdated, meaning, if the templates it
	 * consists of were modified after \a program was compiled.
	 */
	bool isProgramOutdated (const TemplateProgram &program);
	
	/** Clears the program cache. */
	void flushCache ();
	
private:
	
	TemplateProgramPrivate *createProgram (const QString &templateName);
	void removeChangedTemplateFromCache (const QString &templateName);
	TemplateProgram updateProgramVariables (const QString &templateName, TemplateProgram *prog);
	void connectToLoaderSignals ();
	
	TemplateEnginePrivate *d_ptr;
	
};

}

#endif // NURIA_TEMPLATEENGINE_HPP
