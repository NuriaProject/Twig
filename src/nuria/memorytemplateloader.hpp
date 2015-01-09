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

#ifndef NURIA_MEMORYTEMPLATELOADER_HPP
#define NURIA_MEMORYTEMPLATELOADER_HPP

#include "templateloader.hpp"
#include "twig_global.hpp"
#include <QMap>

namespace Nuria {

class MemoryTemplateLoaderPrivate;

/**
 * \brief A template loader which stores templates in-memory.
 * 
 * This template loader is essentially a wraper around a associative array,
 * as in a QMap, storing templates completely in-memory.
 * 
 * \note Template names are case-sensitive
 */
class NURIA_TWIG_EXPORT MemoryTemplateLoader : public TemplateLoader {
	Q_OBJECT
public:
	
	/**
	 * The storage type for templates. The key is the template name and the
	 * corresponding value the template data.
	 */
	typedef QMap< QString, QByteArray > Map;
	
	/**
	 * Default constructor.
	 * The template map is empty.
	 * \sa addTemplate setMap
	 */
	explicit MemoryTemplateLoader (QObject *parent = 0);
	
	/**
	 * Constructor which uses \a map as template storage.
	 */
	explicit MemoryTemplateLoader (Map map, QObject *parent = 0);
	
	/** Destructor. */
	~MemoryTemplateLoader () override;
	
	/** Returns the template map. */
	Map map () const;
	
	/** Sets the template map. */
	void setMap (const Map &map);
	
	/**
	 * Adds a template called \a name with \a data. If there's already a
	 * template with this name, the old one will be overridden.
	 */
	void addTemplate (const QString &name, const QByteArray &data);
	
	/**
	 * Removes the template called \a name.
	 */
	void removeTemplate (const QString &name);
	
	/**
	 * Returns the data of template \a name.
	 * \sa addTemplate setMap
	 */
	QByteArray load (const QString &name) override;
	
	bool hasTemplate (const QString &name) override;
	
private:
	MemoryTemplateLoaderPrivate *d_ptr;
	
};

}

#endif // NURIA_MEMORYTEMPLATELOADER_HPP
