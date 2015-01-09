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

#ifndef NURIA_TEMPLATELOADER_HPP
#define NURIA_TEMPLATELOADER_HPP

#include "twig_global.hpp"
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
class NURIA_TWIG_EXPORT TemplateLoader : public QObject {
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
	 * Returns \c true if \a name has been changed \a since.
	 * The default implementation always returns \c false, which is fine for
	 * template loaders making use of the changed signals in this class.
	 */
	virtual bool hasTemplateChanged (const QString &name, const QDateTime &since);
	
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
