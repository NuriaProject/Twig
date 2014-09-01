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

#ifndef NURIA_TEMPLATELOADER_HPP
#define NURIA_TEMPLATELOADER_HPP

#include <QObject>

namespace Nuria {

/**
 * \brief Loader of template data for TemplateEngine.
 * 
 * A TemplateLoader can find templates by name and make them available to the
 * engine and other templates.
 * 
 * \note Template names are considered to be case-sensitive.
 * 
 * \par Program caching
 * 
 * Template loaders should make use of the two signals templateChanged and
 * allTemplatesChanged to notify listeners, e.g. TemplateProgram caches, of
 * changes to a template source file.
 * 
 */
class TemplateLoader : public QObject {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit TemplateLoader (QObject *parent = 0);
	
	/** Destructor. */
	~TemplateLoader () override;
	
	/**
	 * Returns \c true if a template \a name is known to the template
	 * loader. The default implementation returns \c true if \a name is
	 * not empty.
	 */
	virtual bool hasTemplate (const QString &name);
	
	/**
	 * Returns the template data of the template called \a name.
	 * The default implementation returns \a name.
	 * 
	 * A empty result is treated as failure.
	 */
	virtual QByteArray load (const QString &name);
	
signals:
	
	/**
	 * Signal emitted when a specific template has changed.
	 */
	void templateChanged (const QString &name);
	
	/**
	 * Signal emitted when all (or an unknown amount of) templates have
	 * changed.
	 */
	void allTemplatesChanged ();
	
};

}

#endif // NURIA_TEMPLATELOADER_HPP
