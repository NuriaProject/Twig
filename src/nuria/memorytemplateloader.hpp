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

#ifndef NURIA_MEMORYTEMPLATELOADER_HPP
#define NURIA_MEMORYTEMPLATELOADER_HPP

#include "templateloader.hpp"
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
class MemoryTemplateLoader : public TemplateLoader {
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
